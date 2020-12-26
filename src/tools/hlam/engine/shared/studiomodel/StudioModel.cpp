#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>

#include "core/shared/Platform.hpp"
#include "core/shared/Logging.hpp"

#include "utility/IOUtils.hpp"
#include "utility/StringUtils.hpp"

#include "graphics/GraphicsUtils.hpp"
#include "graphics/Palette.hpp"
#include "graphics/TextureLoader.hpp"

#include "engine/shared/studiomodel/StudioModel.hpp"

namespace studiomdl
{
namespace
{
//Dol differs only in texture storage
//Instead of pixels followed by RGB palette, it has a 32 byte texture name (name of file without extension), followed by an RGBA palette and pixels
void ConvertDolToMdl(byte* pBuffer, const mstudiotexture_t& texture)
{
	const int RGBA_PALETTE_CHANNELS = 4;

	auto palette = std::make_unique<byte[]>(PALETTE_SIZE);

	//Starts off with 32 byte texture name
	auto pSourcePalette = pBuffer + texture.index + 32;

	//Discard alpha value
	//TODO: convert alpha value somehow? is it even used?
	for (int i = 0; i < PALETTE_ENTRIES; ++i)
	{
		for (int j = 0; j < PALETTE_CHANNELS; ++j)
		{
			palette[i * PALETTE_CHANNELS + j] = pSourcePalette[i * RGBA_PALETTE_CHANNELS + j];
		}
	}

	const auto size = texture.width * texture.height;

	auto pixels = std::make_unique<byte[]>(size);

	auto pSourcePixels = pSourcePalette + PALETTE_ENTRIES * RGBA_PALETTE_CHANNELS;

	for (int i = 0; i < size; ++i)
	{
		auto pixel = pSourcePixels[i];

		auto masked = pixel & 0x1F;

		//Adjust the index to map to the correct palette entry
		if (masked >= 8)
		{
			if (masked >= 16)
			{
				if (masked < 24)
				{
					pixel -= 8;
				}
			}
			else
			{
				pixel += 8;
			}
		}

		pixels[i] = pixel;
	}

	//Now write the correct data to the model buffer
	auto pDestPixels = pBuffer + texture.index;

	for (int i = 0; i < size; ++i)
	{
		pDestPixels[i] = pixels[i];
	}

	auto pDestPalette = pBuffer + texture.index + texture.width * texture.height;

	memcpy(pDestPalette, palette.get(), PALETTE_SIZE);

	//Some data will be left dangling after the palette and before the next texture/end of file. Nothing will reference it though
	//in the SL version this will not be a problem since the file isn't loaded in one chunk
}
}

CStudioModel::CStudioModel(std::string&& fileName, studio_ptr<studiohdr_t>&& pStudioHdr, studio_ptr<studiohdr_t>&& pTextureHdr,
	std::vector<studio_ptr<studioseqhdr_t>>&& sequenceHeaders, bool isDol)
	: m_FileName(std::move(fileName))
	, m_pStudioHdr(std::move(pStudioHdr))
	, m_pTextureHdr(std::move(pTextureHdr))
	, m_SequenceHeaders(std::move(sequenceHeaders))
	, m_IsDol(isDol)
{
	assert(m_pStudioHdr);
}

CStudioModel::~CStudioModel()
{
	glDeleteTextures(GetTextureHeader()->numtextures, m_Textures.data());
	m_Textures.clear();
}

mstudioanim_t* CStudioModel::GetAnim(mstudioseqdesc_t* pseqdesc) const
{
	mstudioseqgroup_t* pseqgroup = m_pStudioHdr->GetSequenceGroup(pseqdesc->seqgroup);

	if (pseqdesc->seqgroup == 0)
	{
		return (mstudioanim_t*) ((byte*) m_pStudioHdr.get() + pseqgroup->unused2 + pseqdesc->animindex);
	}

	return (mstudioanim_t*) ((byte*) m_SequenceHeaders[pseqdesc->seqgroup - 1].get() + pseqdesc->animindex);
}

mstudiomodel_t* CStudioModel::GetModelByBodyPart(const int iBody, const int iBodyPart) const
{
	mstudiobodyparts_t* pbodypart = m_pStudioHdr->GetBodypart(iBodyPart);

	int index = iBody / pbodypart->base;
	index = index % pbodypart->nummodels;

	return (mstudiomodel_t*) ((byte*) m_pStudioHdr.get() + pbodypart->modelindex) + index;
}

int CStudioModel::GetBodyValueForGroup(int compositeValue, int group) const
{
	if (group >= m_pStudioHdr->numbodyparts)
	{
		return -1;
	}

	const mstudiobodyparts_t* const bodyPart = m_pStudioHdr->GetBodypart(group);

	return (compositeValue / bodyPart->base) % bodyPart->nummodels;
}

bool CStudioModel::CalculateBodygroup(const int iGroup, const int iValue, int& iInOutBodygroup) const
{
	if (iGroup > m_pStudioHdr->numbodyparts)
		return false;

	const mstudiobodyparts_t* const pbodypart = m_pStudioHdr->GetBodypart(iGroup);

	int iCurrent = (iInOutBodygroup / pbodypart->base) % pbodypart->nummodels;

	if (iValue >= pbodypart->nummodels)
		return true;

	iInOutBodygroup = (iInOutBodygroup - (iCurrent * pbodypart->base) + (iValue * pbodypart->base));

	return true;
}

GLuint CStudioModel::GetTextureId(const int iIndex) const
{
	const studiohdr_t* const pHdr = GetTextureHeader();

	if (!pHdr)
		return GL_INVALID_TEXTURE_ID;

	if (iIndex < 0 || iIndex >= pHdr->numtextures)
		return GL_INVALID_TEXTURE_ID;

	return m_Textures[iIndex];
}

void CStudioModel::CreateTextures(graphics::TextureLoader& textureLoader)
{
	const auto textureHeader = GetTextureHeader();

	if (textureHeader->textureindex > 0 && textureHeader->numtextures <= CStudioModel::MAX_TEXTURES)
	{
		byte* pIn = reinterpret_cast<byte*>(textureHeader);

		for (int i = 0; i < textureHeader->numtextures; ++i)
		{
			GLuint name;

			glBindTexture(GL_TEXTURE_2D, 0);
			glGenTextures(1, &name);

			const auto& texture = *textureHeader->GetTexture(i);

			if (m_IsDol)
			{
				ConvertDolToMdl(pIn, texture);
			}

			textureLoader.UploadIndexed8(
				name,
				texture.width, texture.height,
				pIn + texture.index,
				pIn + texture.index + (texture.width * texture.height),
				(texture.flags & STUDIO_NF_NOMIPS) != 0,
				(texture.flags & STUDIO_NF_MASKED) != 0);

			m_Textures.emplace_back(name);
		}
	}
}

void CStudioModel::ReplaceTexture(graphics::TextureLoader& textureLoader, mstudiotexture_t* ptexture, const byte* data, const byte* pal, GLuint textureId)
{
	textureLoader.UploadIndexed8(
		textureId,
		ptexture->width, ptexture->height,
		data,
		pal,
		(ptexture->flags & STUDIO_NF_NOMIPS) != 0,
		(ptexture->flags & STUDIO_NF_MASKED) != 0);
}

void CStudioModel::ReuploadTexture(graphics::TextureLoader& textureLoader, mstudiotexture_t* ptexture)
{
	assert(ptexture);

	auto header = GetTextureHeader();

	const int iIndex = ptexture - header->GetTextures();

	if (iIndex < 0 || iIndex >= header->numtextures)
	{
		Error("CStudioModel::ReuploadTexture: Invalid texture!");
		return;
	}

	ReplaceTexture(textureLoader, ptexture,
		header->GetData() + ptexture->index, header->GetData() + ptexture->index + ptexture->width * ptexture->height, m_Textures[iIndex]);
}

void CStudioModel::UpdateFilters(graphics::TextureLoader& textureLoader)
{
	if (m_Textures.empty())
	{
		//No textures loaded yet, do nothing
		return;
	}

	const auto textureHeader{GetTextureHeader()};

	for (int i = 0; i < textureHeader->numtextures; ++i)
	{
		glBindTexture(GL_TEXTURE_2D, m_Textures[i]);
		textureLoader.SetFilters(m_Textures[i], (textureHeader->GetTexture(i)->flags & STUDIO_NF_NOMIPS) != 0);
	}
}

void CStudioModel::ReuploadTextures(graphics::TextureLoader& textureLoader)
{
	if (m_Textures.empty())
	{
		//No textures loaded yet, do nothing
		return;
	}

	auto header = GetTextureHeader();

	for (int i = 0; i < header->numtextures; ++i)
	{
		const auto ptexture = header->GetTexture(i);

		ReplaceTexture(textureLoader, ptexture,
			header->GetData() + ptexture->index, header->GetData() + ptexture->index + ptexture->width * ptexture->height, m_Textures[i]);
	}
}

namespace
{
template<typename T>
studio_ptr<T> LoadStudioHeader(const char* const pszFilename, const bool bAllowSeqGroup)
{
	// load the model
	FILE* pFile = utf8_fopen(pszFilename, "rb");

	if (!pFile)
	{
		throw assets::AssetFileNotFound(std::string{"File \""} + pszFilename + "\" not found");
	}

	fseek(pFile, 0, SEEK_END);
	const size_t size = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);

