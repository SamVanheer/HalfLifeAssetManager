#pragma once

#include <array>
#include <cstddef>
#include <vector>

#include <glm/mat3x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "core/shared/Const.hpp"

#include "engine/shared/studiomodel/StudioModelFileFormat.hpp"

namespace studiomdl
{
struct Animation;
struct Bone;
struct Sequence;
class EditableStudioModel;

struct BoneTransformInfo
{
	const int SequenceIndex;
	const float Frame;
	glm::vec3 Scale;
	std::array<byte, SequenceBlendCount> Blenders;
	std::array<byte, ControllerCount> Controllers;
	byte Mouth;
};

/**
*	@brief Transforms bones based on input data
*/
class BoneTransformer final
{
private:
	static constexpr std::size_t TransformStatesCount = 4;

	struct TransformState
	{
		std::array<glm::vec3, MAXSTUDIOBONES> Positions;
		std::array<glm::vec4, MAXSTUDIOBONES> Quaternions;
	};

public:
	BoneTransformer() = default;
	~BoneTransformer() = default;

	/**
	*	@brief Sets up a bone array based on the given model and transform information
	*	@return Reference to the bone array, valid only when used immediately after this call
	*/
	const std::array<glm::mat3x4, MAXSTUDIOBONES>& SetUpBones(const EditableStudioModel& studioModel, const BoneTransformInfo& transformInfo);

private:
	void CalculateRotations(const EditableStudioModel& studioModel, const BoneTransformInfo& transformInfo,
		const Sequence& sequence, const std::vector<Animation>& anims, TransformState& transformState);

	void CalculateBoneAdjust(const EditableStudioModel& studioModel, const BoneTransformInfo& transformInfo,
		std::array<float, MAXSTUDIOCONTROLLERS>& boneAdjust);
	void CalculateBoneQuaternion(const int frame, const float s, const Bone& bone, const Animation& anim,
		const std::array<float, MAXSTUDIOCONTROLLERS>& boneAdjust, glm::vec4& q);
	void CalculateBonePosition(const int frame, const float s, const Bone&, const Animation& anim,
		const std::array<float, MAXSTUDIOCONTROLLERS>& boneAdjust, glm::vec3& pos);
	void SlerpBones(const EditableStudioModel& studioModel, float s, const TransformState& fromState, TransformState& toState);

private:
	//Used to store temporary calculations before calculating final values stored in _boneTransform
	std::array<TransformState, TransformStatesCount> _transformStates;

	std::array<glm::mat3x4, MAXSTUDIOBONES> _boneTransform{};
};
}
