#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <glm/vec3.hpp>

#include "formats/studiomodel/StudioModelFileFormat.hpp"
#include "graphics/OpenGL.hpp"
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
	int Index = -1;
	int ArrayIndex = -1;
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
	int ArrayIndex = -1;
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

	int Activity = -1;
	int ActivityWeight = 0;

	//List of events used for modifying data in UI
	std::vector<std::unique_ptr<SequenceEvent>> Events;

	//List of events used for saving to disk & event playback
	std::vector<SequenceEvent*> SortedEvents;

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

struct TextureData
{
	int Width = 0;
	int Height = 0;
	std::vector<std::byte> Pixels;
	graphics::RGBPalette Palette;
};

struct Texture
{
	std::string Name;
	int Flags = 0;
	TextureData Data;

	int ArrayIndex = -1;

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

	std::vector<std::vector<std::uint8_t>> Transitions;

	const Model* GetModelByBodyPart(const int iBody, const int iBodyPart) const;

	int GetBodyValueForGroup(int compositeValue, int group) const;

	bool CalculateBodygroup(const int iGroup, const int iValue, int& iInOutBodygroup) const;

	std::vector<const studiomdl::Mesh*> ComputeMeshList(const int texture) const;

	void CreateTextures(graphics::TextureLoader& textureLoader);

	void ReplaceTexture(graphics::TextureLoader& textureLoader, Texture* texture, const std::byte* data, const graphics::RGBPalette& pal);

	/**
	*	Reuploads a texture. Useful for making changes made to the texture's pixel, palette or flag data show up in the model itself.
	* *	@param textureLoader Loader to use for texture uploading
	*	@param texture Texture to reupload. Must be a texture that is part of this model.
	*/
	void ReuploadTexture(graphics::TextureLoader& textureLoader, Texture* texture);

	void UpdateFilters(graphics::TextureLoader& textureLoader);

	void ReuploadTextures(graphics::TextureLoader& textureLoader);

	void DeleteTextures(graphics::TextureLoader& textureLoader);

	void RemapTexture(graphics::TextureLoader& textureLoader, int index, int top, int bottom);

	void RemapTextures(graphics::TextureLoader& textureLoader, int top, int bottom);

	std::vector<int> GetRootBoneIndices() const
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

struct RotateBoneData
{
	glm::vec3 Position;
	glm::vec3 Rotation;
};

struct RotateData
{
	std::vector<RotateBoneData> Bones;
};

std::pair<RotateData, RotateData> CalculateRotatedData(const EditableStudioModel& studioModel, glm::vec3 angles);

void ApplyRotateData(EditableStudioModel& studioModel, const RotateData& data);

struct ScaleMeshesData
{
	std::vector<std::vector<glm::vec3>> Vertices;
};

std::pair<ScaleMeshesData, ScaleMeshesData> CalculateScaledMeshesData(const EditableStudioModel& studioModel, const float scale);

void ApplyScaleMeshesData(EditableStudioModel& studioModel, const ScaleMeshesData& data);

struct ScaleHitboxesData
{
	std::vector<std::pair<glm::vec3, glm::vec3>> Hitboxes;
};

std::pair<ScaleHitboxesData, ScaleHitboxesData> CalculateScaledHitboxesData(const EditableStudioModel& studioModel, const float scale);

void ApplyScaleHitboxesData(EditableStudioModel& studioModel, const ScaleHitboxesData& data);

struct ScaleSequenceBBoxesData
{
	std::vector<std::pair<glm::vec3, glm::vec3>> SequenceBBoxes;
};

std::pair<ScaleSequenceBBoxesData, ScaleSequenceBBoxesData> CalculateScaledSequenceBBoxesData(const EditableStudioModel& studioModel, const float scale);

void ApplyScaleSequenceBBoxesData(EditableStudioModel& studioModel, const ScaleSequenceBBoxesData& data);

struct ScaleBonesBoneData
{
	glm::vec3 Position;
	glm::vec3 Scale;
};

struct ScaleBonesData
{
	std::vector<ScaleBonesBoneData> Bones;
};

std::pair<ScaleBonesData, ScaleBonesData> CalculateScaledBonesData(const EditableStudioModel& studioModel, const float scale);

void ApplyScaleBonesData(EditableStudioModel& studioModel, const ScaleBonesData& data);

std::pair<glm::vec3, glm::vec3> CalculateScaledEyePosition(const EditableStudioModel& studioModel, const float scale);

void ApplyScaleEyePosition(EditableStudioModel& studioModel, const glm::vec3& position);

struct ScaleAttachmentsData
{
	std::vector<glm::vec3> Attachments;
};

std::pair<ScaleAttachmentsData, ScaleAttachmentsData> CalculateScaledAttachments(const EditableStudioModel& studioModel, const float scale);

void ApplyScaleAttachments(EditableStudioModel& studioModel, const ScaleAttachmentsData& data);

struct ScaleData
{
	std::optional<ScaleMeshesData> Meshes;
	std::optional<ScaleHitboxesData> Hitboxes;
	std::optional<ScaleSequenceBBoxesData> SequenceBBoxes;
	std::optional<ScaleBonesData> Bones;
	std::optional<glm::vec3> EyePosition;
	std::optional<ScaleAttachmentsData> Attachments;
};

namespace ScaleFlags
{
enum ScaleFlags
{
	None = 0,
	ScaleMeshes = 1 << 0,
	ScaleHitboxes = 1 << 1,
	ScaleSequenceBBoxes = 1 << 2,
	ScaleBones = 1 << 3,
	ScaleEyePosition = 1 << 4,
	ScaleAttachments = 1 << 5,
};
}

std::pair<ScaleData, ScaleData> CalculateScaleData(const EditableStudioModel& studioModel, const float scale, const int flags);

void ApplyScaleData(EditableStudioModel& studioModel, const ScaleData& data);

struct MoveBoneData
{
	int Index;
	glm::vec3 Position;
};

struct MoveData
{
	std::vector<MoveBoneData> BoneData;
};

std::pair<MoveData, MoveData> CalculateMoveData(const EditableStudioModel& studioModel, const glm::vec3 offset);

void ApplyMoveData(EditableStudioModel& studioModel, const MoveData& data);

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

void SortEventsList(std::vector<SequenceEvent*>& events);
}
