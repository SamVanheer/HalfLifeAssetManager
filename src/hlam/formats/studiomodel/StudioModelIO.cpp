#include <cstddef>
#include <cstring>
#include <sstream>

#include "formats/studiomodel/StudioModel.hpp"
#include "formats/studiomodel/StudioModelFileFormat.hpp"
#include "formats/studiomodel/StudioModelIO.hpp"

#include "utility/IOUtils.hpp"

namespace studiomdl
{
bool IsStudioModel(FILE* file)
{
	if (!file)
	{
		return false;
	}

	std::int32_t id;

	if (fread(&id, sizeof(id), 1, file) != 1)
	{
		return false;
	}

	if (strncmp(reinterpret_cast<const char*>(&id), STUDIOMDL_HDR_ID, 4) != 0)
	{
		return false;
	}

	std::int32_t version;

	if (fread(&version, sizeof(version), 1, file) != 1)
	{
		return false;
	}

	if (version != STUDIO_VERSION)
	{
		return false;
	}

	return true;
}

bool IsMainStudioModel(FILE* file)
{
	if (!IsStudioModel(file))
	{
		return false;
	}

	// It's a main file if it has a bone section, even if it has no bones.
	const std::size_t offset = offsetof(studiohdr_t, boneindex);

	if (fseek(file, offset, SEEK_SET) != 0)
	{
		return false;
	}

	int boneindex;

	if (fread(&boneindex, sizeof(boneindex), 1, file) != 1)
	{
		return false;
	}

	return boneindex > 0;
}

namespace
{
template<typename T>
StudioPtr<T> LoadStudioHeader(const std::filesystem::path& fileName, FILE* existingFile, const bool bAllowSeqGroup, const bool externalTextures)
{
	const std::string utf8FileName{reinterpret_cast<const char*>(fileName.u8string().c_str())};

	// load the model
	FILE* file = existingFile ? existingFile : utf8_exclusive_read_fopen(utf8FileName.c_str(), true);

	if (!file)
	{
		//TODO: eventually file open calls will be routed through IFileSystem which will handle case sensitivity automatically
		if (externalTextures)
		{
			auto stem{fileName.stem().u8string()};

			if (!stem.empty())
			{
				stem.back() = 't';

				std::filesystem::path loweredFileName = fileName;

				loweredFileName.replace_filename(stem);
				loweredFileName.replace_extension(fileName.extension());

				file = utf8_exclusive_read_fopen(loweredFileName.u8string().c_str(), true);
			}
		}

		if (!file)
		{
			// Not necessarily an error.
			if (externalTextures)
			{
				return {};
			}

			throw AssetException(std::string{"File \""} + utf8FileName + "\" does not exist or is currently opened by another program");
		}
	}

	fseek(file, 0, SEEK_END);
	const size_t size = ftell(file);
	fseek(file, 0, SEEK_SET);

	auto buffer = std::make_unique<std::byte[]>(size);

	auto header = reinterpret_cast<T*>(buffer.get());

	const size_t readCount = fread(header, size, 1, file);

	if (!existingFile)
	{
		fclose(file);
	}

	if (readCount != 1)
	{
		throw AssetException(std::string{"Error reading file \""} + utf8FileName + "\"");
	}

	if (strncmp(reinterpret_cast<const char*>(&header->id), STUDIOMDL_HDR_ID, 4) &&
		strncmp(reinterpret_cast<const char*>(&header->id), STUDIOMDL_SEQ_ID, 4))
	{
		throw AssetException(std::string{"The file \""} + utf8FileName + "\" is neither a studio header nor a sequence header");
	}

	if (!bAllowSeqGroup && !strncmp(reinterpret_cast<const char*>(&header->id), STUDIOMDL_SEQ_ID, 4))
	{
		throw AssetException(std::string{"File \""} + utf8FileName + "\": Expected a main studio model file, got a sequence file");
	}

	if (header->version != STUDIO_VERSION)
	{
		throw AssetException(std::string{"File \""} + utf8FileName + "\": version differs: expected \"" +
			std::to_string(STUDIO_VERSION) + "\", got \"" + std::to_string(header->version) + "\"");
	}

	/*
	//Validate header length. This should always be valid since it's set by the compiler
	if (header->length < 0 || (static_cast<size_t>(header->length) != size))
	{
		throw AssetException(std::string{"File \""} + utf8FileName + "\": length does not match file size: expected \""
			+ std::to_string(size) + "\", got \"" + std::to_string(header->length) + "\"");
	}
	*/

	buffer.release();

	return StudioPtr<T>(header, size);
}
}

std::unique_ptr<StudioModel> LoadStudioModel(const std::filesystem::path& fileName, FILE* mainFile)
{
	std::filesystem::path baseFileName{fileName};

	baseFileName.replace_extension();

	const auto isDol = fileName.extension() == ".dol";

	//Load the model
	auto mainHeader = LoadStudioHeader<studiohdr_t>(fileName, mainFile, false, false);

	if (mainHeader->name[0] == '\0')
	{
		//Only the main header sets the name, so this must be something else (probably texture header, but could be anything)
		auto message = std::u8string{u8"The file \""} + fileName.u8string() + u8"\" is not a studio model main header file";

		if (!baseFileName.empty() && std::toupper(baseFileName.u8string().back()) == 'T')
		{
			message += u8" (it is probably a texture file)";
		}

		throw AssetException(message);
	}

	StudioPtr<studiohdr_t> textureHeader;

	// preload textures
	// The original model viewer code used numtextures here, whereas the engine uses textureindex.
	// numtextures can be 0 for a model with no textures so this must be handled properly.
	if (mainHeader->textureindex == 0)
	{
		const auto extension = isDol ? "T.dol" : "T.mdl";

		std::filesystem::path texturename = baseFileName;

		texturename += extension;

		textureHeader = LoadStudioHeader<studiohdr_t>(texturename, nullptr, true, true);

		if (!textureHeader)
		{
			throw AssetException(
				std::string{"External texture file \""}
					+ reinterpret_cast<const char*>(texturename.u8string().c_str())
					+ "\" does not exist or is currently opened by another program");
		}
	}

	std::vector<StudioPtr<studioseqhdr_t>> sequenceHeaders;

	// preload animations
	if (mainHeader->numseqgroups > 1)
	{
		sequenceHeaders.reserve(mainHeader->numseqgroups - 1);

		const std::string baseFileNameString = reinterpret_cast<const char*>(baseFileName.u8string().c_str());

		std::ostringstream seqgroupname;

		for (int i = 1; i < mainHeader->numseqgroups; ++i)
		{
			seqgroupname.str({});

			const auto suffix = isDol ? ".dol" : ".mdl";

			seqgroupname << baseFileNameString <<
				std::setfill('0') << std::setw(2) << i <<
				std::setw(0) << suffix;

			sequenceHeaders.emplace_back(
				LoadStudioHeader<studioseqhdr_t>(std::filesystem::u8path(seqgroupname.str()), nullptr, true, false));
		}
	}

	return std::make_unique<StudioModel>(std::move(mainHeader), std::move(textureHeader),
		std::move(sequenceHeaders), isDol);
}

void SaveStudioModel(const std::filesystem::path& fileName, StudioModel& model)
{
	if (fileName.empty())
	{
		throw AssetException("Empty filename provided");
	}

	studiohdr_t* const pStudioHdr = model.GetStudioHeader();

	assert(pStudioHdr->numseqgroups == 1);

	FILE* file = utf8_fopen(fileName.u8string().c_str(), "wb");

	if (!file)
	{
		throw AssetException("Could not open main file for writing");
	}

	bool success = fwrite(pStudioHdr, sizeof(std::byte), pStudioHdr->length, file) == pStudioHdr->length;

	fclose(file);

	if (!success)
	{
		throw AssetException("Error while writing to main file");
	}

	auto baseFileName{fileName};

	baseFileName.replace_extension();

	// write texture model
	if (model.HasSeparateTextureHeader())
	{
		const studiohdr_t* const pTextureHdr = model.GetTextureHeader();

		std::filesystem::path texturename = baseFileName;

		texturename += "T.mdl";

		file = utf8_fopen(texturename.u8string().c_str(), "wb");

		if (!file)
		{
			throw AssetException("Could not open texture file for writing");
		}

		success = fwrite(pTextureHdr, sizeof(std::byte), pTextureHdr->length, file) == pTextureHdr->length;
		fclose(file);

		if (!success)
		{
			throw AssetException("Error while writing to texture file");
		}
	}
}
}
