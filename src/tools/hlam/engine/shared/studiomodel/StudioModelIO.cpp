#include <sstream>

#include "engine/shared/studiomodel/StudioModel.hpp"
#include "engine/shared/studiomodel/StudioModelFileFormat.hpp"
#include "engine/shared/studiomodel/StudioModelIO.hpp"

#include "utility/IOUtils.hpp"

namespace studiomdl
{
bool IsStudioModel(const std::string& fileName)
{
	bool isStudioModel = false;

	if (FILE* file = utf8_fopen(fileName.c_str(), "rb"); file)
	{
		std::int32_t id;

		if (fread(&id, sizeof(id), 1, file) == 1)
		{
			if (strncmp(reinterpret_cast<const char*>(&id), STUDIOMDL_HDR_ID, 4) == 0)
			{
				std::int32_t version;

				if (fread(&version, sizeof(version), 1, file) == 1)
				{
					if (version == STUDIO_VERSION)
					{
						isStudioModel = true;
					}
				}
			}
		}

		fclose(file);
	}

	return isStudioModel;
}

namespace
{
template<typename T>
studio_ptr<T> LoadStudioHeader(const std::filesystem::path& fileName, const bool bAllowSeqGroup, const bool externalTextures)
{
	const std::string utf8FileName{fileName.u8string()};

	// load the model
	FILE* file = utf8_fopen(utf8FileName.c_str(), "rb");

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

				file = utf8_fopen(loweredFileName.u8string().c_str(), "rb");
			}
		}

		if (!file)
		{
			throw assets::AssetFileNotFound(std::string{"File \""} + utf8FileName + "\" not found");
		}
	}

	fseek(file, 0, SEEK_END);
	const size_t size = ftell(file);
	fseek(file, 0, SEEK_SET);

	auto buffer = std::make_unique<byte[]>(size);

	auto header = reinterpret_cast<T*>(buffer.get());

	const size_t readCount = fread(header, size, 1, file);
	fclose(file);

	if (readCount != 1)
	{
		throw assets::AssetInvalidFormat(std::string{"Error reading file \""} + utf8FileName + "\"");
	}

	if (strncmp(reinterpret_cast<const char*>(&header->id), STUDIOMDL_HDR_ID, 4) &&
		strncmp(reinterpret_cast<const char*>(&header->id), STUDIOMDL_SEQ_ID, 4))
	{
		throw assets::AssetInvalidFormat(std::string{"The file \""} + utf8FileName + "\" is neither a studio header nor a sequence header");
	}

	if (!bAllowSeqGroup && !strncmp(reinterpret_cast<const char*>(&header->id), STUDIOMDL_SEQ_ID, 4))
	{
		throw assets::AssetInvalidFormat(std::string{"File \""} + utf8FileName + "\": Expected a main studio model file, got a sequence file");
	}

	if (header->version != STUDIO_VERSION)
	{
		throw assets::AssetVersionDiffers(std::string{"File \""} + utf8FileName + "\": version differs: expected \"" +
			std::to_string(STUDIO_VERSION) + "\", got \"" + std::to_string(header->version) + "\"");
	}

	//Validate header length. This should always be valid since it's set by the compiler
	if (header->length < 0 || (static_cast<size_t>(header->length) != size))
	{
		throw assets::AssetException(std::string{"File \""} + utf8FileName + "\": length does not match file size: expected \""
			+ std::to_string(size) + "\", got \"" + std::to_string(header->length) + "\"");
	}

	buffer.release();

	return studio_ptr<T>(header);
}
}

std::unique_ptr<StudioModel> LoadStudioModel(const std::filesystem::path& fileName)
{
	std::filesystem::path baseFileName{fileName};

	baseFileName.replace_extension();

	const auto isDol = fileName.extension() == ".dol";

	//Load the model
	auto mainHeader = LoadStudioHeader<studiohdr_t>(fileName, false, false);

	if (mainHeader->name[0] == '\0')
	{
		//Only the main hader sets the name, so this must be something else (probably texture header, but could be anything)
		auto message = std::string{"The file \""} + fileName.u8string() + "\" is not a studio model main header file";

		if (!baseFileName.empty() && std::toupper(baseFileName.u8string().back()) == 'T')
		{
			message += " (it is probably a texture file)";
		}

		throw StudioModelIsNotMainHeader(message);
	}

	studio_ptr<studiohdr_t> textureHeader;

	// preload textures
	if (mainHeader->numtextures == 0)
	{
		const auto extension = isDol ? "T.dol" : "T.mdl";

		std::filesystem::path texturename = baseFileName;

		texturename += extension;

		textureHeader = LoadStudioHeader<studiohdr_t>(texturename, true, true);
	}

	std::vector<studio_ptr<studioseqhdr_t>> sequenceHeaders;

	// preload animations
	if (mainHeader->numseqgroups > 1)
	{
		sequenceHeaders.reserve(mainHeader->numseqgroups - 1);

		std::stringstream seqgroupname;

		for (int i = 1; i < mainHeader->numseqgroups; ++i)
		{
			seqgroupname.str({});

			const auto suffix = isDol ? ".dol" : ".mdl";

			seqgroupname << baseFileName.u8string() <<
				std::setfill('0') << std::setw(2) << i <<
				std::setw(0) << suffix;

			sequenceHeaders.emplace_back(LoadStudioHeader<studioseqhdr_t>(seqgroupname.str(), true, false));
		}
	}

	return std::make_unique<StudioModel>(std::move(mainHeader), std::move(textureHeader),
		std::move(sequenceHeaders), isDol);
}

