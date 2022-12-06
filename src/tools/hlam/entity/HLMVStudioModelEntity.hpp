#pragma once

#include "entity/StudioModelEntity.hpp"

// TODO: Qt's OpenGL headers are apparently pulling in Windows.h from somewhere. Need to get rid of that.
#undef PlaySound

class HLMVStudioModelEntity : public StudioModelEntity
{
public:
	DECLARE_CLASS(HLMVStudioModelEntity, StudioModelEntity);

public:
	virtual void Spawn() override;

	virtual void HandleAnimEvent(const AnimEvent& event) override;

	void AnimThink();

	//TODO: these should be moved out of the class to eliminate overhead when multiple entities are in a scene
	bool PlaySequence = true;
	bool PlaySound = false;
	bool PitchFramerateAmplitude = false;
};
