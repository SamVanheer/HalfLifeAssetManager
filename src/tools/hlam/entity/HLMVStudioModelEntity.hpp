#pragma once

#include "game/entity/StudioModelEntity.hpp"

class HLMVStudioModelEntity : public StudioModelEntity
{
public:
	DECLARE_CLASS( HLMVStudioModelEntity, StudioModelEntity);

public:
	virtual void OnCreate() override;

	virtual bool Spawn() override;

	virtual void HandleAnimEvent( const AnimEvent& event ) override;

	void AnimThink();

	//TODO: these should be moved out of the class to eliminate overhead when multiple entities are in a scene
	bool PlaySequence = true;
	bool PlaySound = false;
	bool PitchFramerateAmplitude = false;
};
