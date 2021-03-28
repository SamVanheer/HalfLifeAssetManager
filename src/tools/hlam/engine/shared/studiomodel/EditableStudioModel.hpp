#pragma once

#include <array>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <glm/vec3.hpp>

#include <GL/glew.h>

#include "core/shared/Const.hpp"
#include "engine/shared/studiomodel/StudioModelFileFormat.hpp"
#include "graphics/Palette.hpp"

namespace graphics
{
class TextureLoader;
}

namespace studiomdl
{
struct BoneController
{
	int Type = 0;
	float Start = 0;
	float End = 0;
	int Rest = 0;
	int Index = 0;
	int ArrayIndex = 0;
};

struct BoneAxisData
{
	BoneController* Controller = nullptr;
	float Value = 0;
	float Scale = 0;
};

struct Bone
{
	std::string Name;
	Bone* Parent = nullptr;
	int Flags = 0;
	std::array<BoneAxisData, STUDIO_NUM_COORDINATE_AXES> Axes;

	//Index of this bone. Always use this for indexed operations
	int ArrayIndex = 0;
};

struct Hitbox
{
	Bone* Bone = nullptr;
	int Group = 0;

	glm::vec3 Min{0};
	glm::vec3 Max{0};
};

struct SequenceGroup
{
	std::string Label;
};

struct SequenceEvent
{
	int Frame = 0;
	int EventId = 0;
	int Type = 0;
	std::string Options;
};

struct Animation
{
	//std::array<std::vector<short>, STUDIO_MAX_PER_BONE_CONTROLLERS> Data;
	std::array<std::vector<mstudioanimvalue_t>, STUDIO_NUM_COORDINATE_AXES> Data;
};

struct SequenceBlendData
{
	int Type = 0;
	float Start = 0;
	float End = 0;
};

struct SequencePivot
{
	glm::vec3 Origin{0};
	int Start = 0;
	int End = 0;
};

struct Sequence
{
	std::string Label;

	float FPS = 0;
	int Flags = 0;

	int Activity = 0;
	int ActivityWeight = 0;

	std::vector<SequenceEvent> Events;

	int NumFrames = 0;

	std::vector<SequencePivot> Pivots;

	int MotionType = 0;
	int MotionBone = 0;

	glm::vec3 LinearMovement{0};

	glm::vec3 BBMin{0};
	glm::vec3 BBMax{0};

	std::vector<std::vector<Animation>> AnimationBlends;

	std::array<SequenceBlendData, SequenceBlendCount> BlendData;

	int EntryNode = 0;
	int ExitNode = 0;
	int NodeFlags = 0;

	int NextSequence = 0;
};

struct Attachment
{
	std::string Name;

	int Type = 0;

	Bone* Bone = nullptr;

	glm::vec3 Origin{0};

	std::array<glm::vec3, STUDIO_ATTACH_NUM_VECTORS> Vectors{{glm::vec3{0}, glm::vec3{0}, glm::vec3{0}}};
};

struct Mesh
{
	std::vector<short> Triangles;
	
	int NumTriangles = 0;
	int NumNorms = 0;
	int SkinRef = 0;
};

struct ModelVertexInfo
{
	glm::vec3 Vertex{0};
	Bone* Bone = nullptr;
};

struct Model
{
	std::string Name;

	int Type = 0;
	float BoundingRadius = 0;

	std::vector<Mesh> Meshes;
	std::vector<ModelVertexInfo> Vertices;
	std::vector<ModelVertexInfo> Normals;
};

struct Bodypart
{
	std::string Name;
	int Base = 0;
	std::vector<Model> Models;
};

struct Texture
{
	std::string Name;
	int Flags = 0;
	int Width = 0;
	int Height = 0;

	int ArrayIndex = 0;

	std::vector<byte> Pixels;
	graphics::RGBPalette Palette;

	GLuint TextureId = 0;
};

constexpr std::array<SequenceBlendData, SequenceBlendCount> CounterStrikeBlendRanges{{{0, -180, 180}, {0, -45, 45}}};

/**
*	@brief Contains studiomodel data in a format that can be easily edited
*/
class EditableStudioModel
{
public:
	EditableStudioModel() = default;
	~EditableStudioModel();

	EditableStudioModel(const EditableStudioModel&) = delete;
	EditableStudioModel& operator=(const EditableStudioModel&) = delete;

	EditableStudioModel(EditableStudioModel&&) = default;
	EditableStudioModel& operator=(EditableStudioModel&&) = default;

	glm::vec3 EyePosition{0};

	glm::vec3 BoundingMin{0};
	glm::vec3 BoundingMax{0};
			  
	glm::vec3 ClippingMin{0};
	glm::vec3 ClippingMax{0};

