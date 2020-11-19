#include <cassert>

#include "CBaseEntity.h"

CBaseEntity::CBaseEntity()
{
}

CBaseEntity::~CBaseEntity()
{
}

void CBaseEntity::Construct(const char* const pszClassName, EntityContext* context)
{
	assert( pszClassName && *pszClassName );
	assert(context);

	m_pszClassName = pszClassName;
	_context = context;
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