	auto buffer = std::make_unique<byte[]>(size);

	auto pStudioHdr = reinterpret_cast<T*>(buffer.get());

	const size_t uiRead = fread(pStudioHdr, size, 1, pFile);
	fclose(pFile);

	if (uiRead != 1)
	{
		throw assets::AssetInvalidFormat(std::string{"Error reading file \""} + pszFilename + "\"");
	}

	if (strncmp(reinterpret_cast<const char*>(&pStudioHdr->id), STUDIOMDL_HDR_ID, 4) &&
		strncmp(reinterpret_cast<const char*>(&pStudioHdr->id), STUDIOMDL_SEQ_ID, 4))
	{
		throw assets::AssetInvalidFormat(std::string{"The file \""} + pszFilename + "\" is neither a studio header nor a sequence header");
	}

	if (!bAllowSeqGroup && !strncmp(reinterpret_cast<const char*>(&pStudioHdr->id), STUDIOMDL_SEQ_ID, 4))
	{
		throw assets::AssetInvalidFormat(std::string{"File \""} + pszFilename + "\": Expected a main studio model file, got a sequence file");
	}

	if (pStudioHdr->version != STUDIO_VERSION)
	{
		throw assets::AssetVersionDiffers(std::string{"File \""} + pszFilename + "\": version differs: expected \"" +
			std::to_string(STUDIO_VERSION) + "\", got \"" + std::to_string(pStudioHdr->version) + "\"");
	}