void SaveStudioModel(const std::filesystem::path& fileName, StudioModel& model, bool correctSequenceGroupFileNames)
{
	if (fileName.empty())
	{
		throw assets::AssetException("Empty filename provided");
	}

	studiohdr_t* const pStudioHdr = model.GetStudioHeader();

	if (correctSequenceGroupFileNames)
	{
		std::filesystem::path baseFileName{fileName};

		//Find the "models" directory to determine what the relative path is
		std::filesystem::path relativeTo = baseFileName;

		auto foundRelative = false;

		//This will loop forever unless we test to see if relativeTo is the root path
		while (!relativeTo.empty() && relativeTo != relativeTo.root_path())
		{
			relativeTo = relativeTo.parent_path();

			if (relativeTo.stem() == "models")
			{
				relativeTo = relativeTo.parent_path();
				foundRelative = true;
				break;
			}
		}

		if (!foundRelative)
		{
			throw assets::AssetException("Could not find base directory \"models\" in model filename; needed to correct sequence group filenames");
		}

		std::error_code e;

		baseFileName = std::filesystem::relative(baseFileName, relativeTo, e);

		if (e)
		{
			throw assets::AssetException("Could not determine base filename for model to correct sequence group filenames");
		}

		baseFileName.replace_extension();

		const auto baseFileNameString = baseFileName.u8string();

		std::stringstream seqgroupname;

		//Group 0 is the main file and is never used to load files, and also doesn't have a filename set in the seqgroup structure
		for (int i = 1; i < pStudioHdr->numseqgroups; i++)
		{
			seqgroupname.str({});

			seqgroupname << baseFileNameString <<
				std::setfill('0') << std::setw(2) << i <<
				std::setw(0) << ".mdl";

			const auto groupNameString = seqgroupname.str();

			auto seqGroup = pStudioHdr->GetSequenceGroup(i);

			if (groupNameString.length() >= sizeof(seqGroup->name))
			{
				throw assets::AssetException("Sequence group filename is too long");
			}

			strncpy(seqGroup->name, groupNameString.c_str(), groupNameString.length());

			seqGroup->name[sizeof(seqGroup->name) - 1] = '\0';
		}
	}

	FILE* file = utf8_fopen(fileName.u8string().c_str(), "wb");

	if (!file)
	{
		throw assets::AssetException("Could not open main file for writing");
	}

	bool success = fwrite(pStudioHdr, sizeof(byte), pStudioHdr->length, file) == pStudioHdr->length;

	fclose(file);

	if (!success)
	{
		throw assets::AssetException("Error while writing to main file");
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
			throw assets::AssetException("Could not open texture file for writing");
		}

		success = fwrite(pTextureHdr, sizeof(byte), pTextureHdr->length, file) == pTextureHdr->length;
		fclose(file);

		if (!success)
		{
			throw assets::AssetException("Error while writing to texture file");
		}
	}

	// write seq groups
	if (pStudioHdr->numseqgroups > 1)
	{
		std::stringstream seqgroupname;

		for (int i = 1; i < pStudioHdr->numseqgroups; ++i)
		{
			seqgroupname.str({});

			seqgroupname << baseFileName.u8string() <<
				std::setfill('0') << std::setw(2) << i <<
				std::setw(0) << ".mdl";

			file = utf8_fopen(seqgroupname.str().c_str(), "wb");

			if (!file)
			{
				throw assets::AssetException("Could not open sequence file for writing");
			}

			const auto pAnimHdr = model.GetSeqGroupHeader(i - 1);

			success = fwrite(pAnimHdr, sizeof(byte), pAnimHdr->length, file) == pAnimHdr->length;
			fclose(file);

			if (!success)
			{
				throw assets::AssetException("Error while writing to sequence file");
			}
		}
	}
}
}
