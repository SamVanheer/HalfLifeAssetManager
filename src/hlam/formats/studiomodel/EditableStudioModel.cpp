#include <algorithm>
#include <cassert>
#include <limits>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "formats/studiomodel/BoneTransformer.hpp"
#include "formats/studiomodel/EditableStudioModel.hpp"

#include "graphics/GraphicsUtils.hpp"
#include "graphics/TextureLoader.hpp"

#include "utility/mathlib.hpp"

namespace studiomdl
{
EditableStudioModel::~EditableStudioModel() = default;

const StudioSubModel* EditableStudioModel::GetModelByBodyPart(const int iBody, const int iBodyPart) const
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

std::vector<const StudioMesh*> EditableStudioModel::ComputeMeshList(const int texture) const
{
	if (texture == -1)
	{
		return {};
	}

	std::vector<const StudioMesh*> meshes;

	int iBodygroup = 0;

	for (int iBodyPart = 0; iBodyPart < Bodyparts.size(); ++iBodyPart)
	{
		const auto& bodypart = *Bodyparts[iBodyPart];

		for (int iModel = 0; iModel < bodypart.Models.size(); ++iModel)
		{
			CalculateBodygroup(iBodyPart, iModel, iBodygroup);

			const StudioSubModel& model = *GetModelByBodyPart(iBodygroup, iBodyPart);

			for (int iMesh = 0; iMesh < model.Meshes.size(); ++iMesh)
			{
				const auto& mesh = model.Meshes[iMesh];

				//Check each skin family to detect textures used only by alternate skins (e.g. scientist hands)
				for (int skinFamily = 0; skinFamily < SkinFamilies.size(); ++skinFamily)
				{
					if (SkinFamilies[skinFamily][mesh.SkinRef] == texture)
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
	assert(TextureHandles.empty());

	TextureHandles.resize(Textures.size(), GL_INVALID_TEXTURE_ID);

	for (auto& textureId : TextureHandles)
	{
		textureId = textureLoader.CreateTexture();
	}

	UpdateTextures(textureLoader);
}

void EditableStudioModel::UpdateTexture(graphics::TextureLoader& textureLoader, std::size_t index)
{
	if (index >= Textures.size())
	{
		assert(false);
		return;
	}

	auto& texture = *Textures[index];

	graphics::RGBPalette palette{texture.Data.Palette};

	int low, mid, high;

	if (graphics::TryGetRemapColors(texture.Name, low, mid, high))
	{
		graphics::PaletteHueReplace(palette, TopColor, low, mid);

		if (high)
		{
			graphics::PaletteHueReplace(palette, BottomColor, mid + 1, high);
		}
	}

	textureLoader.UploadIndexed8(
		TextureHandles[index],
		texture.Data.Width, texture.Data.Height,
		texture.Data.Pixels.data(),
		palette,
		(texture.Flags & STUDIO_NF_MIPMAPS) != 0,
		(texture.Flags & STUDIO_NF_MASKED) != 0);
}

void EditableStudioModel::UpdateTextures(graphics::TextureLoader& textureLoader)
{
	for (std::size_t index = 0; const auto& texture : Textures)
	{
		UpdateTexture(textureLoader, index++);
	}
}

void EditableStudioModel::DeleteTextures(graphics::TextureLoader& textureLoader)
{
	for (auto& textureId : TextureHandles)
	{
		textureLoader.DeleteTexture(textureId);
		textureId = GL_INVALID_TEXTURE_ID;
	}

	TextureHandles.clear();
}

void EditableStudioModel::UpdateFilters(graphics::TextureLoader& textureLoader)
{
	for (std::size_t i = 0; i < Textures.size(); ++i)
	{
		if (TextureHandles[i] != GL_INVALID_TEXTURE_ID)
		{
			textureLoader.SetFilters(TextureHandles[i], (Textures[i]->Flags & STUDIO_NF_MIPMAPS) != 0);
		}
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

const StudioBone* FindNearestRootBone(const EditableStudioModel& studioModel, const glm::vec3& position)
{
	const StudioBone* nearest = nullptr;

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

std::vector<RotateBoneData> GetRotateData(const EditableStudioModel& studioModel)
{
	std::vector<RotateBoneData> data;

	data.reserve(studioModel.Bones.size());

	for (const auto& bone : studioModel.Bones)
	{
		if (bone->Parent)
		{
			continue;
		}

		data.emplace_back(RotateBoneData
			{
				glm::vec3{bone->Axes[0].Value, bone->Axes[1].Value, bone->Axes[2].Value},
				glm::vec3{bone->Axes[3].Value, bone->Axes[4].Value, bone->Axes[5].Value}
			});
	}

	data.shrink_to_fit();

	return data;
}

void ApplyRotateData(EditableStudioModel& studioModel, const std::vector<RotateBoneData>& data,
	std::optional<glm::vec3> angles)
{
	if (angles)
	{
		//Determine center of model from root bone nearest to average of all root bones
		glm::vec3 center{FindAverageOfRootBones(studioModel)};

		if (const StudioBone* nearest = FindNearestRootBone(studioModel, center); nearest)
		{
			center = {nearest->Axes[0].Value, nearest->Axes[1].Value, nearest->Axes[2].Value};
		}

		angles->x = glm::radians(angles->x);
		angles->y = glm::radians(angles->y);
		angles->z = glm::radians(angles->z);

		const glm::quat anglesRotation{*angles};

		std::size_t boneIndex = 0;

		for (const auto& bone : studioModel.Bones)
		{
			if (bone->Parent)
			{
				continue;
			}

			const auto& boneData = data[boneIndex++];

			glm::vec3 position{boneData.Position};
			glm::vec3 rotation{boneData.Rotation};

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

			for (int j = 0; j < glm::vec3::length(); ++j)
			{
				bone->Axes[j].Value = position[j];
				bone->Axes[j + 3].Value = rotation[j];
			}
		}
	}
	else
	{
		std::size_t boneIndex = 0;

		for (auto& bone : studioModel.Bones)
		{
			if (bone->Parent)
			{
				continue;
			}

			const auto& boneData = data[boneIndex++];

			for (int j = 0; j < glm::vec3::length(); ++j)
			{
				bone->Axes[j].Value = boneData.Position[j];
				bone->Axes[j + 3].Value = boneData.Rotation[j];
			}
		}
	}
}

std::vector<glm::vec3> GetScaleMeshesData(const EditableStudioModel& studioModel)
{
	std::vector<glm::vec3> vertices;

	for (std::size_t i = 0; i < studioModel.Bodyparts.size(); ++i)
	{
		const auto& bodypart = *studioModel.Bodyparts[i];

		for (std::size_t j = 0; j < bodypart.Models.size(); ++j)
		{
			const auto& model = bodypart.Models[j];

			vertices.reserve(vertices.size() + model.Vertices.size());

			for (std::size_t k = 0; k < model.Vertices.size(); ++k)
			{
				vertices.push_back(model.Vertices[k].Vertex);
			}
		}
	}

	// TODO: maybe scale pivots

	return vertices;
}

void ApplyScaleMeshesData(EditableStudioModel& studioModel, const std::vector<glm::vec3>& data,
	std::optional<float> scale)
{
	std::size_t vertexIndex = 0;

	if (scale)
	{
		for (auto& bodypart : studioModel.Bodyparts)
		{
			for (auto& model : bodypart->Models)
			{
				for (auto& vertex : model.Vertices)
				{
					vertex.Vertex = data[vertexIndex++] * (*scale);
				}
			}
		}
	}
	else
	{
		for (auto& bodypart : studioModel.Bodyparts)
		{
			for (auto& model : bodypart->Models)
			{
				for (auto& vertex : model.Vertices)
				{
					vertex.Vertex = data[vertexIndex++];
				}
			}
		}
	}
}

std::vector<std::pair<glm::vec3, glm::vec3>> GetScaleHitboxesData(const EditableStudioModel& studioModel)
{
	std::vector<std::pair<glm::vec3, glm::vec3>> hitboxes;
	hitboxes.reserve(studioModel.Hitboxes.size());

	for (const auto& hitbox : studioModel.Hitboxes)
	{
		hitboxes.emplace_back(std::make_pair(hitbox->Min, hitbox->Max));
	}

	return hitboxes;
}

void ApplyScaleHitboxesData(EditableStudioModel& studioModel, const std::vector<std::pair<glm::vec3, glm::vec3>>& data,
	std::optional<float> scale)
{
	if (scale)
	{
		for (std::size_t i = 0; i < studioModel.Hitboxes.size(); ++i)
		{
			auto& hitbox = *studioModel.Hitboxes[i];
			hitbox.Min = data[i].first * (*scale);
			hitbox.Max = data[i].second * (*scale);
		}
	}
	else
	{
		for (std::size_t i = 0; i < studioModel.Hitboxes.size(); ++i)
		{
			auto& hitbox = *studioModel.Hitboxes[i];
			hitbox.Min = data[i].first;
			hitbox.Max = data[i].second;
		}
	}
}

std::vector<std::pair<glm::vec3, glm::vec3>> GetScaleSequenceBBoxesData(const EditableStudioModel& studioModel)
{
	std::vector<std::pair<glm::vec3, glm::vec3>> data;
	data.reserve(studioModel.Sequences.size());

	for (const auto& sequence : studioModel.Sequences)
	{
		data.emplace_back(std::make_pair(sequence->BBMin, sequence->BBMax));
	}

	return data;
}

void ApplyScaleSequenceBBoxesData(EditableStudioModel& studioModel,
	const std::vector<std::pair<glm::vec3, glm::vec3>>& data, std::optional<float> scale)
{
	if (scale)
	{
		for (std::size_t i = 0; i < studioModel.Sequences.size(); ++i)
		{
			auto& sequence = *studioModel.Sequences[i];
			sequence.BBMin = data[i].first * (*scale);
			sequence.BBMax = data[i].second * (*scale);
		}
	}
	else
	{
		for (std::size_t i = 0; i < studioModel.Sequences.size(); ++i)
		{
			auto& sequence = *studioModel.Sequences[i];
			sequence.BBMin = data[i].first;
			sequence.BBMax = data[i].second;
		}
	}
	
}

std::vector<ScaleBonesBoneData> GetScaleBonesData(const EditableStudioModel& studioModel)
{
	std::vector<ScaleBonesBoneData> data;

	data.reserve(studioModel.Bones.size());

	for (const auto& bone : studioModel.Bones)
	{
		data.emplace_back(
			ScaleBonesBoneData
			{
				{bone->Axes[0].Value, bone->Axes[1].Value, bone->Axes[2].Value},
				{bone->Axes[0].Scale, bone->Axes[1].Scale, bone->Axes[2].Scale}
			});
	}

	return data;
}

void ApplyScaleBonesData(EditableStudioModel& studioModel, const std::vector<ScaleBonesBoneData>& data,
	std::optional<float> scale)
{
	if (scale)
	{
		for (int i = 0; i < studioModel.Bones.size(); ++i)
		{
			auto& bone = *studioModel.Bones[i];

			const auto& boneData = data[i];

			for (int j = 0; j < glm::vec3::length(); ++j)
			{
				bone.Axes[j].Value = boneData.Position[j] * (*scale);
				bone.Axes[j].Scale = boneData.Scale[j] * (*scale);
			}
		}
	}
	else
	{
		for (int i = 0; i < studioModel.Bones.size(); ++i)
		{
			auto& bone = *studioModel.Bones[i];

			const auto& boneData = data[i];

			for (int j = 0; j < glm::vec3::length(); ++j)
			{
				bone.Axes[j].Value = boneData.Position[j];
				bone.Axes[j].Scale = boneData.Scale[j];
			}
		}
	}
}

std::vector<glm::vec3> GetScaleAttachments(const EditableStudioModel& studioModel)
{
	std::vector<glm::vec3> data;
	data.reserve(studioModel.Attachments.size());

	for (const auto& attachment : studioModel.Attachments)
	{
		data.push_back(attachment->Origin);
	}

	return data;
}

void ApplyScaleAttachments(EditableStudioModel& studioModel, const std::vector<glm::vec3>& data,
	std::optional<float> scale)
{
	if (scale)
	{
		for (std::size_t i = 0; i < studioModel.Attachments.size(); ++i)
		{
			studioModel.Attachments[i]->Origin = data[i] * (*scale);
		}
	}
	else
	{
		for (std::size_t i = 0; i < studioModel.Attachments.size(); ++i)
		{
			studioModel.Attachments[i]->Origin = data[i];
		}
	}
}

ScaleData CalculateScaleData(const EditableStudioModel& studioModel, const int flags)
{
	ScaleData data;

	if (flags & ScaleFlags::ScaleMeshes)
	{
		data.Meshes = GetScaleMeshesData(studioModel);
	}

	if (flags & ScaleFlags::ScaleHitboxes)
	{
		data.Hitboxes = GetScaleHitboxesData(studioModel);
	}

	if (flags & ScaleFlags::ScaleSequenceBBoxes)
	{
		data.SequenceBBoxes = GetScaleSequenceBBoxesData(studioModel);
	}

	if (flags & ScaleFlags::ScaleBones)
	{
		data.Bones = GetScaleBonesData(studioModel);
	}

	if (flags & ScaleFlags::ScaleEyePosition)
	{
		data.EyePosition = studioModel.EyePosition;
	}

	if (flags & ScaleFlags::ScaleAttachments)
	{
		data.Attachments = GetScaleAttachments(studioModel);
	}

	return data;
}

void ApplyScaleData(EditableStudioModel& studioModel, const ScaleData& data, std::optional<float> scale)
{
	if (data.Meshes.has_value())
	{
		ApplyScaleMeshesData(studioModel, data.Meshes.value(), scale);
	}

	if (data.Hitboxes.has_value())
	{
		ApplyScaleHitboxesData(studioModel, data.Hitboxes.value(), scale);
	}

	if (data.SequenceBBoxes.has_value())
	{
		ApplyScaleSequenceBBoxesData(studioModel, data.SequenceBBoxes.value(), scale);
	}

	if (data.Bones.has_value())
	{
		ApplyScaleBonesData(studioModel, data.Bones.value(), scale);
	}

	if (data.EyePosition.has_value())
	{
		studioModel.EyePosition = data.EyePosition.value();

		if (scale)
		{
			studioModel.EyePosition *= *scale;
		}
	}

	if (data.Attachments.has_value())
	{
		ApplyScaleAttachments(studioModel, data.Attachments.value(), scale);
	}
}

std::vector<MoveBoneData> GetMoveData(const EditableStudioModel& studioModel)
{
	const auto rootBoneIndices{studioModel.GetRootBoneIndices()};

	std::vector<MoveBoneData> rootBonePositions;

	rootBonePositions.reserve(rootBoneIndices.size());

	for (auto rootBoneIndex : rootBoneIndices)
	{
		const auto& rootBone = *studioModel.Bones[rootBoneIndex];

		rootBonePositions.emplace_back(
			MoveBoneData
			{
				rootBoneIndex,
				{rootBone.Axes[0].Value, rootBone.Axes[1].Value, rootBone.Axes[2].Value}
			}
		);
	}

	return rootBonePositions;
}

void ApplyMoveData(EditableStudioModel& studioModel, const std::vector<MoveBoneData>& rootBonePositions,
	std::optional<glm::vec3> offset)
{
	if (offset)
	{
		for (const auto& data : rootBonePositions)
		{
			auto& bone = *studioModel.Bones[data.Index];

			for (int i = 0; i < glm::vec3::length(); ++i)
			{
				bone.Axes[i].Value = data.Position[i] + (*offset)[i];
			}
		}
	}
	else
	{
		for (const auto& data : rootBonePositions)
		{
			auto& bone = *studioModel.Bones[data.Index];

			for (int i = 0; i < glm::vec3::length(); ++i)
			{
				bone.Axes[i].Value = data.Position[i];
			}
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

void SortEventsList(std::vector<StudioSequenceEvent*>& events)
{
	//Retain relative order of events
	std::stable_sort(events.begin(), events.end(), [](const auto& lhs, const auto& rhs)
		{
			return lhs->Frame < rhs->Frame;
		});
}
}