	buffer.release();

	return studio_ptr<T>(pStudioHdr);
}
}

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

std::unique_ptr<CStudioModel> LoadStudioModel(const char* const pszFilename)
{
	const std::filesystem::path fileName{std::filesystem::u8path(pszFilename)};

	std::filesystem::path baseFileName{fileName};

	baseFileName.replace_extension();

	const auto bIsDol = fileName.extension() == ".dol";

	//Load the model
	auto mainHeader = LoadStudioHeader<studiohdr_t>(pszFilename, false);

	if (mainHeader->name[0] == '\0')
	{
		//Only the main hader sets the name, so this must be something else (probably texture header, but could be anything)
		auto message = std::string{"The file \""} + pszFilename + "\" is not a studio model main header file";

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
		const auto extension = bIsDol ? "T.dol" : "T.mdl";

		std::filesystem::path texturename = baseFileName;

		texturename += extension;

		textureHeader = LoadStudioHeader<studiohdr_t>(texturename.u8string().c_str(), true);
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

			const auto suffix = bIsDol ? ".dol" : ".mdl";

			seqgroupname << baseFileName.u8string() <<
				std::setfill('0') << std::setw(2) << i <<
				std::setw(0) << suffix;

			sequenceHeaders.emplace_back(LoadStudioHeader<studioseqhdr_t>(seqgroupname.str().c_str(), true));
		}
	}

	return std::make_unique<CStudioModel>(pszFilename, std::move(mainHeader), std::move(textureHeader),
		std::move(sequenceHeaders), bIsDol);
}

