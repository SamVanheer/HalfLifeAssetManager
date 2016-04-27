#include <cassert>

#include "shared/Logging.h"

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
	CBaseEntity* pEntity = GetEntityDict().CreateEntity( pszClassName );

	//This is where you can handle custom entities.
	if( !pEntity )
	{
		Error( "Couldn't create \"%s\"!\n", pszClassName );
		return nullptr;
	}

	if( GetEntityList().Add( pEntity ) == entity::INVALID_ENTITY_INDEX )
	{
		GetEntityDict().DestroyEntity( pEntity );

		return nullptr;
	}

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