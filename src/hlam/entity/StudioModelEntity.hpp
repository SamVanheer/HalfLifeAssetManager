#pragma once

#include <cassert>
#include <cstdint>
#include <optional>
#include <vector>

#include "entity/AnimEvent.hpp"
#include "entity/BaseAnimating.hpp"
#include "entity/Events.hpp"

#include "formats/studiomodel/EditableStudioModel.hpp"
#include "formats/studiomodel/IStudioModelRenderer.hpp"

enum class StudioLoopingMode
{
	AlwaysLoop = 0,
	NeverLoop,
	UseSequenceSetting
};

enum class StudioBlendMode
{
	Standard,
	CounterStrike
};

/**
*	Studio model entity.
*/
class StudioModelEntity : public BaseAnimating
{
public:
	DECLARE_CLASS(StudioModelEntity, BaseAnimating);

public:
	explicit StudioModelEntity(studiomdl::EditableStudioModel* model);

	void Spawn() override;

	RenderPasses GetRenderPasses() const override { return RenderPass::Standard; }

	void Draw(graphics::SceneContext& sc, RenderPasses renderPass) override;

	void CreateDeviceObjects(graphics::SceneContext& sc) override;

	void DestroyDeviceObjects(graphics::SceneContext& sc) override;

	studiomdl::ModelRenderInfo GetRenderInfo() const;

	/**
	*	Advances the frame. If dt is 0, advances to current time, otherwise, advances by the given amount of time.
	*	@param deltaTime Delta time.
	*	@param maximum Maximum amount of time to advance by. If -1, no limit.
	*	@return Delta time that was used to advance the frame. Can be 0.
	*/
	float AdvanceFrame(float deltaTime = 0.0f, const float maximum = -1.f);

	/**
	*	Gets an animation event for the current sequence for the given time range.
	*	@param event Output. Event data.
	*	@param start Start of the range of frames to check.
	*	@param end End of the range of frames to check.
	*	@param index Event index to start checking at.
	*	@return Next event index to use as the index parameter. If 0, no more events are left.
	*/
	int GetAnimationEvent(AnimEvent& event, float start, float end, int index);

	/**
	*	Dispatches events for the current sequence and frame.
	*	This will dispatch events between the frame number during last call to DispatchAnimEvents and the current frame.
	*/
	void DispatchAnimEvents();

	/**
	*	Method to handle animation events. Override to handle events.
	*/
	virtual void HandleAnimEvent(const AnimEvent& event);

public:
	void SetFrame(float frame);

private:
	studiomdl::EditableStudioModel* _editableModel = nullptr;

	int _sequence = 0;				// sequence index
	int _bodygroup = 0;				// bodypart selection	
	int _skin = 0;				// skin group selection
	std::uint8_t _controller[STUDIO_MAX_CONTROLLERS] = {0, 0, 0, 0};	// bone controllers
	float _controllerValues[STUDIO_MAX_CONTROLLERS] = {};
	std::uint8_t _mouth = 0;				// mouth position
	float _mouthValue = 0;
	std::uint8_t _blending[STUDIO_MAX_BLENDERS] = {0, 0};			// animation blending
	float _blendingValues[STUDIO_MAX_BLENDERS] = {};

	float _lastEventCheck = 0;				//Last time we checked for animation events.
	float _animTime = 0;				//Time when the frame was set.

	StudioLoopingMode _loopingMode = StudioLoopingMode::AlwaysLoop;

	struct IBlender
	{
		virtual StudioBlendMode GetBlendMode() const = 0;

		virtual bool AlwaysHasBlender() const = 0;

		virtual std::optional<std::uint8_t> CalculateBlend(const studiomdl::Sequence& sequenceDescriptor, int blender, float value) const = 0;
	};

	struct StandardBlender final : public IBlender
	{
		StudioBlendMode GetBlendMode() const override final { return StudioBlendMode::Standard; }

		bool AlwaysHasBlender() const override final { return false; }

		std::optional<std::uint8_t> CalculateBlend(const studiomdl::Sequence& sequenceDescriptor, int blender, float value) const override final;
	};

