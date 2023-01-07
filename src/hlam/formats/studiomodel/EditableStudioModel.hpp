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
struct StudioBoneController
{
	int Type = 0;
	float Start = 0;
	float End = 0;
	int Rest = 0;
	int Index = -1;
	int ArrayIndex = -1;
};

struct StudioBoneAxisData
{
	StudioBoneController* Controller = nullptr;
	float Value = 0;
	float Scale = 0;
};

struct StudioBone
{
	std::string Name;
	StudioBone* Parent = nullptr;
	int Flags = 0;
	std::array<StudioBoneAxisData, STUDIO_NUM_COORDINATE_AXES> Axes;

	//Index of this bone. Always use this for indexed operations
	int ArrayIndex = -1;
};

struct StudioHitbox
{
	StudioBone* Bone = nullptr;
	int Group = 0;

	glm::vec3 Min{0};
	glm::vec3 Max{0};
};

struct StudioSequenceGroup
{
	std::string Label;
};

struct StudioSequenceEvent
{
	int Frame = 0;
	int EventId = 0;
	int Type = 0;
	std::string Options;
};

struct StudioAnimation
{
	//std::array<std::vector<short>, STUDIO_MAX_PER_BONE_CONTROLLERS> Data;
	std::array<std::vector<mstudioanimvalue_t>, STUDIO_NUM_COORDINATE_AXES> Data;
};

struct StudioSequenceBlendData
{
	int Type = 0;
	float Start = 0;
	float End = 0;
};

struct StudioSequencePivot
{
	glm::vec3 Origin{0};
	int Start = 0;
	int End = 0;
};

struct StudioSequence
{
	std::string Label;

	float FPS = 0;
	int Flags = 0;

	int Activity = -1;
	int ActivityWeight = 0;

	//List of events used for modifying data in UI
	std::vector<std::unique_ptr<StudioSequenceEvent>> Events;

	//List of events used for saving to disk & event playback
	std::vector<StudioSequenceEvent*> SortedEvents;

	int NumFrames = 0;

	std::vector<StudioSequencePivot> Pivots;

	int MotionType = 0;
	int MotionBone = 0;

	glm::vec3 LinearMovement{0};

	glm::vec3 BBMin{0};
	glm::vec3 BBMax{0};

	std::vector<std::vector<StudioAnimation>> AnimationBlends;

	std::array<StudioSequenceBlendData, SequenceBlendCount> BlendData;

	int EntryNode = 0;
	int ExitNode = 0;
	int NodeFlags = 0;

	int NextSequence = 0;
};

struct StudioAttachment
{
	std::string Name;

	int Type = 0;

	StudioBone* Bone = nullptr;

	glm::vec3 Origin{0};

	std::array<glm::vec3, STUDIO_ATTACH_NUM_VECTORS> Vectors{{glm::vec3{0}, glm::vec3{0}, glm::vec3{0}}};
};

struct StudioMesh
{
	std::vector<short> Triangles;
	
	int NumTriangles = 0;
	int NumNorms = 0;
	int SkinRef = 0;
};

struct StudioModelVertexInfo
{
	glm::vec3 Vertex{0};
	StudioBone* Bone = nullptr;
};

struct StudioSubModel
{
	std::string Name;

	int Type = 0;
	float BoundingRadius = 0;

	std::vector<StudioMesh> Meshes;
	std::vector<StudioModelVertexInfo> Vertices;
	std::vector<StudioModelVertexInfo> Normals;
};

struct StudioBodypart
{
	std::string Name;
	int Base = 0;
	std::vector<StudioSubModel> Models;
};

struct StudioTextureData
{
	int Width = 0;
	int Height = 0;
	std::vector<std::byte> Pixels;
	graphics::RGBPalette Palette;
};

struct StudioTexture
{
	std::string Name;
	int Flags = 0;
	StudioTextureData Data;

	int ArrayIndex = -1;

	GLuint TextureId = 0;
};

