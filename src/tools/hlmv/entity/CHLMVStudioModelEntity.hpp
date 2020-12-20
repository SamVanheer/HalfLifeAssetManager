#pragma once

#include "game/entity/CStudioModelEntity.hpp"

class CHLMVStudioModelEntity : public CStudioModelEntity
{
public:
	DECLARE_CLASS( CHLMVStudioModelEntity, CStudioModelEntity );

public:
	virtual void OnCreate() override;

	virtual bool Spawn() override;

	virtual void HandleAnimEvent( const CAnimEvent& event ) override;

	void AnimThink();

	//TODO: these should be moved out of the class to eliminate overhead when multiple entities are in a scene
	bool PlaySequence = true;
	bool PlaySound = false;
	bool PitchFramerateAmplitude = false;
};
