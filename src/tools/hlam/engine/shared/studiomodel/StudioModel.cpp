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

StudioModel::StudioModel(std::string&& fileName, studio_ptr<studiohdr_t>&& studioHeader, studio_ptr<studiohdr_t>&& textureHeader,
	std::vector<studio_ptr<studioseqhdr_t>>&& sequenceHeaders, bool isDol)
	: _fileName(std::move(fileName))
	, _studioHeader(std::move(studioHeader))
	, _textureHeader(std::move(textureHeader))
	, _sequenceHeaders(std::move(sequenceHeaders))
	, _isDol(isDol)
{
	assert(_studioHeader);
}

StudioModel::~StudioModel()
{
	glDeleteTextures(_textures.size(), _textures.data());
	_textures.clear();
}

mstudioanim_t* StudioModel::GetAnim(const mstudioseqdesc_t* pseqdesc) const
{
	mstudioseqgroup_t* pseqgroup = _studioHeader->GetSequenceGroup(pseqdesc->seqgroup);

	if (pseqdesc->seqgroup == 0)
	{
		return (mstudioanim_t*)((byte*)_studioHeader.get() + pseqgroup->unused2 + pseqdesc->animindex);
	}

	return (mstudioanim_t*)((byte*)_sequenceHeaders[pseqdesc->seqgroup - 1].get() + pseqdesc->animindex);
}

mstudiomodel_t* StudioModel::GetModelByBodyPart(const int iBody, const int iBodyPart) const
{
	mstudiobodyparts_t* pbodypart = _studioHeader->GetBodypart(iBodyPart);

	int index = iBody / pbodypart->base;
	index = index % pbodypart->nummodels;

	return (mstudiomodel_t*)((byte*)_studioHeader.get() + pbodypart->modelindex) + index;
}

int StudioModel::GetBodyValueForGroup(int compositeValue, int group) const
{
	if (group >= _studioHeader->numbodyparts)
	{
		return -1;
	}

	const mstudiobodyparts_t* const bodyPart = _studioHeader->GetBodypart(group);

	return (compositeValue / bodyPart->base) % bodyPart->nummodels;
}

bool StudioModel::CalculateBodygroup(const int iGroup, const int iValue, int& iInOutBodygroup) const
{
	if (iGroup > _studioHeader->numbodyparts)
		return false;

	const mstudiobodyparts_t* const pbodypart = _studioHeader->GetBodypart(iGroup);

	int current = (iInOutBodygroup / pbodypart->base) % pbodypart->nummodels;

	if (iValue >= pbodypart->nummodels)
		return true;

	iInOutBodygroup = (iInOutBodygroup - (current * pbodypart->base) + (iValue * pbodypart->base));

	return true;
}

GLuint StudioModel::GetTextureId(const int iIndex) const
{
	if (iIndex < 0 || iIndex >= _textures.size())
	{
		return GL_INVALID_TEXTURE_ID;
	}

	return _textures[iIndex];
}

void StudioModel::CreateTextures(graphics::TextureLoader& textureLoader)
{
	const auto textureHeader = GetTextureHeader();

	if (textureHeader->textureindex > 0)
	{
		byte* pIn = reinterpret_cast<byte*>(textureHeader);

		for (int i = 0; i < textureHeader->numtextures; ++i)
		{
			GLuint name;

			glBindTexture(GL_TEXTURE_2D, 0);
			glGenTextures(1, &name);

			const auto& texture = *textureHeader->GetTexture(i);

			if (_isDol)
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

			_textures.emplace_back(name);
		}
	}
}

void StudioModel::ReplaceTexture(graphics::TextureLoader& textureLoader, mstudiotexture_t* ptexture, const byte* data, const byte* pal, GLuint textureId)
{
	textureLoader.UploadIndexed8(
		textureId,
		ptexture->width, ptexture->height,
		data,
		pal,
		(ptexture->flags & STUDIO_NF_NOMIPS) != 0,
		(ptexture->flags & STUDIO_NF_MASKED) != 0);
}

void StudioModel::ReuploadTexture(graphics::TextureLoader& textureLoader, mstudiotexture_t* ptexture)
{
	assert(ptexture);

	auto header = GetTextureHeader();

	const int index = ptexture - header->GetTextures();

	if (index < 0 || index >= header->numtextures)
	{
		Error("StudioModel::ReuploadTexture: Invalid texture!");
		return;
	}

	ReplaceTexture(textureLoader, ptexture,
		header->GetData() + ptexture->index, header->GetData() + ptexture->index + ptexture->width * ptexture->height, _textures[index]);
}

void StudioModel::UpdateFilters(graphics::TextureLoader& textureLoader)
{
	if (_textures.empty())
	{
		//No textures loaded yet, do nothing
		return;
	}

	const auto textureHeader{GetTextureHeader()};

	for (int i = 0; i < textureHeader->numtextures; ++i)
	{
		glBindTexture(GL_TEXTURE_2D, _textures[i]);
		textureLoader.SetFilters(_textures[i], (textureHeader->GetTexture(i)->flags & STUDIO_NF_NOMIPS) != 0);
	}
}

void StudioModel::ReuploadTextures(graphics::TextureLoader& textureLoader)
{
	if (_textures.empty())
	{
		//No textures loaded yet, do nothing
		return;
	}

	auto header = GetTextureHeader();

	for (int i = 0; i < header->numtextures; ++i)
	{
		const auto ptexture = header->GetTexture(i);

		ReplaceTexture(textureLoader, ptexture,
			header->GetData() + ptexture->index, header->GetData() + ptexture->index + ptexture->width * ptexture->height, _textures[i]);
	}
}

std::pair<ScaleMeshesData, ScaleMeshesData> CalculateScaledMeshesData(const StudioModel& studioModel, const float scale)
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

void ApplyScaleMeshesData(StudioModel& studioModel, const ScaleMeshesData& data)
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

std::pair<std::vector<ScaleBonesBoneData>, std::vector<ScaleBonesBoneData>> CalculateScaledBonesData(const StudioModel& studioModel, const float scale)
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
				{bone->value[0] * scale, bone->value[1] * scale, bone->value[2] * scale},
				{bone->scale[0] * scale, bone->scale[1] * scale, bone->scale[2] * scale}
			});
	}

	return std::make_pair(std::move(oldData), std::move(newData));
}

void ApplyScaleBonesData(StudioModel& studioModel, const std::vector<studiomdl::ScaleBonesBoneData>& data)
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