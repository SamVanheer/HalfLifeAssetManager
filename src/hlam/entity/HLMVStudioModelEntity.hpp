#pragma once

#include "entity/StudioModelEntity.hpp"

class HLMVStudioModelEntity : public StudioModelEntity
{
public:
	DECLARE_CLASS(HLMVStudioModelEntity, StudioModelEntity);

public:
	using StudioModelEntity::StudioModelEntity;

	virtual void Spawn() override;

	virtual void HandleAnimEvent(const AnimEvent& event) override;

	void AnimThink();
};