constexpr std::array<StudioSequenceBlendData, SequenceBlendCount> CounterStrikeBlendRanges{{{0, -180, 180}, {0, -45, 45}}};

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

	std::vector<std::unique_ptr<StudioBone>> Bones;
	std::vector<std::unique_ptr<StudioBoneController>> BoneControllers;
	std::vector<std::unique_ptr<StudioHitbox>> Hitboxes;
	std::vector<std::unique_ptr<StudioSequenceGroup>> SequenceGroups;
	std::vector<std::unique_ptr<StudioSequence>> Sequences;
	std::vector<std::unique_ptr<StudioAttachment>> Attachments;
	std::vector<std::unique_ptr<StudioBodypart>> Bodyparts;

	std::vector<std::unique_ptr<StudioTexture>> Textures;
	std::vector<std::vector<StudioTexture*>> SkinFamilies;

	std::vector<std::vector<std::uint8_t>> Transitions;

	// Used for remapping; not stored in the model.
	int TopColor = 0;
	int BottomColor = 0;

	const StudioSubModel* GetModelByBodyPart(const int iBody, const int iBodyPart) const;

	int GetBodyValueForGroup(int compositeValue, int group) const;

	bool CalculateBodygroup(const int iGroup, const int iValue, int& iInOutBodygroup) const;

	std::vector<const StudioMesh*> ComputeMeshList(const int texture) const;

	void CreateTextures(graphics::TextureLoader& textureLoader);

	/**
	*	(Re)uploads a texture. Useful for making changes made to the texture's pixel, palette or flag data show up in the model itself.
	* *	@param textureLoader Loader to use for texture uploading
	*	@param texture Texture to reupload. Must be a texture that is part of this model.
	*/
	void UpdateTexture(graphics::TextureLoader& textureLoader, std::size_t index);

	void UpdateTextures(graphics::TextureLoader& textureLoader);

	void DeleteTextures(graphics::TextureLoader& textureLoader);

	void UpdateFilters(graphics::TextureLoader& textureLoader);

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

std::vector<RotateBoneData> GetRotateData(const EditableStudioModel& studioModel);

void ApplyRotateData(EditableStudioModel& studioModel, const std::vector<RotateBoneData>& data,
	std::optional<glm::vec3> angles);

std::vector<glm::vec3> GetScaleMeshesData(const EditableStudioModel& studioModel);

void ApplyScaleMeshesData(EditableStudioModel& studioModel, const std::vector<glm::vec3>& data,
	std::optional<float> scale);

std::vector<std::pair<glm::vec3, glm::vec3>> GetScaleHitboxesData(const EditableStudioModel& studioModel);

void ApplyScaleHitboxesData(EditableStudioModel& studioModel, const std::vector<std::pair<glm::vec3, glm::vec3>>& data,
	std::optional<float> scale);

std::vector<std::pair<glm::vec3, glm::vec3>> GetScaleSequenceBBoxesData(const EditableStudioModel& studioModel);

void ApplyScaleSequenceBBoxesData(EditableStudioModel& studioModel,
	const std::vector<std::pair<glm::vec3, glm::vec3>>& data, std::optional<float> scale);

struct ScaleBonesBoneData
{
	glm::vec3 Position;
	glm::vec3 Scale;
};

std::vector<ScaleBonesBoneData> GetScaleBonesData(const EditableStudioModel& studioModel);

void ApplyScaleBonesData(EditableStudioModel& studioModel, const std::vector<ScaleBonesBoneData>& data,
	std::optional<float> scale);

std::vector<glm::vec3> GetScaleAttachments(const EditableStudioModel& studioModel);

void ApplyScaleAttachments(EditableStudioModel& studioModel, const std::vector<glm::vec3>& data,
	std::optional<float> scale);

struct ScaleData
{
	std::optional<std::vector<glm::vec3>> Meshes;
	std::optional<std::vector<std::pair<glm::vec3, glm::vec3>>> Hitboxes;
	std::optional<std::vector<std::pair<glm::vec3, glm::vec3>>> SequenceBBoxes;
	std::optional<std::vector<ScaleBonesBoneData>> Bones;
	std::optional<glm::vec3> EyePosition;
	std::optional<std::vector<glm::vec3>> Attachments;
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

ScaleData CalculateScaleData(const EditableStudioModel& studioModel, const int flags);

void ApplyScaleData(EditableStudioModel& studioModel, const ScaleData& data, std::optional<float> scale);

struct MoveBoneData
{
	int Index;
	glm::vec3 Position;
};

std::vector<MoveBoneData> GetMoveData(const EditableStudioModel& studioModel);

void ApplyMoveData(EditableStudioModel& studioModel, const std::vector<MoveBoneData>& rootBonePositions,
	std::optional<glm::vec3> offset);

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

void SortEventsList(std::vector<StudioSequenceEvent*>& events);
}
