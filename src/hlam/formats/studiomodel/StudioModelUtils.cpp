#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>

#include "formats/studiomodel/StudioModelUtils.hpp"

#include "utility/Platform.hpp"
#include "utility/StringUtils.hpp"

namespace studiomdl
{
namespace
{
std::vector<std::unique_ptr<BoneController>> ConvertBoneControllersToEditable(const StudioModel& studioModel)
{
	auto header = studioModel.GetStudioHeader();

	std::vector<std::unique_ptr<BoneController>> result;

	result.reserve(header->numbonecontrollers);

	for (int i = 0; i < header->numbonecontrollers; ++i)
	{
		auto source = header->GetBoneController(i);

		BoneController controller
		{
			source->type,
			source->start,
			source->end,
			source->rest,
			source->index,
			i
		};

		result.push_back(std::make_unique<BoneController>(controller));
	}

	return result;
}

std::vector<std::unique_ptr<Bone>> ConvertBonesToEditable(const StudioModel& studioModel, std::vector<std::unique_ptr<BoneController>>& boneControllers)
{
	auto header = studioModel.GetStudioHeader();

	std::vector<std::unique_ptr<Bone>> result;

	result.reserve(header->numbones);

	for (int i = 0; i < header->numbones; ++i)
	{
		auto source = header->GetBone(i);

		std::array<BoneAxisData, STUDIO_NUM_COORDINATE_AXES> axisData{};

		for (int j = 0; j < axisData.size(); ++j)
		{
			auto& data = axisData[j];

			if (source->bonecontroller[j] != -1)
			{
				data.Controller = boneControllers[source->bonecontroller[j]].get();
			}

			data.Value = source->value[j];
			data.Scale = source->scale[j];
		}

		Bone bone
		{
			source->name,
			nullptr,
			source->flags,
			axisData,
			i
		};

		result.push_back(std::make_unique<Bone>(bone));
	}

	//Patch up parent bones
	for (int i = 0; i < header->numbones; ++i)
	{
		auto source = header->GetBone(i);

		if (source->parent != -1)
		{
			result[i]->Parent = result[source->parent].get();
		}
	}

	return result;
}

std::vector<std::unique_ptr<Hitbox>> ConvertHitboxesToEditable(const StudioModel& studioModel, std::vector<std::unique_ptr<Bone>>& bones)
{
	auto header = studioModel.GetStudioHeader();

	std::vector<std::unique_ptr<Hitbox>> result;

	result.reserve(header->numhitboxes);

	for (int i = 0; i < header->numhitboxes; ++i)
	{
		auto source = header->GetHitBox(i);

		Hitbox hitbox
		{
			bones[source->bone].get(),
			source->group,
			source->bbmin,
			source->bbmax
		};

		result.push_back(std::make_unique<Hitbox>(hitbox));
	}

	return result;
}

std::vector<std::unique_ptr<SequenceGroup>> ConvertSequenceGroupsToEditable(const StudioModel& studioModel)
{
	auto header = studioModel.GetStudioHeader();

	std::vector<std::unique_ptr<SequenceGroup>> result;

	//Only copy the first group (main file)
	const int numseqgroups = 1; //header->numseqgroups;

	result.reserve(numseqgroups);

	for (int i = 0; i < numseqgroups; ++i)
	{
		auto source = header->GetSequenceGroup(i);

		SequenceGroup group
		{
			source->label
		};

		result.push_back(std::make_unique<SequenceGroup>(std::move(group)));
	}

	return result;
}

std::vector<std::unique_ptr<SequenceEvent>> ConvertEventsToEditable(const StudioModel& studioModel, const mstudioseqdesc_t& sequence)
{
	auto header = studioModel.GetStudioHeader();

	std::vector<std::unique_ptr<SequenceEvent>> result;

	result.reserve(sequence.numevents);

	for (int i = 0; i < sequence.numevents; ++i)
	{
		auto source = reinterpret_cast<const mstudioevent_t*>(header->GetData() + sequence.eventindex) + i;

		SequenceEvent event
		{
			source->frame,
			source->event,
			source->type,
			source->options
		};

		result.push_back(std::make_unique<SequenceEvent>(std::move(event)));
	}

	return result;
}

std::vector<SequencePivot> ConvertPivotsToEditable(const StudioModel& studioModel, const mstudioseqdesc_t& sequence)
{
	auto header = studioModel.GetStudioHeader();

	std::vector<SequencePivot> result;

	result.reserve(sequence.numpivots);

	for (int i = 0; i < sequence.numpivots; ++i)
	{
		auto source = reinterpret_cast<const mstudiopivot_t*>(header->GetData() + sequence.pivotindex) + i;

		SequencePivot pivot
		{
			source->org,
			source->start,
			source->end
		};

		result.push_back(pivot);
	}

	return result;
}

std::vector<std::vector<Animation>> ConvertAnimationBlendsToEditable(const StudioModel& studioModel, const mstudioseqdesc_t& sequence)
{
	auto header = studioModel.GetStudioHeader();

	std::vector<std::vector<Animation>> result;

	result.reserve(sequence.numblends);

	auto source = studioModel.GetAnim(&sequence);

	for (int i = 0; i < sequence.numblends; ++i)
	{
		std::vector<Animation> animations;

		animations.reserve(header->numbones);

		for (int b = 0; b < header->numbones; ++b, ++source)
		{
			Animation animation;

			for (int j = 0; j < STUDIO_NUM_COORDINATE_AXES; ++j)
			{
				if (source->offset[j] != 0)
				{
					std::vector<mstudioanimvalue_t> values;

					auto valuesStart = reinterpret_cast<const mstudioanimvalue_t*>((reinterpret_cast<std::byte*>(source) + source->offset[j]));
					auto valuesEnd = valuesStart;

					//Determine number of values
					std::size_t valuesCount = 0;

					if (sequence.numframes > 0)
					{
						for (int f = 0; f < sequence.numframes;)
						{
							valuesCount += 1 + valuesEnd->num.valid;
							f += valuesEnd->num.total;

							valuesEnd += 1 + valuesEnd->num.valid;
						}
					}
					else
					{
						//Just the first count entry
						valuesCount = 1;
					}

					values.resize(valuesCount);

					std::memcpy(values.data(), valuesStart, valuesCount * sizeof(mstudioanimvalue_t));

					animation.Data[j] = std::move(values);
				}
			}

			animations.push_back(std::move(animation));
		}

		result.push_back(std::move(animations));
	}

	return result;
}

std::vector<std::unique_ptr<Sequence>> ConvertSequencesToEditable(const StudioModel& studioModel)
{
	auto header = studioModel.GetStudioHeader();

	std::vector<std::unique_ptr<Sequence>> result;

	result.reserve(header->numseq);

	for (int i = 0; i < header->numseq; ++i)
	{
		auto source = header->GetSequence(i);

		auto events = ConvertEventsToEditable(studioModel, *source);

		std::vector<SequenceEvent*> sortedEvents;

		sortedEvents.reserve(events.size());

		std::transform(events.begin(), events.end(), std::back_inserter(sortedEvents), [](const auto& event)
			{
				return event.get();
			}
		);

		SortEventsList(sortedEvents);

		Sequence sequence
		{
			source->label,
			source->fps,
			source->flags,
			source->activity,
			source->actweight,
			std::move(events),
			std::move(sortedEvents),
			source->numframes,
			ConvertPivotsToEditable(studioModel, *source),
			source->motiontype,
			source->motionbone,
			source->linearmovement,
			source->bbmin,
			source->bbmax,
			ConvertAnimationBlendsToEditable(studioModel, *source),
			{
				{
					{
						source->blendtype[0],
						source->blendstart[0],
						source->blendend[0]
					},
					{
						source->blendtype[1],
						source->blendstart[1],
						source->blendend[1]
					}
				}
			},
			source->entrynode,
			source->exitnode,
			source->nodeflags,
			source->nextseq
		};

		result.push_back(std::make_unique<Sequence>(std::move(sequence)));
	}

	return result;
}

std::vector<std::unique_ptr<Attachment>> ConvertAttachmentsToEditable(const StudioModel& studioModel, std::vector<std::unique_ptr<Bone>>& bones)
{
	auto header = studioModel.GetStudioHeader();

	std::vector<std::unique_ptr<Attachment>> result;

	result.reserve(header->numattachments);

	for (int i = 0; i < header->numattachments; ++i)
	{
		auto source = header->GetAttachment(i);

		Attachment attachment
		{
			source->name,
			source->type,
			bones[source->bone].get(),
			source->org,
			{
				source->vectors[0],
				source->vectors[1],
				source->vectors[2]
			}
		};

		result.push_back(std::make_unique<Attachment>(attachment));
	}

	return result;
}

std::vector<Mesh> ConvertMeshesToEditable(const StudioModel& studioModel, const mstudiomodel_t& model)
{
	auto header = studioModel.GetStudioHeader();

	std::vector<Mesh> result;

	result.reserve(model.nummesh);

	for (int i = 0; i < model.nummesh; ++i)
	{
		auto source = reinterpret_cast<const mstudiomesh_t*>(header->GetData() + model.meshindex) + i;

		auto cmdStart = reinterpret_cast<const short*>(header->GetData() + source->triindex);
		auto cmdEnd = cmdStart;

		for (int i = std::abs(*cmdEnd++); i > 0; i = std::abs(*cmdEnd++))
		{
			cmdEnd += i * 4;
		}

		Mesh mesh
		{
			{cmdStart, cmdEnd},
			source->numtris,
			source->numnorms,
			source->skinref,
		};

		result.push_back(std::move(mesh));
	}

	return result;
}

std::vector<ModelVertexInfo> ConvertModelVertexInfoToEditable(const StudioModel& studioModel, std::vector<std::unique_ptr<Bone>>& bones,
	int vertexIndex, int vertexInfoIndex, int count)
{
	auto header = studioModel.GetStudioHeader();

	std::vector<ModelVertexInfo> result;

	result.reserve(count);

	for (int i = 0; i < count; ++i)
	{
		ModelVertexInfo info
		{
			reinterpret_cast<const glm::vec3*>(header->GetData() + vertexIndex)[i],
			bones[reinterpret_cast<std::uint8_t*>(header->GetData() + vertexInfoIndex)[i]].get()
		};

		result.push_back(info);
	}

	return result;
}

std::vector<Model> ConvertModelsToEditable(const StudioModel& studioModel, std::vector<std::unique_ptr<Bone>>& bones, const mstudiobodyparts_t& bodypart)
{
	auto header = studioModel.GetStudioHeader();

	std::vector<Model> result;

	result.reserve(bodypart.nummodels);

	for (int i = 0; i < bodypart.nummodels; ++i)
	{
		auto source = reinterpret_cast<const mstudiomodel_t*>(header->GetData() + bodypart.modelindex) + i;

		Model model
		{
			source->name,
			source->type,
			source->boundingradius,
			ConvertMeshesToEditable(studioModel, *source),
			ConvertModelVertexInfoToEditable(studioModel, bones, source->vertindex, source->vertinfoindex, source->numverts),
			ConvertModelVertexInfoToEditable(studioModel, bones, source->normindex, source->norminfoindex, source->numnorms)
		};

		result.push_back(std::move(model));
	}

	return result;
}

std::vector<std::unique_ptr<Bodypart>> ConvertBodypartsToEditable(const StudioModel& studioModel, std::vector<std::unique_ptr<Bone>>& bones)
{
	auto header = studioModel.GetStudioHeader();

	std::vector<std::unique_ptr<Bodypart>> result;

	result.reserve(header->numbodyparts);

	for (int i = 0; i < header->numbodyparts; ++i)
	{
		auto source = header->GetBodypart(i);

		Bodypart bodypart
		{
			source->name,
			source->base,
			ConvertModelsToEditable(studioModel, bones, *source)
		};

		result.push_back(std::make_unique<Bodypart>(std::move(bodypart)));
	}

	return result;
}

//Dol differs only in texture storage
//Instead of pixels followed by RGB palette, it has a 32 byte texture name (name of file without extension), followed by an RGBA palette and pixels
std::vector<std::unique_ptr<Texture>> ConvertDolTexturesToEditable(const StudioModel& studioModel)
{
	auto header = studioModel.GetTextureHeader();

	std::vector<std::unique_ptr<Texture>> result;

	result.reserve(header->numtextures);

	for (int i = 0; i < header->numtextures; ++i)
	{
		auto source = header->GetTexture(i);

		//Starts off with 32 byte texture name
		const auto& sourcePalette = *reinterpret_cast<graphics::RGBAPalette*>(header->GetData() + source->index + 32);

		graphics::RGBPalette palette;

		//Discard alpha value
		for (int e = 0; e < palette.size(); ++e)
		{
			palette[e] = sourcePalette[e];
		}

		const auto size = source->width * source->height;

		std::vector<std::byte> pixels;

		pixels.resize(size);

		auto pSourcePixels = header->GetData() + source->index + 32 + sourcePalette.GetSizeInBytes();

		for (int i = 0; i < size; ++i)
		{
			auto pixel = std::to_integer<int>(pSourcePixels[i]);

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

			pixels[i] = std::byte(pixel);
		}

		Texture texture
		{
			source->name,
			source->flags,
			{
				source->width,
				source->height,
				std::move(pixels),
				palette
			},
			i
		};

		result.push_back(std::make_unique<Texture>(texture));
	}

	return result;
}

std::vector<std::unique_ptr<Texture>> ConvertMdlTexturesToEditable(const StudioModel& studioModel)
{
	auto header = studioModel.GetTextureHeader();

	std::vector<std::unique_ptr<Texture>> result;

	result.reserve(header->numtextures);

	for (int i = 0; i < header->numtextures; ++i)
	{
		auto source = header->GetTexture(i);

		graphics::RGBPalette palette;

		std::memcpy(palette.AsByteArray(), header->GetData() + source->index + (source->width * source->height), sizeof(palette));

		Texture texture
		{
			source->name,
			source->flags,
			{
				source->width,
				source->height,
				{header->GetData() + source->index, header->GetData() + source->index + (source->width * source->height)},
				palette
			},
			i
		};

		result.push_back(std::make_unique<Texture>(texture));
	}

	return result;
}

std::vector<std::unique_ptr<Texture>> ConvertTexturesToEditable(const StudioModel& studioModel)
{
	if (studioModel.IsDol())
	{
		return ConvertDolTexturesToEditable(studioModel);
	}
	else
	{
		return ConvertMdlTexturesToEditable(studioModel);
	}
}

std::vector<std::vector<Texture*>> ConvertSkinFamiliesToEditable(const StudioModel& studioModel, std::vector<std::unique_ptr<Texture>>& textures)
{
	auto header = studioModel.GetTextureHeader();

	std::vector<std::vector<Texture*>> result;

	result.reserve(header->numskinfamilies);

	auto source = header->GetSkins();

	for (int i = 0; i < header->numskinfamilies; ++i)
	{
		std::vector<Texture*> skinRef;

		skinRef.reserve(header->numskinref);

		for (int j = 0; j < header->numskinref; ++j, ++source)
		{
			skinRef.push_back(textures[*source].get());
		}

		result.push_back(std::move(skinRef));
	}

	return result;
}

std::vector<std::vector<std::uint8_t>> ConvertTransitionsToEditable(const StudioModel& studioModel)
{
	auto header = studioModel.GetStudioHeader();

	std::vector<std::vector<std::uint8_t>> result;

	result.reserve(header->numtransitions);

	for (int i = 0; i < header->numtransitions; ++i)
	{
		auto source = header->GetTransitions() + (header->numtransitions * i);

		std::vector<std::uint8_t> transitions;

		transitions.resize(header->numtransitions);

		std::memcpy(transitions.data(), source, header->numtransitions);

		result.push_back(std::move(transitions));
	}

	return result;
}
}

EditableStudioModel ConvertToEditable(const StudioModel& studioModel)
{
	auto header = studioModel.GetStudioHeader();

	EditableStudioModel result;

	result.EyePosition = header->eyeposition;
	result.BoundingMin = header->min;
	result.BoundingMax = header->max;
	result.ClippingMin = header->bbmin;
	result.ClippingMax = header->bbmax;
	result.Flags = header->flags;

	result.BoneControllers = ConvertBoneControllersToEditable(studioModel);
	result.Bones = ConvertBonesToEditable(studioModel, result.BoneControllers);
	result.Hitboxes = ConvertHitboxesToEditable(studioModel, result.Bones);
	result.SequenceGroups = ConvertSequenceGroupsToEditable(studioModel);
	result.Sequences = ConvertSequencesToEditable(studioModel);
	result.Attachments = ConvertAttachmentsToEditable(studioModel, result.Bones);
	result.Bodyparts = ConvertBodypartsToEditable(studioModel, result.Bones);

	result.Textures = ConvertTexturesToEditable(studioModel);
	result.SkinFamilies = ConvertSkinFamiliesToEditable(studioModel, result.Textures);

	result.Transitions = ConvertTransitionsToEditable(studioModel);

	return result;
}

namespace
{
template<typename T>
T* AllocateBufferArray(std::vector<std::byte>& buffer, std::size_t count)
{
	const auto position = buffer.size();
	buffer.resize(position + sizeof(T) * count);

	return reinterpret_cast<T*>(buffer.data() + position);
}

static void WriteRawBytes(std::vector<std::byte>& buffer, const std::byte* data, std::size_t sizeInBytes)
{
	const auto position = buffer.size();
	buffer.resize(position + sizeInBytes);
	std::memcpy(buffer.data() + position, data, sizeInBytes);
}

template<typename T>
static void WriteBytes(std::vector<std::byte>& buffer, const T& data)
{
	WriteRawBytes(buffer, reinterpret_cast<const std::byte*>(&data), sizeof(data));
}

static void AlignBuffer(std::vector<std::byte>& buffer)
{
	const std::size_t remainder = buffer.size() % 4;

	if (remainder != 0)
	{
		//Align start of next data to a 4 byte boundary
		const std::size_t bytesToAdd = 4 - remainder;

		const std::byte pad[3]{};

		WriteRawBytes(buffer, pad, bytesToAdd);
	}
}

void ConvertBonesFromEditable(const EditableStudioModel& studioModel, studiohdr_t& header, std::vector<std::byte>& buffer)
{
	assert(MAXSTUDIOCONTROLLERS >= studioModel.BoneControllers.size());

	std::array<int, MAXSTUDIOCONTROLLERS> boneControllerToBoneMap{};

	{
		header.numbones = studioModel.Bones.size();
		header.boneindex = buffer.size();

		auto bones = AllocateBufferArray<mstudiobone_t>(buffer, studioModel.Bones.size());

		for (int i = 0; i < header.numbones; ++i)
		{
			const auto& source = *studioModel.Bones[i];
			auto& dest = bones[i];

			UTIL_CopyString(dest.name, source.Name.c_str());
			dest.parent = source.Parent ? source.Parent->ArrayIndex : -1;
			dest.flags = source.Flags;

			for (int j = 0; j < STUDIO_NUM_COORDINATE_AXES; ++j)
			{
				const auto& axis = source.Axes[j];

				if (axis.Controller)
				{
					dest.bonecontroller[j] = axis.Controller->ArrayIndex;
					boneControllerToBoneMap[axis.Controller->ArrayIndex] = i;
				}
				else
				{
					dest.bonecontroller[j] = -1;
				}

				dest.value[j] = axis.Value;
				dest.scale[j] = axis.Scale;
			}
		}
	}

	{
		header.numbonecontrollers = studioModel.BoneControllers.size();
		header.bonecontrollerindex = buffer.size();

		auto boneControllers = AllocateBufferArray<mstudiobonecontroller_t>(buffer, studioModel.BoneControllers.size());

		for (int i = 0; i < header.numbonecontrollers; ++i)
		{
			const auto& source = *studioModel.BoneControllers[i];
			auto& dest = boneControllers[i];

			dest.bone = boneControllerToBoneMap[i];
			dest.index = source.Index;
			dest.type = source.Type;
			dest.start = source.Start;
			dest.end = source.End;
		}

		AlignBuffer(buffer);
	}
}

void ConvertAttachmentsFromEditable(const EditableStudioModel& studioModel, studiohdr_t& header, std::vector<std::byte>& buffer)
{
	header.numattachments = studioModel.Attachments.size();
	header.attachmentindex = buffer.size();

	auto attachments = AllocateBufferArray<mstudioattachment_t>(buffer, studioModel.Attachments.size());

	for (int i = 0; i < header.numattachments; ++i)
	{
		const auto& source = *studioModel.Attachments[i];
		auto& dest = attachments[i];

		UTIL_CopyString(dest.name, source.Name.c_str());
		dest.bone = source.Bone->ArrayIndex;
		dest.org = source.Origin;
		dest.type = source.Type;

		for (int j = 0; j < STUDIO_ATTACH_NUM_VECTORS; ++j)
		{
			dest.vectors[j] = source.Vectors[j];
		}
	}

	AlignBuffer(buffer);
}

void ConvertHitboxesFromEditable(const EditableStudioModel& studioModel, studiohdr_t& header, std::vector<std::byte>& buffer)
{
	header.numhitboxes = studioModel.Hitboxes.size();
	header.hitboxindex = buffer.size();

	auto hitboxes = AllocateBufferArray<mstudiobbox_t>(buffer, studioModel.Hitboxes.size());

	for (int i = 0; i < header.numhitboxes; ++i)
	{
		const auto& source = *studioModel.Hitboxes[i];
		auto& dest = hitboxes[i];

		dest.bone = source.Bone->ArrayIndex;
		dest.group = source.Group;
		dest.bbmin = source.Min;
		dest.bbmax = source.Max;
	}

	AlignBuffer(buffer);
}

std::vector<std::size_t> ConvertAnimationsFromEditable(const EditableStudioModel& studioModel, studiohdr_t& header, std::vector<std::byte>& buffer)
{
	std::vector<std::size_t> sequenceAnimationIndices;

	sequenceAnimationIndices.reserve(studioModel.Sequences.size());

	std::vector<mstudioanim_t> animations;

	for (std::size_t i = 0; i < studioModel.Sequences.size(); ++i)
	{
		const auto& source = *studioModel.Sequences[i];

		animations.resize(source.AnimationBlends.size() * studioModel.Bones.size());

		sequenceAnimationIndices.push_back(buffer.size());
		
		//Allocate the space for the offsets array, but don't write to it yet
		AllocateBufferArray<mstudioanim_t>(buffer, animations.size());

		AlignBuffer(buffer);

		for (std::size_t blend = 0; blend < source.AnimationBlends.size(); ++blend)
		{
			for (std::size_t bone = 0; bone < studioModel.Bones.size(); ++bone)
			{
				auto& destOffsets = animations[(blend * studioModel.Bones.size()) + bone];

				for (int axis = 0; axis < STUDIO_NUM_COORDINATE_AXES; ++axis)
				{
					const auto& sourceOffsets = source.AnimationBlends[blend][bone].Data[axis];

					if (sourceOffsets.size() == 0)
					{
						destOffsets.offset[axis] = 0;
					}
					else
					{
						//Offsets are relative to the current animation, not relative to start of the buffer
						destOffsets.offset[axis] = (buffer.size() - sequenceAnimationIndices.back())
							- (((blend * studioModel.Bones.size()) + bone) * sizeof(mstudioanim_t));

						WriteRawBytes(buffer, reinterpret_cast<const std::byte*>(sourceOffsets.data()), sourceOffsets.size() * sizeof(mstudioanimvalue_t));
					}
				}
			}
		}

		AlignBuffer(buffer);

		//Copy the finished animations array
		std::memcpy(buffer.data() + sequenceAnimationIndices.back(), animations.data(), animations.size() * sizeof(mstudioanim_t));
	}

	return sequenceAnimationIndices;
}

void ConvertSequencesFromEditable(const EditableStudioModel& studioModel, const std::vector<std::size_t>& sequenceAnimationIndices,
	studiohdr_t& header, std::vector<std::byte>& buffer)
{
	header.numseq = studioModel.Sequences.size();
	header.seqindex = buffer.size();

	std::vector<mstudioseqdesc_t> sequences;

	sequences.reserve(header.numseq);

	//Allocate the space for the sequences array, but don't write to it yet
	AllocateBufferArray<mstudioseqdesc_t>(buffer, header.numseq);

	for (int i = 0; i < header.numseq; ++i)
	{
		const auto& source = *studioModel.Sequences[i];

		mstudioseqdesc_t dest{};

		std::memset(&dest, 0, sizeof(dest));

		UTIL_CopyString(dest.label, source.Label.c_str());
		dest.numframes = source.NumFrames;
		dest.fps = source.FPS;
		dest.flags = source.Flags;

		dest.numblends = source.AnimationBlends.size();

		for (std::size_t b = 0; b < SequenceBlendCount; ++b)
		{
			const auto& sourceBlend = source.BlendData[b];

			dest.blendtype[b] = sourceBlend.Type;
			dest.blendstart[b] = sourceBlend.Start;
			dest.blendend[b] = sourceBlend.End;
		}

		dest.motiontype = source.MotionType;
		dest.motionbone = source.MotionBone;
		dest.linearmovement = source.LinearMovement;

		//Always in the main file
		dest.seqgroup = 0;

		dest.animindex = sequenceAnimationIndices[i];

		dest.activity = source.Activity;
		dest.actweight = source.ActivityWeight;

		dest.bbmin = source.BBMin;
		dest.bbmax = source.BBMax;

		dest.entrynode = source.EntryNode;
		dest.exitnode = source.ExitNode;
		dest.nodeflags = source.NodeFlags;

		dest.nextseq = source.NextSequence;

		{
			dest.eventindex = buffer.size();
			dest.numevents = source.SortedEvents.size();

			auto events = AllocateBufferArray<mstudioevent_t>(buffer, source.SortedEvents.size());

			for (std::size_t e = 0; e < source.SortedEvents.size(); ++e)
			{
				const auto& sourceEvent = *source.SortedEvents[e];
				auto& destEvent = events[e];

				destEvent.frame = sourceEvent.Frame;
				destEvent.event = sourceEvent.EventId;
				destEvent.type = sourceEvent.Type;
				UTIL_CopyString(destEvent.options, sourceEvent.Options.c_str());
			}

			AlignBuffer(buffer);
		}

		{
			dest.pivotindex = buffer.size();
			dest.numpivots = source.Pivots.size();

			auto pivots = AllocateBufferArray<mstudiopivot_t>(buffer, source.Pivots.size());

			for (std::size_t p = 0; p < source.Pivots.size(); ++p)
			{
				const auto& sourcePivot = source.Pivots[p];
				auto& destPivot = pivots[p];

				destPivot.org = sourcePivot.Origin;
				destPivot.start = sourcePivot.Start;
				destPivot.end = sourcePivot.End;
			}

			AlignBuffer(buffer);
		}

		sequences.push_back(dest);
	}

	//Copy the finished sequences array
	std::memcpy(buffer.data() + header.seqindex, sequences.data(), sequences.size() * sizeof(mstudioseqdesc_t));
}

void ConvertSequenceGroupsFromEditable(const EditableStudioModel& studioModel, studiohdr_t& header, std::vector<std::byte>& buffer)
{
	header.numseqgroups = studioModel.SequenceGroups.size();
	header.seqgroupindex = buffer.size();

	auto groups = AllocateBufferArray<mstudioseqgroup_t>(buffer, studioModel.SequenceGroups.size());

	//Only one group will ever exist at this point
	for (int i = 0; i < header.numseqgroups; ++i)
	{
		const auto& source = *studioModel.SequenceGroups[i];
		auto& dest = groups[i];

		UTIL_CopyString(dest.label, source.Label.c_str());
		dest.name[0] = '\0';
		dest.unused1 = 0;
		dest.unused2 = 0;
	}

	AlignBuffer(buffer);
}

void ConvertTransitionsFromEditable(const EditableStudioModel& studioModel, studiohdr_t& header, std::vector<std::byte>& buffer)
{
	header.numtransitions = studioModel.Transitions.size();
	header.transitionindex = buffer.size();

	auto transitions = AllocateBufferArray<std::byte>(buffer, studioModel.Transitions.size() * studioModel.Transitions.size());

	for (int i = 0; i < header.numtransitions; ++i)
	{
		const auto& source = studioModel.Transitions[i];
		
		std::memcpy(transitions + (i * header.numtransitions), source.data(), header.numtransitions);
	}

	AlignBuffer(buffer);
}

void ConvertBodypartsFromEditable(const EditableStudioModel& studioModel, studiohdr_t& header, std::vector<std::byte>& buffer)
{
	header.numbodyparts = studioModel.Bodyparts.size();
	header.bodypartindex = buffer.size();

	//Allocate the space for the sequences array, but don't write to it yet
	AllocateBufferArray<mstudiobodyparts_t>(buffer, studioModel.Bodyparts.size());

	std::vector<mstudiobodyparts_t> bodyparts;

	bodyparts.reserve(header.numbodyparts);

	std::size_t modelsOffset = buffer.size();

	//Allocate the entire models array upfront to match the compiler
	{
		std::size_t modelsCount = 0;

		for (int i = 0; i < header.numbodyparts; ++i)
		{
			const auto& source = *studioModel.Bodyparts[i];
			modelsCount += source.Models.size();
		}

		AllocateBufferArray<mstudiomodel_t>(buffer, modelsCount);
	}

	for (int i = 0; i < header.numbodyparts; ++i)
	{
		const auto& source = *studioModel.Bodyparts[i];

		mstudiobodyparts_t bodypart{};

		UTIL_CopyString(bodypart.name, source.Name.c_str());

		bodypart.nummodels = source.Models.size();
		bodypart.base = source.Base;
		bodypart.modelindex = modelsOffset;

		modelsOffset += bodypart.nummodels * sizeof(mstudiomodel_t);

		std::vector<mstudiomodel_t> models;

		models.reserve(source.Models.size());

		for (std::size_t m = 0; m < source.Models.size(); ++m)
		{
			const auto& sourceModel = source.Models[m];
			mstudiomodel_t destModel{};

			UTIL_CopyString(destModel.name, sourceModel.Name.c_str());

			{
				destModel.numverts = sourceModel.Vertices.size();
				destModel.vertinfoindex = buffer.size();

				auto vertexInfo = AllocateBufferArray<std::uint8_t>(buffer, destModel.numverts);

				for (std::size_t j = 0; j < sourceModel.Vertices.size(); ++j)
				{
					vertexInfo[j] = sourceModel.Vertices[j].Bone->ArrayIndex;
				}

				AlignBuffer(buffer);
			}

			{
				destModel.numnorms = sourceModel.Normals.size();
				destModel.norminfoindex = buffer.size();

				auto normalInfo = AllocateBufferArray<std::uint8_t>(buffer, destModel.numnorms);

				for (std::size_t j = 0; j < sourceModel.Normals.size(); ++j)
				{
					normalInfo[j] = sourceModel.Normals[j].Bone->ArrayIndex;
				}

				AlignBuffer(buffer);
			}

			{
				destModel.vertindex = buffer.size();

				auto vertices = AllocateBufferArray<glm::vec3>(buffer, destModel.numverts);

				for (std::size_t j = 0; j < sourceModel.Vertices.size(); ++j)
				{
					vertices[j] = sourceModel.Vertices[j].Vertex;
				}

				AlignBuffer(buffer);
			}

			{
				destModel.normindex = buffer.size();

				auto normals = AllocateBufferArray<glm::vec3>(buffer, destModel.numnorms);

				for (std::size_t j = 0; j < sourceModel.Normals.size(); ++j)
				{
					normals[j] = sourceModel.Normals[j].Vertex;
				}

				AlignBuffer(buffer);
			}

			{
				destModel.nummesh = sourceModel.Meshes.size();
				destModel.meshindex = buffer.size();

				AllocateBufferArray<mstudiomesh_t>(buffer, destModel.nummesh);

				std::vector<mstudiomesh_t> meshes;

				meshes.reserve(destModel.nummesh);

				for (int mesh = 0; mesh < sourceModel.Meshes.size(); ++mesh)
				{
					const auto& sourceMesh = sourceModel.Meshes[mesh];

					mstudiomesh_t destMesh{};

					destMesh.numtris = sourceMesh.NumTriangles;
					destMesh.skinref = sourceMesh.SkinRef;
					destMesh.numnorms = sourceMesh.NumNorms;
					destMesh.normindex = 0;

					destMesh.triindex = buffer.size();

					auto trianglesCmdBuffer = AllocateBufferArray<short>(buffer, sourceMesh.Triangles.size());

					std::memcpy(trianglesCmdBuffer, sourceMesh.Triangles.data(), sourceMesh.Triangles.size() * sizeof(short));

					AlignBuffer(buffer);

					meshes.push_back(destMesh);
				}

				std::memcpy(buffer.data() + destModel.meshindex, meshes.data(), meshes.size() * sizeof(mstudiomesh_t));
			}

			models.push_back(destModel);
		}

		std::memcpy(buffer.data() + bodypart.modelindex, models.data(), models.size() * sizeof(mstudiomodel_t));

		bodyparts.push_back(bodypart);
	}

	AlignBuffer(buffer);

	//Copy the finished bodyparts array
	std::memcpy(buffer.data() + header.bodypartindex, bodyparts.data(), bodyparts.size() * sizeof(mstudiobodyparts_t));
}

void ConvertTexturesFromEditable(const EditableStudioModel& studioModel, studiohdr_t& header, std::vector<std::byte>& buffer)
{
	header.numtextures = studioModel.Textures.size();
	header.textureindex = buffer.size();

	//Initialized below
	AllocateBufferArray<mstudiotexture_t>(buffer, studioModel.Textures.size());
	AlignBuffer(buffer);

	header.numskinfamilies = studioModel.SkinFamilies.size();
	header.numskinref = !studioModel.SkinFamilies.empty() ? studioModel.SkinFamilies[0].size() : 0;
	header.skinindex = buffer.size();

	for (std::size_t i = 0; i < studioModel.SkinFamilies.size(); ++i)
	{
		const auto& source = studioModel.SkinFamilies[i];

		auto skinref = AllocateBufferArray<short>(buffer, source.size());

		for (std::size_t j = 0; j < source.size(); ++j)
		{
			skinref[j] = source[j]->ArrayIndex;
		}
	}

	AlignBuffer(buffer);

	header.texturedataindex = buffer.size();

	for (int i = 0; i < header.numtextures; ++i)
	{
		const auto& source = *studioModel.Textures[i];

		{
			auto& dest = *(reinterpret_cast<mstudiotexture_t*>(buffer.data() + header.textureindex) + i);

			UTIL_CopyString(dest.name, source.Name.c_str());
			dest.flags = source.Flags;
			dest.width = source.Data.Width;
			dest.height = source.Data.Height;
			dest.index = buffer.size();
		}

		auto textureData = AllocateBufferArray<std::byte>(buffer, source.Data.Pixels.size() + sizeof(source.Data.Palette));

		std::memcpy(textureData, source.Data.Pixels.data(), source.Data.Pixels.size());
		std::memcpy(textureData + source.Data.Pixels.size(), source.Data.Palette.AsByteArray(), sizeof(source.Data.Palette));
	}

	AlignBuffer(buffer);
}
}

/**
*	@brief The initial size of the buffer used for conversion. Matches the compiler, helps to avoid constant reallocations.
*/
constexpr std::size_t StudioModelBufferSize = 16 * 1024 * 1024;

StudioModel ConvertFromEditable(const std::filesystem::path& fileName, const EditableStudioModel& studioModel)
{
	//Use a local header until all data is written, then write the header to the start of the buffer
	//This avoids having to reacquire the header pointer every time the buffer is reallocated
	studiohdr_t header{};

	std::memset(&header, 0, sizeof(header));

	std::vector<std::byte> buffer;

	buffer.reserve(StudioModelBufferSize);

	//Write dummy header
	WriteBytes(buffer, header);

	std::memcpy(&header.id, STUDIOMDL_HDR_ID, sizeof(header.id));
	header.version = STUDIO_VERSION;


	//Store only the filename itself. It's never used for file loading so it's not terribly important
	UTIL_CopyString(header.name, reinterpret_cast<const char*>(fileName.filename().u8string().c_str()));

	header.eyeposition = studioModel.EyePosition;
	header.min = studioModel.BoundingMin;
	header.max = studioModel.BoundingMax;
	header.bbmin = studioModel.ClippingMin;
	header.bbmax = studioModel.ClippingMax;
	header.flags = studioModel.Flags;

	ConvertBonesFromEditable(studioModel, header, buffer);
	ConvertAttachmentsFromEditable(studioModel, header, buffer);
	ConvertHitboxesFromEditable(studioModel, header, buffer);

	{
		const auto animationIndices = ConvertAnimationsFromEditable(studioModel, header, buffer);
		ConvertSequencesFromEditable(studioModel, animationIndices, header, buffer);
	}

	ConvertSequenceGroupsFromEditable(studioModel, header, buffer);
	ConvertTransitionsFromEditable(studioModel, header, buffer);
	ConvertBodypartsFromEditable(studioModel, header, buffer);
	ConvertTexturesFromEditable(studioModel, header, buffer);

	header.length = buffer.size();

	//Copy completed header into buffer
	std::memcpy(buffer.data(), &header, sizeof(header));

	//Create standalone copy for transfer
	auto studioHeader = std::make_unique<std::byte[]>(buffer.size());

	std::memcpy(studioHeader.get(), buffer.data(), buffer.size());

	return StudioModel{studio_ptr<studiohdr_t>{reinterpret_cast<studiohdr_t*>(studioHeader.release())}, {}, {}, false};
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
