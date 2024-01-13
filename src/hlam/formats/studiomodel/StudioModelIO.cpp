#include <cstddef>
#include <cstring>
#include <iterator>

#include <fmt/format.h>
#include <fmt/std.h>

#include "filesystem/IFileSystem.hpp"

#include "formats/studiomodel/StudioModel.hpp"
#include "formats/studiomodel/StudioModelFileFormat.hpp"
#include "formats/studiomodel/StudioModelIO.hpp"

#include "utility/IOUtils.hpp"

namespace studiomdl
{
bool IsStudioModel(FILE* file, bool acceptSequenceFiles)
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

	if (strncmp(reinterpret_cast<const char*>(&id), STUDIOMDL_HDR_ID, 4) != 0
		&& (!acceptSequenceFiles || strncmp(reinterpret_cast<const char*>(&id), STUDIOMDL_SEQ_ID, 4) != 0))
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
	if (!IsStudioModel(file, false))
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

static std::tuple<std::unique_ptr<std::byte[]>, size_t> ReadStudioFileIntoBuffer(
	const std::filesystem::path& fileName, FILE* file)
{
	auto result = ReadFileIntoBuffer(file);

	if (!std::get<0>(result))
	{
		throw AssetException(fmt::format("Error reading file \"{}\"", fileName));
	}

	return result;
}

template<typename T>
static void CheckHeaderIntegrity(const std::filesystem::path& fileName, const T* header, const char* headerId)
{
	if (strncmp(reinterpret_cast<const char*>(&header->id), headerId, 4) != 0)
	{
		auto message = fmt::format("The file \"{}\" is not a studio model type {} file", fileName, headerId);

		if (strncmp(reinterpret_cast<const char*>(&header->id), STUDIOMDL_SEQ_ID, 4) == 0)
		{
			message += "\nIt is a sequence group file (load the main file instead)";
		}

		throw AssetException(message);
	}

	if (header->version != STUDIO_VERSION)
	{
		throw AssetException(fmt::format("File \"{}\": version differs: expected \"{}\", got \"{}\"",
			fileName, std::to_string(STUDIO_VERSION), header->version));
	}
}

static studiomdl::StudioPtr<studiohdr_t> LoadMainHeader(const std::filesystem::path& fileName, FILE* mainFile)
{
	auto [buffer, size] = ReadStudioFileIntoBuffer(fileName, mainFile);

	auto header = reinterpret_cast<studiohdr_t*>(buffer.get());

	CheckHeaderIntegrity(fileName, header, STUDIOMDL_HDR_ID);

	buffer.release();

	StudioPtr<studiohdr_t> mainHeader(header, size);

	if (mainHeader->name[0] == '\0')
	{
		//Only the main header sets the name, so this must be something else (probably texture header, but could be anything)
		auto message = fmt::format("The file \"{}\" is not a studio model main header file",
			reinterpret_cast<const char*>(fileName.u8string().c_str()));

		if (!fileName.empty() && std::toupper(fileName.stem().u8string().back()) == 'T')
		{
			message += "\nIt is probably a texture file (load the main file instead)";
		}

		throw AssetException(message);
	}

	return mainHeader;
}

static studiomdl::StudioPtr<studiohdr_t> LoadTextureHeader(
	const std::filesystem::path& fileName, studiohdr_t* mainHeader, IFileSystem& fileSystem)
{
	// preload textures
	// The original model viewer code used numtextures here, whereas the engine uses textureindex.
	// numtextures can be 0 for a model with no textures so this must be handled properly.
	if (mainHeader->textureindex != 0)
	{
		return {};
	}

	std::filesystem::path texturename = fileName;

	texturename.replace_filename(texturename.stem().u8string() + u8'T' + texturename.extension().u8string());

	FilePtr file{ fileSystem.TryOpenAbsolute(reinterpret_cast<const char*>(texturename.u8string().c_str()), true) };

	if (!file)
	{
		throw AssetException(fmt::format(
			"External texture file \"{}\" does not exist or is currently opened by another program", texturename));
	}

	auto [buffer, size] = ReadStudioFileIntoBuffer(fileName, file.get());

	auto header = reinterpret_cast<studiohdr_t*>(buffer.get());

	CheckHeaderIntegrity(fileName, header, STUDIOMDL_HDR_ID);

	buffer.release();

	return StudioPtr<studiohdr_t>(header, size);
}

static StudioPtr<studioseqhdr_t> LoadSequenceGroup(const std::filesystem::path& fileName, IFileSystem& fileSystem)
{
	FilePtr file{ fileSystem.TryOpenAbsolute(reinterpret_cast<const char*>(fileName.u8string().c_str()), true, true) };

	if (!file)
	{
		throw AssetException(fmt::format(
			"Sequence group file \"{}\" does not exist or is currently opened by another program", fileName));
	}

	auto [buffer, size] = ReadStudioFileIntoBuffer(fileName, file.get());

	auto header = reinterpret_cast<studioseqhdr_t*>(buffer.get());

	CheckHeaderIntegrity(fileName, header, STUDIOMDL_SEQ_ID);

	buffer.release();

	return StudioPtr<studioseqhdr_t>(header, size);
}

static std::vector<StudioPtr<studioseqhdr_t>> LoadSequenceGroups(
	const std::filesystem::path& fileName, studiohdr_t* mainHeader, IFileSystem& fileSystem)
{
	// preload animations
	if (mainHeader->numseqgroups <= 1)
	{
		return {};
	}

	std::vector<StudioPtr<studioseqhdr_t>> sequenceHeaders;

	sequenceHeaders.reserve(mainHeader->numseqgroups - 1);

	const std::string baseFileName = reinterpret_cast<const char*>(fileName.stem().u8string().c_str());
	const std::string extension = reinterpret_cast<const char*>(fileName.extension().u8string().c_str());

	std::filesystem::path groupFileName = fileName;

	std::string seqgroupname;

	for (int i = 1; i < mainHeader->numseqgroups; ++i)
	{
		seqgroupname.clear();
		fmt::format_to(std::back_inserter(seqgroupname), "{}{:0>2}{}", baseFileName, i, extension);

		groupFileName.replace_filename(seqgroupname);

		sequenceHeaders.emplace_back(LoadSequenceGroup(groupFileName, fileSystem));
	}

	return sequenceHeaders;
}

std::unique_ptr<StudioModel> LoadStudioModel(
	const std::filesystem::path& fileName, FILE* mainFile, IFileSystem& fileSystem)
{
	StudioPtr<studiohdr_t> mainHeader = LoadMainHeader(fileName, mainFile);
	StudioPtr<studiohdr_t> textureHeader = LoadTextureHeader(fileName, mainHeader.get(), fileSystem);
	std::vector<StudioPtr<studioseqhdr_t>> sequenceHeaders = LoadSequenceGroups(fileName, mainHeader.get(), fileSystem);
	const auto isDol = fileName.extension() == ".dol";

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