	struct CounterStrikeBlender final : public IBlender
	{
		StudioBlendMode GetBlendMode() const override final { return StudioBlendMode::CounterStrike; }

		bool AlwaysHasBlender() const override final { return true; }

		std::optional<std::uint8_t> CalculateBlend(const studiomdl::Sequence& sequenceDescriptor, int blender, float value) const override final;
	};

	static constexpr StandardBlender StandardBlender{};
	static constexpr CounterStrikeBlender CounterStrikeBlender{};

	const IBlender* _blender = &StandardBlender;

public:
	studiomdl::EditableStudioModel* GetEditableModel() const { return _editableModel; }

	void SetEditableModel(studiomdl::EditableStudioModel* model);

	int GetNumFrames() const;

	int GetSequence() const { return _sequence; }

	void SetSequence(int sequence);

	/**
	*	Gets info from the current sequence.
	*	@param frameRate The sequence's frame rate.
	*	@param groundSpeed How fast the entity should move on the ground during this sequence.
	*/
	void GetSequenceInfo(float& frameRate, float& groundSpeed) const;

	int GetBodygroup() const { return _bodygroup; }

	int GetBodyValueForGroup(int group) const;

	/**
	*	Sets the value for the given bodygroup.
	*	@param bodygroup Bodygroup to set.
	*	@param value Value to set.
	*/
	void SetBodygroup(const int bodygroup, const int value);

	void SetCompoundBodyValue(int value)
	{
		//TODO: verify that this is a correct value
		_bodygroup = value;
	}

	int GetSkin() const { return _skin; }

	void SetSkin(const int skin);

	/**
	*	Gets the given controller by index. This is the stored value, not the computed value.
	*	@param controller Controller to get.
	*/
	std::uint8_t GetControllerByIndex(const int controller) const;

	/**
	*	Gets the given controller value. The value is computed for the associated bone controller.
	*	@param controller Controller value to get.
	*/
	float GetControllerValue(const int controller) const;

	/**
	*	Sets the controller value. The value is processed into a value that is in the range [0, 255].
	*	@param controller Controller to set.
	*	@param value Value to set.
	*/
	void SetController(const int controller, float value);

	/**
	*	Gets the mouth controller. This is the stored value, not the computed value.
	*/
	std::uint8_t GetMouth() const { return _mouth; }

	float GetMouthValue() const { return _mouthValue; }

	/**
	*	Sets the mouth controller value. The value is processed into a value that is in the range [0, 255]
	*	@param value Value to set.
	*/
	void SetMouth(float value);

	void SetBlendMode(StudioBlendMode blendMode)
	{
		switch (blendMode)
		{
		case StudioBlendMode::Standard:
		{
			_blender = &StandardBlender;
			break;
		}

		case StudioBlendMode::CounterStrike:
		{
			_blender = &CounterStrikeBlender;
			break;
		}

		default:
			assert(!"Invalid blend mode");
			break;
		}
	}

	/**
	*	Gets the given blender by index. This is the stored value, not the computed value.
	*	@param blender Blender to get.
	*/
	std::uint8_t GetBlendingByIndex(const int blender) const;

	/**
	*	Gets the given blender value. The value is computed for the associated blender.
	*	@param blender Blender value to get.
	*/
	float GetBlendingValue(const int blender) const;

	/**
	*	Sets the given blender's value.
	*	@param blender Blender to set.
	*	@param value Value to set.
	*/
	void SetBlending(const int blender, float value);

	/**
	*	Gets the last event check.
	*	This is the end of the range used to check for animation events the last time DispatchAnimEvents was called.
	*/
	float GetLastEventCheck() const { return _lastEventCheck; }

	/**
	*	Gets the last time this entity advanced its frame.
	*/
	float GetAnimTime() const { return _animTime; }

	StudioLoopingMode GetLoopingMode() const { return _loopingMode; }

	void SetLoopingMode(StudioLoopingMode value)
	{
		_loopingMode = value;
	}

	/**
	*	Extracts the bounding box from the current sequence.
	*/
	void ExtractBbox(glm::vec3& mins, glm::vec3& maxs) const;

	void AlignOnGround();
};
