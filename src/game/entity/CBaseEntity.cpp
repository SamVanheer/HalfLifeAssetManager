#include <cassert>

#include "CBaseEntityList.h"

#include "CBaseEntity.h"

CBaseEntity::CBaseEntity()
{
}

CBaseEntity::~CBaseEntity()
{
}

void CBaseEntity::Construct( const char* const pszClassName )
{
	assert( pszClassName && *pszClassName );

	m_pszClassName = pszClassName;
}

void CBaseEntity::OnCreate()
{
}

void CBaseEntity::OnDestroy()
{
}

bool CBaseEntity::Spawn()
{
	return true;
}

CBaseEntity* CBaseEntity::Create( const char* const pszClassName, const glm::vec3& vecOrigin, const glm::vec3& vecAngles, const bool bSpawn )
{
	CBaseEntity* pEntity = GetEntityList().Create( pszClassName );

	pEntity->SetOrigin( vecOrigin );
	pEntity->SetAngles( vecAngles );

	if( bSpawn )
	{
		if( !pEntity->Spawn() )
		{
			GetEntityList().Remove( pEntity );
			return nullptr;
		}
	}

	return pEntity;
}