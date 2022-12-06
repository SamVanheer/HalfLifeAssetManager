#include <algorithm>
#include <limits>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "engine/shared/studiomodel/BoneTransformer.hpp"
#include "engine/shared/studiomodel/EditableStudioModel.hpp"

#include "graphics/TextureLoader.hpp"

#include "utility/mathlib.hpp"

namespace studiomdl
{
EditableStudioModel::~EditableStudioModel()
{
	// TODO: rework resource management.
	/*
	//TODO: need to be sure the context is valid when this is done
	for (auto& texture : Textures)
	{
		glDeleteTextures(1, &texture->TextureId);
		texture->TextureId = 0;
	}
	*/
}

const Model* EditableStudioModel::GetModelByBodyPart(const int iBody, const int iBodyPart) const
{
	auto& bodypart = *Bodyparts[iBodyPart];

	int index = iBody / bodypart.Base;
	index = index % static_cast<int>(bodypart.Models.size());

	return &bodypart.Models[index];
}

int EditableStudioModel::GetBodyValueForGroup(int compositeValue, int group) const
{
	if (group >= Bodyparts.size())
	{
		return -1;
	}

	const auto& bodypart = *Bodyparts[group];

	return (compositeValue / bodypart.Base) % static_cast<int>(bodypart.Models.size());
}

bool EditableStudioModel::CalculateBodygroup(const int iGroup, const int iValue, int& iInOutBodygroup) const
{
	if (iGroup > Bodyparts.size())
		return false;

	const auto& bodypart = *Bodyparts[iGroup];

	int current = (iInOutBodygroup / bodypart.Base) % static_cast<int>(bodypart.Models.size());

	if (iValue >= bodypart.Models.size())
		return true;

	iInOutBodygroup = (iInOutBodygroup - (current * bodypart.Base) + (iValue * bodypart.Base));

	return true;
}

std::vector<const studiomdl::Mesh*> EditableStudioModel::ComputeMeshList(const int texture) const
{
	if (texture == -1)
	{
		return {};
	}

	std::vector<const studiomdl::Mesh*> meshes;

	int iBodygroup = 0;

	for (int iBodyPart = 0; iBodyPart < Bodyparts.size(); ++iBodyPart)
	{
		const auto& bodypart = *Bodyparts[iBodyPart];

		for (int iModel = 0; iModel < bodypart.Models.size(); ++iModel)
		{
			CalculateBodygroup(iBodyPart, iModel, iBodygroup);

			const studiomdl::Model& model = *GetModelByBodyPart(iBodygroup, iBodyPart);

			for (int iMesh = 0; iMesh < model.Meshes.size(); ++iMesh)
			{
				const auto& mesh = model.Meshes[iMesh];

				//Check each skin family to detect textures used only by alternate skins (e.g. scientist hands)
				for (int skinFamily = 0; skinFamily < SkinFamilies.size(); ++skinFamily)
				{
					if (SkinFamilies[skinFamily][mesh.SkinRef]->ArrayIndex == texture)
					{
						meshes.push_back(&mesh);
						break;
					}
				}
			}
		}
	}

	return meshes;
}

void EditableStudioModel::CreateTextures(graphics::TextureLoader& textureLoader)
{
	for (auto& texture : Textures)
	{
		texture->TextureId = textureLoader.CreateTexture();;

		auto& data = texture->Data;

		textureLoader.UploadIndexed8(
			texture->TextureId,
			data.Width, data.Height,
			data.Pixels.data(),
			data.Palette,
			(texture->Flags & STUDIO_NF_NOMIPS) != 0,
			(texture->Flags & STUDIO_NF_MASKED) != 0);;
	}
}

void EditableStudioModel::ReplaceTexture(graphics::TextureLoader& textureLoader, Texture* texture, const std::byte* data, const graphics::RGBPalette& pal)
{
	textureLoader.UploadIndexed8(
		texture->TextureId,
		texture->Data.Width, texture->Data.Height,
		data,
		pal,
		(texture->Flags & STUDIO_NF_NOMIPS) != 0,
		(texture->Flags & STUDIO_NF_MASKED) != 0);
}

void EditableStudioModel::ReuploadTexture(graphics::TextureLoader& textureLoader, Texture* texture)
{
	assert(texture);

	//TODO: could use the index to check if it's a member
	if (std::find_if(Textures.begin(), Textures.end(), [=](const auto& candidate)
		{
			return candidate.get() == texture;
		}
	) == Textures.end())
	{
		return;
	}

	ReplaceTexture(textureLoader, texture, texture->Data.Pixels.data(), texture->Data.Palette);
}

void EditableStudioModel::UpdateFilters(graphics::TextureLoader& textureLoader)
{
	for (const auto& texture : Textures)
	{
		if (texture->TextureId)
		{
			textureLoader.SetFilters(texture->TextureId, (texture->Flags & STUDIO_NF_NOMIPS) != 0);
		}
	}
}

void EditableStudioModel::ReuploadTextures(graphics::TextureLoader& textureLoader)
{
	for (const auto& texture : Textures)
	{
		if (texture->TextureId)
		{
			ReplaceTexture(textureLoader, texture.get(), texture->Data.Pixels.data(), texture->Data.Palette);
		}
	}
}

void EditableStudioModel::DeleteTextures(graphics::TextureLoader& textureLoader)
{
	for (auto& texture : Textures)
	{
		textureLoader.DeleteTexture(texture->TextureId);
		texture->TextureId = 0;
	}
}

glm::vec3 FindAverageOfRootBones(const EditableStudioModel& studioModel)
{
	glm::vec3 center{0};

	int count = 0;

	for (const auto& bone : studioModel.Bones)
	{
		if (bone->Parent)
		{
			continue;
		}

		glm::vec3 position{bone->Axes[0].Value, bone->Axes[1].Value, bone->Axes[2].Value};

		center += position;

		++count;
	}

	if (count)
	{
		center /= count;
	}

	return center;
}

const Bone* FindNearestRootBone(const EditableStudioModel& studioModel, const glm::vec3& position)
{
	const Bone* nearest = nullptr;

	float dist = std::numeric_limits<float>::max();

	for (const auto& bone : studioModel.Bones)
	{
		if (bone->Parent)
		{
			continue;
		}

		glm::vec3 bonePosition{bone->Axes[0].Value, bone->Axes[1].Value, bone->Axes[2].Value};

		const float dist2 = glm::length(bonePosition - position);

		if (dist2 < dist)
		{
			dist = dist2;
			nearest = bone.get();
		}
	}

	return nearest;
}

std::pair<RotateData, RotateData> CalculateRotatedData(const EditableStudioModel& studioModel, glm::vec3 angles)
{
	//Determine center of model from root bone nearest to average of all root bones
	glm::vec3 center{FindAverageOfRootBones(studioModel)};

	if (const Bone* nearest = FindNearestRootBone(studioModel, center); nearest)
	{
		center = {nearest->Axes[0].Value, nearest->Axes[1].Value, nearest->Axes[2].Value};
	}

	angles.x = glm::radians(angles.x);
	angles.y = glm::radians(angles.y);
	angles.z = glm::radians(angles.z);

	const glm::quat anglesRotation{angles};

	std::vector<RotateBoneData> oldBoneData;
	std::vector<RotateBoneData> newBoneData;

	oldBoneData.reserve(studioModel.Bones.size());
	newBoneData.reserve(studioModel.Bones.size());

	for (const auto& bone : studioModel.Bones)
	{
		if (bone->Parent)
		{
			continue;
		}

		glm::vec3 position{bone->Axes[0].Value, bone->Axes[1].Value, bone->Axes[2].Value};
		glm::vec3 rotation{bone->Axes[3].Value, bone->Axes[4].Value, bone->Axes[5].Value};

		oldBoneData.emplace_back(
			RotateBoneData
			{
				position, rotation
			});

		//Rotate around center
		position -= center;

		position = anglesRotation * position;

		position += center;

		//Convert euler angles to quaternion
		glm::quat boneRotationQuat{rotation};

		//Apply desired rotation
		boneRotationQuat = anglesRotation * boneRotationQuat;

		//Convert back to euler angles
		rotation = glm::eulerAngles(boneRotationQuat);

		newBoneData.emplace_back(
			RotateBoneData
			{
				position, rotation
			});
	}

	return {{std::move(oldBoneData)}, {std::move(newBoneData)}};
}

void ApplyRotateData(EditableStudioModel& studioModel, const RotateData& data)
{
	std::size_t boneIndex = 0;

	for (int i = 0; i < studioModel.Bones.size(); ++i)
	{
		auto& bone = *studioModel.Bones[i];

		if (bone.Parent)
		{
			continue;
		}

		const auto& boneData = data.Bones[boneIndex++];

		for (int j = 0; j < boneData.Position.length(); ++j)
		{
			bone.Axes[j].Value = boneData.Position[j];
			bone.Axes[j + 3].Value = boneData.Rotation[j];
		}
	}
}

std::pair<ScaleMeshesData, ScaleMeshesData> CalculateScaledMeshesData(const EditableStudioModel& studioModel, const float scale)
{
	std::vector<std::vector<glm::vec3>> oldVertices;
	std::vector<std::vector<glm::vec3>> newVertices;

	for (int i = 0; i < studioModel.Bodyparts.size(); ++i)
	{
		const auto& bodypart = *studioModel.Bodyparts[i];

		oldVertices.reserve(oldVertices.size() + bodypart.Models.size());
		newVertices.reserve(newVertices.size() + bodypart.Models.size());

		for (int j = 0; j < bodypart.Models.size(); ++j)
		{
			const auto& model = bodypart.Models[j];

			std::vector<glm::vec3> oldVerticesList;
			std::vector<glm::vec3> newVerticesList;

			oldVerticesList.reserve(model.Vertices.size());
			newVerticesList.reserve(model.Vertices.size());

			for (int k = 0; k < model.Vertices.size(); ++k)
			{
				oldVerticesList.push_back(model.Vertices[k].Vertex);
				newVerticesList.push_back(model.Vertices[k].Vertex * scale);
			}

			oldVertices.emplace_back(std::move(oldVerticesList));
			newVertices.emplace_back(std::move(newVerticesList));
		}
	}

	// TODO: maybe scale pivots

	return {{std::move(oldVertices)}, {std::move(newVertices)}};
}

void ApplyScaleMeshesData(EditableStudioModel& studioModel, const ScaleMeshesData& data)
{
	int vertexIndex = 0;

	for (int i = 0; i < studioModel.Bodyparts.size(); ++i)
	{
		auto& bodypart = *studioModel.Bodyparts[i];

		for (int j = 0; j < bodypart.Models.size(); ++j)
		{
			auto& model = bodypart.Models[j];

			for (std::size_t k = 0; k < model.Vertices.size(); ++k)
			{
				model.Vertices[k].Vertex = data.Vertices[vertexIndex][k];
			}

			++vertexIndex;
		}
	}
}

std::pair<ScaleHitboxesData, ScaleHitboxesData> CalculateScaledHitboxesData(const EditableStudioModel& studioModel, const float scale)
{
	std::vector<std::pair<glm::vec3, glm::vec3>> oldHitboxes;
	std::vector<std::pair<glm::vec3, glm::vec3>> newHitboxes;

	oldHitboxes.reserve(studioModel.Hitboxes.size());
	newHitboxes.reserve(studioModel.Hitboxes.size());

	for (int i = 0; i < studioModel.Hitboxes.size(); ++i)
	{
		const auto& hitbox = *studioModel.Hitboxes[i];

		oldHitboxes.emplace_back(std::make_pair(hitbox.Min, hitbox.Max));
		newHitboxes.emplace_back(std::make_pair(hitbox.Min * scale, hitbox.Max * scale));
	}

	return {{std::move(oldHitboxes)}, {std::move(newHitboxes)}};
}

void ApplyScaleHitboxesData(EditableStudioModel& studioModel, const ScaleHitboxesData& data)
{
	for (int i = 0; i < studioModel.Hitboxes.size(); ++i)
	{
		auto& hitbox = *studioModel.Hitboxes[i];

		hitbox.Min = data.Hitboxes[i].first;
		hitbox.Max = data.Hitboxes[i].second;
	}
}

std::pair<ScaleSequenceBBoxesData, ScaleSequenceBBoxesData> CalculateScaledSequenceBBoxesData(const EditableStudioModel& studioModel, const float scale)
{
	std::vector<std::pair<glm::vec3, glm::vec3>> oldSequenceBBBoxes;
	std::vector<std::pair<glm::vec3, glm::vec3>> newSequenceBBBoxes;

	oldSequenceBBBoxes.reserve(studioModel.Sequences.size());
	newSequenceBBBoxes.reserve(studioModel.Sequences.size());

	for (int i = 0; i < studioModel.Sequences.size(); ++i)
	{
		const auto& sequence = *studioModel.Sequences[i];

		oldSequenceBBBoxes.emplace_back(std::make_pair(sequence.BBMin, sequence.BBMax));
		newSequenceBBBoxes.emplace_back(std::make_pair(sequence.BBMin * scale, sequence.BBMax * scale));
	}

	return {{std::move(oldSequenceBBBoxes)}, {std::move(newSequenceBBBoxes)}};
}

void ApplyScaleSequenceBBoxesData(EditableStudioModel& studioModel, const ScaleSequenceBBoxesData& data)
{
	for (int i = 0; i < studioModel.Sequences.size(); ++i)
	{
		auto& sequence = *studioModel.Sequences[i];

		sequence.BBMin = data.SequenceBBoxes[i].first;
		sequence.BBMax = data.SequenceBBoxes[i].second;
	}
}

std::pair<ScaleBonesData, ScaleBonesData> CalculateScaledBonesData(const EditableStudioModel& studioModel, const float scale)
{
	std::vector<ScaleBonesBoneData> oldData;
	std::vector<ScaleBonesBoneData> newData;

	oldData.reserve(studioModel.Bones.size());
	newData.reserve(studioModel.Bones.size());

	for (int i = 0; i < studioModel.Bones.size(); ++i)
	{
		const auto& bone = *studioModel.Bones[i];

		oldData.emplace_back(
			ScaleBonesBoneData
			{
				{bone.Axes[0].Value, bone.Axes[1].Value, bone.Axes[2].Value},
				{bone.Axes[0].Scale, bone.Axes[1].Scale, bone.Axes[2].Scale}
			});

		newData.emplace_back(
			ScaleBonesBoneData
			{
				{bone.Axes[0].Value * scale, bone.Axes[1].Value * scale, bone.Axes[2].Value * scale},
				{bone.Axes[0].Scale * scale, bone.Axes[1].Scale * scale, bone.Axes[2].Scale * scale}
			});
	}

	return {{std::move(oldData)}, {std::move(newData)}};
}

void ApplyScaleBonesData(EditableStudioModel& studioModel, const ScaleBonesData& data)
{
	for (int i = 0; i < studioModel.Bones.size(); ++i)
	{
		auto& bone = *studioModel.Bones[i];

		const auto& boneData = data.Bones[i];

		for (int j = 0; j < boneData.Position.length(); ++j)
		{
			bone.Axes[j].Value = boneData.Position[j];
			bone.Axes[j].Scale = boneData.Scale[j];
		}
	}
}

std::pair<glm::vec3, glm::vec3> CalculateScaledEyePosition(const EditableStudioModel& studioModel, const float scale)
{
	return {studioModel.EyePosition, studioModel.EyePosition * scale};
}

void ApplyScaleEyePosition(EditableStudioModel& studioModel, const glm::vec3& position)
{
	studioModel.EyePosition = position;
}

std::pair<ScaleAttachmentsData, ScaleAttachmentsData> CalculateScaledAttachments(const EditableStudioModel& studioModel, const float scale)
{
	std::vector<glm::vec3> oldAttachments;
	std::vector<glm::vec3> newAttachments;

	oldAttachments.reserve(studioModel.Attachments.size());
	newAttachments.reserve(studioModel.Attachments.size());

	for (const auto& attachment : studioModel.Attachments)
	{
		oldAttachments.push_back(attachment->Origin);
		newAttachments.push_back(attachment->Origin * scale);
	}

	return {{std::move(oldAttachments)}, {std::move(newAttachments)}};
}

void ApplyScaleAttachments(EditableStudioModel& studioModel, const ScaleAttachmentsData& data)
{
	for (int i = 0; i < studioModel.Attachments.size(); ++i)
	{
		studioModel.Attachments[i]->Origin = data.Attachments[i];
	}
}

std::pair<ScaleData, ScaleData> CalculateScaleData(const EditableStudioModel& studioModel, const float scale, const int flags)
{
	ScaleData oldData;
	ScaleData newData;

	if (flags & ScaleFlags::ScaleMeshes)
	{
		auto meshData = CalculateScaledMeshesData(studioModel, scale);

		oldData.Meshes = std::move(meshData.first);
		newData.Meshes = std::move(meshData.second);
	}

	if (flags & ScaleFlags::ScaleHitboxes)
	{
		auto hitboxData = CalculateScaledHitboxesData(studioModel, scale);

		oldData.Hitboxes = std::move(hitboxData.first);
		newData.Hitboxes = std::move(hitboxData.second);
	}

	if (flags & ScaleFlags::ScaleSequenceBBoxes)
	{
		auto sequenceData = CalculateScaledSequenceBBoxesData(studioModel, scale);

		oldData.SequenceBBoxes = std::move(sequenceData.first);
		newData.SequenceBBoxes = std::move(sequenceData.second);
	}

	if (flags & ScaleFlags::ScaleBones)
	{
		auto bonesData = CalculateScaledBonesData(studioModel, scale);

		oldData.Bones = std::move(bonesData.first);
		newData.Bones = std::move(bonesData.second);
	}

	if (flags & ScaleFlags::ScaleEyePosition)
	{
		auto data = CalculateScaledEyePosition(studioModel, scale);

		oldData.EyePosition = std::move(data.first);
		newData.EyePosition = std::move(data.second);
	}

	if (flags & ScaleFlags::ScaleAttachments)
	{
		auto data = CalculateScaledAttachments(studioModel, scale);

		oldData.Attachments = std::move(data.first);
		newData.Attachments = std::move(data.second);
	}

	return {std::move(oldData), std::move(newData)};
}

void ApplyScaleData(EditableStudioModel& studioModel, const ScaleData& data)
{
	if (data.Meshes.has_value())
	{
		ApplyScaleMeshesData(studioModel, data.Meshes.value());
	}

	if (data.Hitboxes.has_value())
	{
		ApplyScaleHitboxesData(studioModel, data.Hitboxes.value());
	}

	if (data.SequenceBBoxes.has_value())
	{
		ApplyScaleSequenceBBoxesData(studioModel, data.SequenceBBoxes.value());
	}

	if (data.Bones.has_value())
	{
		ApplyScaleBonesData(studioModel, data.Bones.value());
	}

	if (data.EyePosition.has_value())
	{
		ApplyScaleEyePosition(studioModel, data.EyePosition.value());
	}

	if (data.Attachments.has_value())
	{
		ApplyScaleAttachments(studioModel, data.Attachments.value());
	}
}

std::pair<MoveData, MoveData> CalculateMoveData(const EditableStudioModel& studioModel, const glm::vec3 offset)
{
	const auto rootBoneIndices{studioModel.GetRootBoneIndices()};

	std::vector<MoveBoneData> oldRootBonePositions;
	std::vector<MoveBoneData> newRootBonePositions;

	oldRootBonePositions.reserve(rootBoneIndices.size());
	newRootBonePositions.reserve(rootBoneIndices.size());

	for (auto rootBoneIndex : rootBoneIndices)
	{
		const auto& rootBone = *studioModel.Bones[rootBoneIndex];

		oldRootBonePositions.emplace_back(
			MoveBoneData
			{
				rootBoneIndex,
				{rootBone.Axes[0].Value, rootBone.Axes[1].Value, rootBone.Axes[2].Value}
			}
		);

		newRootBonePositions.emplace_back(
			MoveBoneData
			{
				rootBoneIndex,
				{
					rootBone.Axes[0].Value + offset[0],
					rootBone.Axes[1].Value + offset[1],
					rootBone.Axes[2].Value + offset[2]
				}
			});
	}

	return {{std::move(oldRootBonePositions)}, {std::move(newRootBonePositions)}};
}

void ApplyMoveData(EditableStudioModel& studioModel, const MoveData& data)
{
	for (const auto& data : data.BoneData)
	{
		auto& bone = *studioModel.Bones[data.Index];

		for (int i = 0; i < data.Position.length(); ++i)
		{
			bone.Axes[i].Value = data.Position[i];
		}
	}
}

std::pair<ScaleSTCoordinatesData, ScaleSTCoordinatesData> CalculateScaledSTCoordinatesData(const EditableStudioModel& studioModel,
	const int textureIndex, const int oldWidth, const int oldHeight, const int newWidth, const int newHeight)
{
	//Nothing to do if the sizes match
	if (oldWidth == newWidth && oldHeight == newHeight)
	{
		return {};
	}

	std::optional<double> newWidthFactor;
	std::optional<double> newHeightFactor;

	if (oldWidth != newWidth && oldWidth > 0 && newWidth > 0)
	{
		newWidthFactor = static_cast<double>(newWidth) / oldWidth;
	}

	if (oldHeight != newHeight && oldHeight > 0 && newHeight > 0)
	{
		newHeightFactor = static_cast<double>(newHeight) / oldHeight;
	}

	std::vector<ScaleSTCoordinatesData::STCoordinate> originalCoordinates;
	std::vector<ScaleSTCoordinatesData::STCoordinate> scaledCoordinates;

	for (std::size_t b = 0; b < studioModel.Bodyparts.size(); ++b)
	{
		auto& bodypart = *studioModel.Bodyparts[b];

		for (std::size_t m = 0; m < bodypart.Models.size(); ++m)
		{
			auto& model = bodypart.Models[m];

			for (std::size_t n = 0; n < model.Meshes.size(); ++n)
			{
				auto& mesh = model.Meshes[n];

				if (mesh.SkinRef == textureIndex)
				{
					auto cmds = mesh.Triangles.data();

					//Try to avoid reallocations by guessing the amount of vertices stored
					originalCoordinates.reserve(originalCoordinates.size() + (mesh.NumTriangles * 3));
					scaledCoordinates.reserve(scaledCoordinates.size() + (mesh.NumTriangles * 3));

					for (int cmd = std::abs(*cmds++); cmd > 0; cmd = std::abs(*cmds++))
					{
						while (cmd-- > 0)
						{
							short s = cmds[2];
							short t = cmds[3];

							originalCoordinates.push_back({s, t});

							//Rescale coordinates only if necessary to avoid loss of data
							if (newWidthFactor.has_value())
							{
								s = static_cast<short>(s * newWidthFactor.value());
							}

							if (newHeightFactor.has_value())
							{
								t = static_cast<short>(t * newHeightFactor.value());
							}

							scaledCoordinates.push_back({s, t});

							cmds += 4;
						}
					}
				}
			}
		}
	}

	originalCoordinates.shrink_to_fit();
	scaledCoordinates.shrink_to_fit();

	return {ScaleSTCoordinatesData{std::move(originalCoordinates)}, ScaleSTCoordinatesData{std::move(scaledCoordinates)}};
}

void ApplyScaledSTCoordinatesData(const EditableStudioModel& studioModel, const int textureIndex, const ScaleSTCoordinatesData& data)
{
	//Nothing to do if no coordinates were modified
	if (data.Coordinates.empty())
	{
		return;
	}

	auto coordinates = data.Coordinates.begin();

	for (std::size_t b = 0; b < studioModel.Bodyparts.size(); ++b)
	{
		auto& bodypart = *studioModel.Bodyparts[b];

		for (std::size_t m = 0; m < bodypart.Models.size(); ++m)
		{
			auto& model = bodypart.Models[m];

			for (std::size_t n = 0; n < model.Meshes.size(); ++n)
			{
				auto& mesh = model.Meshes[n];

				if (mesh.SkinRef == textureIndex)
				{
					auto cmds = mesh.Triangles.data();

					for (int cmd = std::abs(*cmds++); cmd > 0; cmd = std::abs(*cmds++))
					{
						while (cmd-- > 0)
						{
							short s = cmds[2];
							short t = cmds[3];

							cmds[2] = coordinates->S;
							cmds[3] = coordinates->T;

							cmds += 4;
							++coordinates;
						}
					}
				}
			}
		}
	}
}

void SortEventsList(std::vector<SequenceEvent*>& events)
{
	//Retain relative order of events
	std::stable_sort(events.begin(), events.end(), [](const auto& lhs, const auto& rhs)
		{
			return lhs->Frame < rhs->Frame;
		});
}
}