void SaveStudioModel(const char* const pszFilename, CStudioModel& model, bool correctSequenceGroupFileNames)
{
	if (!pszFilename)
	{
		throw assets::AssetException("Null filename provided");
	}

	if (!(*pszFilename))
	{
		throw assets::AssetException("Empty filename provided");
	}

	studiohdr_t* const pStudioHdr = model.GetStudioHeader();

	if (correctSequenceGroupFileNames)
	{
		std::filesystem::path baseFileName{pszFilename};

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

	FILE* pFile = utf8_fopen(pszFilename, "wb");

	if (!pFile)
	{
		throw assets::AssetException("Could not open main file for writing");
	}

	bool bSuccess = fwrite(pStudioHdr, sizeof(byte), pStudioHdr->length, pFile) == pStudioHdr->length;

	fclose(pFile);

	if (!bSuccess)
	{
		throw assets::AssetException("Error while writing to main file");
	}

	const std::filesystem::path fileName{std::filesystem::u8path(pszFilename)};

	auto baseFileName{fileName};

	baseFileName.replace_extension();

	// write texture model
	if (model.HasSeparateTextureHeader())
	{
		const studiohdr_t* const pTextureHdr = model.GetTextureHeader();

		std::filesystem::path texturename = baseFileName;

		texturename += "T.mdl";

		pFile = utf8_fopen(texturename.u8string().c_str(), "wb");

		if (!pFile)
		{
			throw assets::AssetException("Could not open texture file for writing");
		}

		bSuccess = fwrite(pTextureHdr, sizeof(byte), pTextureHdr->length, pFile) == pTextureHdr->length;
		fclose(pFile);

		if (!bSuccess)
		{
			throw assets::AssetException("Error while writing to texture file");
		}
	}

	// write seq groups
	if (pStudioHdr->numseqgroups > 1)
	{
		std::stringstream seqgroupname;

		for (int i = 1; i < pStudioHdr->numseqgroups; i++)
		{
			seqgroupname.str({});

			seqgroupname << baseFileName.u8string() <<
				std::setfill('0') << std::setw(2) << i <<
				std::setw(0) << ".mdl";

			pFile = utf8_fopen(seqgroupname.str().c_str(), "wb");

			if (!pFile)
			{
				throw assets::AssetException("Could not open sequence file for writing");
			}

			const auto pAnimHdr = model.GetSeqGroupHeader(i - 1);

			bSuccess = fwrite(pAnimHdr, sizeof(byte), pAnimHdr->length, pFile) == pAnimHdr->length;
			fclose(pFile);

			if (!bSuccess)
			{
				throw assets::AssetException("Error while writing to sequence file");
			}
		}
	}
}

std::pair<ScaleMeshesData, ScaleMeshesData> CalculateScaledMeshesData(const CStudioModel& studioModel, const float scale)
{
	const auto header = studioModel.GetStudioHeader();

	std::vector<std::vector<glm::vec3>> oldVertices;
	std::vector<std::vector<glm::vec3>> newVertices;

	// scale verts
	for (int i = 0; i < header->numbodyparts; ++i)
	{
		const auto bodypart = header->GetBodypart(i);

		oldVertices.reserve(oldVertices.size() + bodypart->nummodels);
		newVertices.reserve(newVertices.size() + bodypart->nummodels);

		for (int j = 0; j < bodypart->nummodels; ++j)
		{
			const auto model = reinterpret_cast<const mstudiomodel_t*>(header->GetData() + bodypart->modelindex) + j;
			const auto verts = reinterpret_cast<const glm::vec3*>(header->GetData() + model->vertindex);

			std::vector<glm::vec3> oldVerticesList{verts, verts + model->numverts};
			auto newVerticesList{oldVerticesList};

			for (auto& vertex : newVerticesList)
			{
				vertex *= scale;
			}

			oldVertices.emplace_back(std::move(oldVerticesList));
			newVertices.emplace_back(std::move(newVerticesList));
		}
	}

	// scale complex hitboxes
	std::vector<std::pair<glm::vec3, glm::vec3>> oldHitboxes;
	std::vector<std::pair<glm::vec3, glm::vec3>> newHitboxes;

	oldHitboxes.reserve(header->numhitboxes);
	newHitboxes.reserve(header->numhitboxes);

	for (int i = 0; i < header->numhitboxes; ++i)
	{
		const auto hitbox = header->GetHitBox(i);

		oldHitboxes.emplace_back(std::make_pair(hitbox->bbmin, hitbox->bbmax));
		newHitboxes.emplace_back(std::make_pair(hitbox->bbmin * scale, hitbox->bbmax * scale));
	}

	// scale bounding boxes
	std::vector<std::pair<glm::vec3, glm::vec3>> oldSequenceBBBoxes;
	std::vector<std::pair<glm::vec3, glm::vec3>> newSequenceBBBoxes;

	oldSequenceBBBoxes.reserve(header->numseq);
	newSequenceBBBoxes.reserve(header->numseq);

	for (int i = 0; i < header->numseq; ++i)
	{
		const auto sequence = header->GetSequence(i);

		oldSequenceBBBoxes.emplace_back(std::make_pair(sequence->bbmin, sequence->bbmax));
		newSequenceBBBoxes.emplace_back(std::make_pair(sequence->bbmin * scale, sequence->bbmax * scale));
	}

	// TODO: maybe scale eyeposition, pivots, attachments

	return std::make_pair(
		ScaleMeshesData
		{
			std::move(oldVertices),
			std::move(oldHitboxes),
			std::move(oldSequenceBBBoxes)
		},
		ScaleMeshesData
		{
			std::move(newVertices),
			std::move(newHitboxes),
			std::move(newSequenceBBBoxes)
		});
}

void ApplyScaleMeshesData(CStudioModel& studioModel, const ScaleMeshesData& data)
{
	const auto header = studioModel.GetStudioHeader();

	// scale verts
	int vertexIndex = 0;

	for (int i = 0; i < header->numbodyparts; ++i)
	{
		const auto bodypart = header->GetBodypart(i);

		for (int j = 0; j < bodypart->nummodels; ++j)
		{
			const auto model = reinterpret_cast<const mstudiomodel_t*>(header->GetData() + bodypart->modelindex + j);
			const auto verts = reinterpret_cast<glm::vec3*>(header->GetData() + model->vertindex);

			std::copy_n(data.Vertices[vertexIndex].begin(), model->numverts, verts);

			++vertexIndex;
		}
	}

	// scale complex hitboxes
	for (int i = 0; i < header->numhitboxes; ++i)
	{
		const auto hitbox = header->GetHitBox(i);

		hitbox->bbmin = data.Hitboxes[i].first;
		hitbox->bbmax = data.Hitboxes[i].second;
	}

	// scale bounding boxes
	for (int i = 0; i < header->numseq; ++i)
	{
		const auto sequence = header->GetSequence(i);

		sequence->bbmin = data.SequenceBBoxes[i].first;
		sequence->bbmax = data.SequenceBBoxes[i].second;
	}
}

std::pair<std::vector<ScaleBonesBoneData>, std::vector<ScaleBonesBoneData>> CalculateScaledBonesData(const CStudioModel& studioModel, const float scale)
{
	const auto header = studioModel.GetStudioHeader();

	std::vector<ScaleBonesBoneData> oldData;
	std::vector<ScaleBonesBoneData> newData;

	oldData.reserve(header->numbones);
	newData.reserve(header->numbones);

	for (int i = 0; i < header->numbones; ++i)
	{
		const auto bone = header->GetBone(i);

		oldData.emplace_back(
			ScaleBonesBoneData
			{
				{bone->value[0], bone->value[1], bone->value[2]},
				{bone->scale[0], bone->scale[1], bone->scale[2]}
			});

		newData.emplace_back(
			ScaleBonesBoneData
			{
				{bone->value[0]* scale, bone->value[1] * scale, bone->value[2] * scale},
				{bone->scale[0]* scale, bone->scale[1] * scale, bone->scale[2] * scale}
			});
	}

	return std::make_pair(std::move(oldData), std::move(newData));
}

void ApplyScaleBonesData(CStudioModel& studioModel, const std::vector<studiomdl::ScaleBonesBoneData>& data)
{
	const auto header = studioModel.GetStudioHeader();

	for (int i = 0; i < header->numbones; ++i)
	{
		const auto bone = header->GetBone(i);

		const auto& boneData = data[i];

		for (int j = 0; j < boneData.Position.length(); ++j)
		{
			bone->value[j] = boneData.Position[j];
			bone->scale[j] = boneData.Scale[j];
		}
	}
}

const char* ControlToString(const int iControl)
{
	switch (iControl)
	{
	case STUDIO_X:		return "X";
	case STUDIO_Y:		return "Y";
	case STUDIO_Z:		return "Z";
	case STUDIO_XR:		return "XR";
	case STUDIO_YR:		return "YR";
	case STUDIO_ZR:		return "ZR";
	case STUDIO_LX:		return "LX";
	case STUDIO_LY:		return "LY";
	case STUDIO_LZ:		return "LZ";
	case STUDIO_AX:		return "AX";
	case STUDIO_AY:		return "AY";
	case STUDIO_AZ:		return "AZ";
	case STUDIO_AXR:	return "AXR";
	case STUDIO_AYR:	return "AYR";
	case STUDIO_AZR:	return "AZR";

	default:			return nullptr;
	}
}

const char* ControlToStringDescription(const int iControl)
{
	switch (iControl)
	{
	case STUDIO_X:		return "Sequence motiontype flag: Zeroes out movement in the X axis\nBone controller type flag: Sets X offset";
	case STUDIO_Y:		return "Sequence motiontype flag: Zeroes out movement in the Y axis\nBone controller type flag: Sets Y offset";
	case STUDIO_Z:		return "Sequence motiontype flag: Zeroes out movement in the Z axis\nBone controller type flag: Sets Z offset";
	case STUDIO_XR:		return "Bone controller type flag: Sets X rotation\nSequence blendtype: Controls blending range clamping in the X axis";
	case STUDIO_YR:		return "Bone controller type flag: Sets Y rotation\nSequence blendtype: Controls blending range clamping in the Y axis";
	case STUDIO_ZR:		return "Bone controller type flag: Sets Z rotation\nSequence blendtype: Controls blending range clamping in the Z axis";
	case STUDIO_LX:		return "StudioMdl Compiler flag: Sets the sequence's last frame's X axis position to the first frame's position";
	case STUDIO_LY:		return "StudioMdl Compiler flag: Sets the sequence's last frame's Y axis position to the first frame's position";
	case STUDIO_LZ:		return "StudioMdl Compiler flag: Sets the sequence's last frame's Z axis position to the first frame's position";
	case STUDIO_AX:		return "StudioMdl Compiler flag: Extract relative movement distance between the first and current frame in the X axis (Unused)";
	case STUDIO_AY:		return "StudioMdl Compiler flag: Extract relative movement distance between the first and current frame in the Y axis (Unused)";
	case STUDIO_AZ:		return "StudioMdl Compiler flag: Extract relative movement distance between the first and current frame in the Z axis (Unused)";
	case STUDIO_AXR:	return "StudioMdl Compiler flag: Extract relative rotation distance between the first and current frame around the X axis (Unused)";
	case STUDIO_AYR:	return "StudioMdl Compiler flag: Extract relative rotation distance between the first and current frame around the Y axis (Unused)";
	case STUDIO_AZR:	return "StudioMdl Compiler flag: Extract relative rotation distance between the first and current frame around the Z axis (Unused)";

	default:			return nullptr;
	}
}

int StringToControl(const char* const pszString)
{
	assert(pszString);

	if (strcasecmp(pszString, "X") == 0) return STUDIO_X;
	if (strcasecmp(pszString, "Y") == 0) return STUDIO_Y;
	if (strcasecmp(pszString, "Z") == 0) return STUDIO_Z;
	if (strcasecmp(pszString, "XR") == 0) return STUDIO_XR;
	if (strcasecmp(pszString, "YR") == 0) return STUDIO_YR;
	if (strcasecmp(pszString, "ZR") == 0) return STUDIO_ZR;
	if (strcasecmp(pszString, "LX") == 0) return STUDIO_LX;
	if (strcasecmp(pszString, "LY") == 0) return STUDIO_LY;
	if (strcasecmp(pszString, "LZ") == 0) return STUDIO_LZ;
	if (strcasecmp(pszString, "AX") == 0) return STUDIO_AX;
	if (strcasecmp(pszString, "AY") == 0) return STUDIO_AY;
	if (strcasecmp(pszString, "AZ") == 0) return STUDIO_AZ;
	if (strcasecmp(pszString, "AXR") == 0) return STUDIO_AXR;
	if (strcasecmp(pszString, "AYR") == 0) return STUDIO_AYR;
	if (strcasecmp(pszString, "AZR") == 0) return STUDIO_AZR;

	return -1;
}
}