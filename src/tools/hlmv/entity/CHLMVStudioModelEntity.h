#ifndef HLMV_ENTITY_CHLMVSTUDIOMODELENTITY_H
#define HLMV_ENTITY_CHLMVSTUDIOMODELENTITY_H

#include "game/entity/CStudioModelEntity.h"

namespace hlmv
{
class CHLMVState;
}

class CHLMVStudioModelEntity : public CStudioModelEntity
{
public:
	DECLARE_CLASS( CHLMVStudioModelEntity, CStudioModelEntity );

public:
	virtual void OnCreate() override;

	virtual bool Spawn() override;

	virtual void HandleAnimEvent( const CAnimEvent& event ) override;

	void AnimThink();

	hlmv::CHLMVState* m_pState = nullptr;
};

#endif //HLMV_ENTITY_CHLMVSTUDIOMODELENTITY_H