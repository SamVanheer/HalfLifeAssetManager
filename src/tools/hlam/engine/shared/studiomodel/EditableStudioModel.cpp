#include <algorithm>

#include "core/shared/Logging.hpp"

#include "engine/shared/studiomodel/EditableStudioModel.hpp"

#include "graphics/TextureLoader.hpp"

namespace studiomdl
{
EditableStudioModel::~EditableStudioModel()
{
	//TODO: need to be sure the context is valid when this is done
	for (auto& texture : Textures)
	{
		glDeleteTextures(1, &texture->TextureId);
		texture->TextureId = 0;
	}
}

Model* EditableStudioModel::GetModelByBodyPart(const int iBody, const int iBodyPart)
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

void EditableStudioModel::CreateTextures(graphics::TextureLoader& textureLoader)
{
	for (auto& texture : Textures)
	{
		GLuint name;

		glBindTexture(GL_TEXTURE_2D, 0);
		glGenTextures(1, &name);

		textureLoader.UploadIndexed8(
			name,
			texture->Width, texture->Height,
			texture->Pixels.data(),
			texture->Palette,
			(texture->Flags & STUDIO_NF_NOMIPS) != 0,
			(texture->Flags & STUDIO_NF_MASKED) != 0);

		texture->TextureId = name;
	}
}

void EditableStudioModel::ReplaceTexture(graphics::TextureLoader& textureLoader, Texture* texture, const byte* data, const graphics::RGBPalette& pal)
{
	textureLoader.UploadIndexed8(
		texture->TextureId,
		texture->Width, texture->Height,
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
		Error("EditableStudioModel::ReuploadTexture: Invalid texture!");
		return;
	}

	ReplaceTexture(textureLoader, texture, texture->Pixels.data(), texture->Palette);
}

void EditableStudioModel::UpdateFilters(graphics::TextureLoader& textureLoader)
{
	for (const auto& texture : Textures)
	{
		if (texture->TextureId)
		{
			glBindTexture(GL_TEXTURE_2D, texture->TextureId);
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
			ReplaceTexture(textureLoader, texture.get(), texture->Pixels.data(), texture->Palette);
		}
	}
}

std::pair<ScaleMeshesData, ScaleMeshesData> CalculateScaledMeshesData(const EditableStudioModel& studioModel, const float scale)
{
	std::vector<std::vector<glm::vec3>> oldVertices;
	std::vector<std::vector<glm::vec3>> newVertices;

	// scale verts
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

	// scale complex hitboxes
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

	// scale bounding boxes
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

void ApplyScaleMeshesData(EditableStudioModel& studioModel, const ScaleMeshesData& data)
{
	// scale verts
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

	// scale complex hitboxes
	for (int i = 0; i < studioModel.Hitboxes.size(); ++i)
	{
		auto& hitbox = *studioModel.Hitboxes[i];

		hitbox.Min = data.Hitboxes[i].first;
		hitbox.Max = data.Hitboxes[i].second;
	}

	// scale bounding boxes
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

	return std::make_pair(ScaleBonesData{std::move(oldData)}, ScaleBonesData{std::move(newData)});
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

	if (flags & ScaleFlags::ScaleBones)
	{
		auto bonesData = CalculateScaledBonesData(studioModel, scale);

		oldData.Bones = std::move(bonesData.first);
		newData.Bones = std::move(bonesData.second);
	}

	return {std::move(oldData), std::move(newData)};
}

void ApplyScaleData(EditableStudioModel& studioModel, const ScaleData& data)
{
	if (data.Meshes.has_value())
	{
		ApplyScaleMeshesData(studioModel, data.Meshes.value());
	}

	if (data.Bones.has_value())
	{
		ApplyScaleBonesData(studioModel, data.Bones.value());
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
