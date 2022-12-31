#include <algorithm>
#include <array>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

#include "formats/studiomodel/BoneTransformer.hpp"
#include "formats/studiomodel/EditableStudioModel.hpp"

#include "utility/mathlib.hpp"

namespace studiomdl
{
const std::array<glm::mat4x4, MAXSTUDIOBONES>& BoneTransformer::SetUpBones(
	const EditableStudioModel& studioModel, const BoneTransformInfo& transformInfo)
{
	int sequenceIndex = transformInfo.SequenceIndex;

	if (sequenceIndex != -1 && (sequenceIndex < -1 || sequenceIndex >= studioModel.Sequences.size()))
	{
		sequenceIndex = 0;
	}

	const StudioSequence emptySequence;

	const auto& sequence = sequenceIndex  != -1 ? *studioModel.Sequences[sequenceIndex] : emptySequence;

	if (sequence.AnimationBlends.size() == 9)
	{
		const auto blendX = static_cast<double>(transformInfo.Blenders[0]);
		const auto blendY = static_cast<double>(transformInfo.Blenders[1]);

		double interpolantX;
		double interpolantY;

		if (blendX > 127.0)
		{
			interpolantX = (blendX - 127.0) * 2;

			if (blendY > 127.0)
			{
				interpolantY = (blendY - 127.0) * 2;

				CalculateRotations(studioModel, transformInfo, sequence, sequence.AnimationBlends[4].data(), _transformStates[0]);
				CalculateRotations(studioModel, transformInfo, sequence, sequence.AnimationBlends[5].data(), _transformStates[1]);
				CalculateRotations(studioModel, transformInfo, sequence, sequence.AnimationBlends[7].data(), _transformStates[2]);
				CalculateRotations(studioModel, transformInfo, sequence, sequence.AnimationBlends[8].data(), _transformStates[3]);
			}
			else
			{
				interpolantY = blendY * 2;

				CalculateRotations(studioModel, transformInfo, sequence, sequence.AnimationBlends[1].data(), _transformStates[0]);
				CalculateRotations(studioModel, transformInfo, sequence, sequence.AnimationBlends[2].data(), _transformStates[1]);
				CalculateRotations(studioModel, transformInfo, sequence, sequence.AnimationBlends[4].data(), _transformStates[2]);
				CalculateRotations(studioModel, transformInfo, sequence, sequence.AnimationBlends[5].data(), _transformStates[3]);
			}
		}
		else
		{
			interpolantX = blendX * 2;

			if (blendY <= 127.0)
			{
				interpolantY = blendY * 2;

				CalculateRotations(studioModel, transformInfo, sequence, sequence.AnimationBlends[0].data(), _transformStates[0]);
				CalculateRotations(studioModel, transformInfo, sequence, sequence.AnimationBlends[1].data(), _transformStates[1]);
				CalculateRotations(studioModel, transformInfo, sequence, sequence.AnimationBlends[3].data(), _transformStates[2]);
				CalculateRotations(studioModel, transformInfo, sequence, sequence.AnimationBlends[4].data(), _transformStates[3]);
			}
			else
			{
				interpolantY = (blendY - 127.0) * 2;

				CalculateRotations(studioModel, transformInfo, sequence, sequence.AnimationBlends[3].data(), _transformStates[0]);
				CalculateRotations(studioModel, transformInfo, sequence, sequence.AnimationBlends[4].data(), _transformStates[1]);
				CalculateRotations(studioModel, transformInfo, sequence, sequence.AnimationBlends[6].data(), _transformStates[2]);
				CalculateRotations(studioModel, transformInfo, sequence, sequence.AnimationBlends[7].data(), _transformStates[3]);
			}
		}

		const auto normalizedInterpolantX = interpolantX / 255.0;
		SlerpBones(studioModel, normalizedInterpolantX, _transformStates[1], _transformStates[0]);
		SlerpBones(studioModel, normalizedInterpolantX, _transformStates[3], _transformStates[2]);

		const auto normalizedInterpolantY = interpolantY / 255.0;
		SlerpBones(studioModel, normalizedInterpolantY, _transformStates[2], _transformStates[0]);
	}
	else if (sequence.AnimationBlends.size() > 0)
	{
		CalculateRotations(studioModel, transformInfo, sequence, sequence.AnimationBlends[0].data(), _transformStates[0]);

		if (sequence.AnimationBlends.size() > 1)
		{
			CalculateRotations(studioModel, transformInfo, sequence, sequence.AnimationBlends[1].data(), _transformStates[1]);
			float s = transformInfo.Blenders[0] / 255.0;

			SlerpBones(studioModel, s, _transformStates[1], _transformStates[0]);

			if (sequence.AnimationBlends[0].size() == 4)
			{
				CalculateRotations(studioModel, transformInfo, sequence, sequence.AnimationBlends[2].data(), _transformStates[2]);
				CalculateRotations(studioModel, transformInfo, sequence, sequence.AnimationBlends[3].data(), _transformStates[3]);

				s = transformInfo.Blenders[0] / 255.0;
				SlerpBones(studioModel, s, _transformStates[3], _transformStates[2]);

				s = transformInfo.Blenders[1] / 255.0;
				SlerpBones(studioModel, s, _transformStates[2], _transformStates[0]);
			}
		}
	}
	else
	{
		std::array<StudioAnimation, MAXSTUDIOBONES> dummyAnims;

		CalculateRotations(studioModel, transformInfo, sequence, dummyAnims.data(), _transformStates[0]);
	}

	for (std::size_t i = 0; i < studioModel.Bones.size(); ++i)
	{
		const auto& bone = *studioModel.Bones[i];

		const auto bonematrix = glm::translate(_transformStates[0].Positions[i]) * glm::toMat4(_transformStates[0].Quaternions[i]);

		if (!bone.Parent)
		{
			//Apply scale to each root bone so only the model is scaled and mirrored, and not anything else in the scene
			//Apply the scale *after* the root bone's position and rotation is set so the mirror effect occurs in the correct coordinate space
			_boneTransform[i] = glm::scale(transformInfo.Scale) * bonematrix;
		}
		else
		{
			_boneTransform[i] = _boneTransform[bone.Parent->ArrayIndex] * bonematrix;
		}
	}

	return _boneTransform;
}

void BoneTransformer::CalculateRotations(
	const EditableStudioModel& studioModel, const BoneTransformInfo& transformInfo,
	const StudioSequence& sequence, const StudioAnimation* anims, TransformState& transformState)
{
	const int frame = (int)transformInfo.Frame;
	const float s = (transformInfo.Frame - frame);

	// add in programatic controllers
	std::array<float, MAXSTUDIOCONTROLLERS> boneAdjust;
	CalculateBoneAdjust(studioModel, transformInfo, boneAdjust);

	for (std::size_t i = 0; i < studioModel.Bones.size(); ++i)
	{
		const auto& bone = *studioModel.Bones[i];
		const auto& anim = anims[i];

		CalculateBoneQuaternion(frame, s, bone, anim, boneAdjust, transformState.Quaternions[i]);
		CalculateBonePosition(frame, s, bone, anim, boneAdjust, transformState.Positions[i]);
	}

	if (sequence.MotionType & STUDIO_X)
	{
		transformState.Positions[sequence.MotionBone][0] = 0.0;
	}

	if (sequence.MotionType & STUDIO_Y)
	{
		transformState.Positions[sequence.MotionBone][1] = 0.0;
	}

	if (sequence.MotionType & STUDIO_Z)
	{
		transformState.Positions[sequence.MotionBone][2] = 0.0;
	}
}

void BoneTransformer::CalculateBoneAdjust(
	const EditableStudioModel& studioModel, const BoneTransformInfo& transformInfo,
	std::array<float, MAXSTUDIOCONTROLLERS>& boneAdjust)
{
	for (std::size_t j = 0; j < studioModel.BoneControllers.size(); ++j)
	{
		const auto& boneController = *studioModel.BoneControllers[j];

		const auto i = boneController.Index;

		float value;

		if (i <= 3)
		{
			// check for 360% wrapping
			if (boneController.Type & STUDIO_RLOOP)
			{
				value = transformInfo.Controllers[i] * (360.0 / 256.0) + boneController.Start;
			}
			else
			{
				value = std::clamp(transformInfo.Controllers[i] / 255.0, 0.0, 1.0);
				value = (1.0 - value) * boneController.Start + value * boneController.End;
			}
			// Con_DPrintf( "%d %d %f : %f\n", m_controller[j], m_prevcontroller[j], value, dadt );
		}
		else
		{
			value = std::min(1.0, transformInfo.Mouth / 64.0);
			value = (1.0 - value) * boneController.Start + value * boneController.End;
			// Con_DPrintf("%d %f\n", mouthopen, value );
		}
		switch (boneController.Type & STUDIO_TYPES)
		{
		case STUDIO_XR:
		case STUDIO_YR:
		case STUDIO_ZR:
			boneAdjust[j] = value * (PI<double> / 180.0);
			break;
		case STUDIO_X:
		case STUDIO_Y:
		case STUDIO_Z:
			boneAdjust[j] = value;
			break;
		}
	}
}

void BoneTransformer::CalculateBoneQuaternion(
	const int frame, const float s, const StudioBone& bone, const StudioAnimation& anim,
	const std::array<float, MAXSTUDIOCONTROLLERS>& boneAdjust, glm::quat& q)
{
	glm::vec3 angle1{}, angle2{};

	for (std::size_t j = 0; j < 3; ++j)
	{
		const auto& axis = bone.Axes[j + 3];

		if (anim.Data[j + 3].empty())
		{
			angle2[j] = angle1[j] = axis.Value; // default;
		}
		else
		{
			auto panimvalue = anim.Data[j + 3].data();
			auto k = frame;

			while (panimvalue->num.total <= k)
			{
				k -= panimvalue->num.total;
				panimvalue += panimvalue->num.valid + 1;
			}

			// Bah, missing blend!
			if (panimvalue->num.valid > k)
			{
				angle1[j] = panimvalue[k + 1].value;

				if (panimvalue->num.valid > k + 1)
				{
					angle2[j] = panimvalue[k + 2].value;
				}
				else
				{
					if (panimvalue->num.total > k + 1)
					{
						angle2[j] = angle1[j];
					}
					else
					{
						angle2[j] = panimvalue[panimvalue->num.valid + 2].value;
					}
				}
			}
			else
			{
				angle1[j] = panimvalue[panimvalue->num.valid].value;

				if (panimvalue->num.total > k + 1)
				{
					angle2[j] = angle1[j];
				}
				else
				{
					angle2[j] = panimvalue[panimvalue->num.valid + 2].value;
				}
			}

			angle1[j] = axis.Value + angle1[j] * axis.Scale;
			angle2[j] = axis.Value + angle2[j] * axis.Scale;
		}

		if (axis.Controller)
		{
			angle1[j] += boneAdjust[axis.Controller->ArrayIndex];
			angle2[j] += boneAdjust[axis.Controller->ArrayIndex];
		}
	}

	if (!VectorCompare(angle1, angle2))
	{
		const glm::quat q1{angle1};
		const glm::quat q2{angle2};

		q = glm::slerp(q1, q2, s);
	}
	else
	{
		q = glm::quat{angle1};
	}
}

void BoneTransformer::CalculateBonePosition(
	const int frame, const float s, const StudioBone& bone, const StudioAnimation& anim,
	const std::array<float, MAXSTUDIOCONTROLLERS>& boneAdjust, glm::vec3& pos)
{
	for (std::size_t j = 0; j < 3; ++j)
	{
		const auto& axis = bone.Axes[j];

		pos[j] = axis.Value; // default;

		if (!anim.Data[j].empty())
		{
			auto panimvalue = anim.Data[j].data();

			auto k = frame;

			// find span of values that includes the frame we want
			while (panimvalue->num.total <= k)
			{
				k -= panimvalue->num.total;
				panimvalue += panimvalue->num.valid + 1;
			}

			// if we're inside the span
			if (panimvalue->num.valid > k)
			{
				// and there's more data in the span
				if (panimvalue->num.valid > k + 1)
				{
					pos[j] += (panimvalue[k + 1].value * (1.0 - s) + s * panimvalue[k + 2].value) * axis.Scale;
				}
				else
				{
					pos[j] += panimvalue[k + 1].value * axis.Scale;
				}
			}
			else
			{
				// are we at the end of the repeating values section and there's another section with data?
				if (panimvalue->num.total <= k + 1)
				{
					pos[j] += (panimvalue[panimvalue->num.valid].value * (1.0 - s) + s * panimvalue[panimvalue->num.valid + 2].value) * axis.Scale;
				}
				else
				{
					pos[j] += panimvalue[panimvalue->num.valid].value * axis.Scale;
				}
			}
		}

		if (axis.Controller)
		{
			pos[j] += boneAdjust[axis.Controller->ArrayIndex];
		}
	}
}

void BoneTransformer::SlerpBones(
	const EditableStudioModel& studioModel, float s, const TransformState& fromState, TransformState& toState)
{
	s = std::clamp(s, 0.0f, 1.0f);

	const float s1 = 1.0 - s;

	for (std::size_t i = 0; i < studioModel.Bones.size(); ++i)
	{
		toState.Quaternions[i] = glm::slerp(toState.Quaternions[i], fromState.Quaternions[i], s);

		toState.Positions[i] = toState.Positions[i] * s1 + fromState.Positions[i] * s;
	}
}
}