	int Flags = 0;

	std::vector<std::unique_ptr<Bone>> Bones;
	std::vector<std::unique_ptr<BoneController>> BoneControllers;
	std::vector<std::unique_ptr<Hitbox>> Hitboxes;
	std::vector<std::unique_ptr<SequenceGroup>> SequenceGroups;
	std::vector<std::unique_ptr<Sequence>> Sequences;
	std::vector<std::unique_ptr<Attachment>> Attachments;
	std::vector<std::unique_ptr<Bodypart>> Bodyparts;

	std::vector<std::unique_ptr<Texture>> Textures;
	std::vector<std::vector<Texture*>> SkinFamilies;

	std::vector<std::vector<byte>> Transitions;

	Model* GetModelByBodyPart(const int iBody, const int iBodyPart);

	int GetBodyValueForGroup(int compositeValue, int group) const;

	bool CalculateBodygroup(const int iGroup, const int iValue, int& iInOutBodygroup) const;

	void CreateTextures(graphics::TextureLoader& textureLoader);

	void ReplaceTexture(graphics::TextureLoader& textureLoader, Texture* texture, const byte* data, const graphics::RGBPalette& pal);

	/**
	*	Reuploads a texture. Useful for making changes made to the texture's pixel, palette or flag data show up in the model itself.
	* *	@param textureLoader Loader to use for texture uploading
	*	@param texture Texture to reupload. Must be a texture that is part of this model.
	*/
	void ReuploadTexture(graphics::TextureLoader& textureLoader, Texture* texture);

	void UpdateFilters(graphics::TextureLoader& textureLoader);

	void ReuploadTextures(graphics::TextureLoader& textureLoader);

	std::vector<int> GetRootBoneIndices()
	{
		std::vector<int> bones;

		for (int i = 0; i < Bones.size(); ++i)
		{
			const auto& bone = *Bones[i];

			if (!bone.Parent)
			{
				bones.emplace_back(i);
			}
		}

		return bones;
	}

	std::optional<std::pair<int, int>> FindBoneControllerIsAttachedTo(int boneControllerIndex)
	{
		if (boneControllerIndex >= 0 && boneControllerIndex < BoneControllers.size())
		{
			auto controller = BoneControllers[boneControllerIndex].get();

			for (auto& bone : Bones)
			{
				for (int i = 0; i < bone->Axes.size(); ++i)
				{
					if (bone->Axes[i].Controller == controller)
					{
						return {{bone->ArrayIndex, i}};
					}
				}
			}
		}

		return {};
	}
};

struct ScaleMeshesData
{
	std::vector<std::vector<glm::vec3>> Vertices;
	std::vector<std::pair<glm::vec3, glm::vec3>> Hitboxes;
	std::vector<std::pair<glm::vec3, glm::vec3>> SequenceBBoxes;
};

std::pair<ScaleMeshesData, ScaleMeshesData> CalculateScaledMeshesData(const EditableStudioModel& studioModel, const float scale);

void ApplyScaleMeshesData(EditableStudioModel& studioModel, const ScaleMeshesData& data);

struct ScaleBonesBoneData
{
	glm::vec3 Position;
	glm::vec3 Scale;
};

std::pair<std::vector<ScaleBonesBoneData>, std::vector<ScaleBonesBoneData>> CalculateScaledBonesData(const EditableStudioModel& studioModel, const float scale);

void ApplyScaleBonesData(EditableStudioModel& studioModel, const std::vector<studiomdl::ScaleBonesBoneData>& data);

struct ScaleSTCoordinatesData
{
	struct STCoordinate
	{
		short S;
		short T;
	};

	std::vector<STCoordinate> Coordinates;

	ScaleSTCoordinatesData() = default;

	ScaleSTCoordinatesData(std::vector<STCoordinate>&& coordinates)
		: Coordinates(std::move(coordinates))
	{
	}

	ScaleSTCoordinatesData(ScaleSTCoordinatesData&&) = default;
	ScaleSTCoordinatesData& operator=(ScaleSTCoordinatesData&&) = default;

	ScaleSTCoordinatesData(const ScaleSTCoordinatesData&) = delete;
	ScaleSTCoordinatesData& operator=(const ScaleSTCoordinatesData&) = delete;
};

std::pair<ScaleSTCoordinatesData, ScaleSTCoordinatesData> CalculateScaledSTCoordinatesData(const EditableStudioModel& studioModel,
	const int textureIndex, const int oldWidth, const int oldHeight, const int newWidth, const int newHeight);

void ApplyScaledSTCoordinatesData(const EditableStudioModel& studioModel, const int textureIndex, const ScaleSTCoordinatesData& data);
}
