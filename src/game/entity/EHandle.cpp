#include "CBaseEntity.h"
#include "CBaseEntityList.h"

#include "EHandle.h"

EHandle::EHandle( CBaseEntity* pEntity )
{
	*this = pEntity;
}

void EHandle::Invalidate()
{
	m_Handle = entity::INVALID_ENTITY_HANDLE;
}

CBaseEntity* EHandle::Get() const
{
	return GetEntityList().GetEntityByHandle( *this );
}

void EHandle::Set( CBaseEntity* pEntity )
{
	if( pEntity )
	{
		*this = pEntity->GetEntHandle();
	}
	else
	{
		m_Handle = entity::INVALID_ENTITY_HANDLE;
	}
}

EHandle& EHandle::operator=( CBaseEntity* pEntity )
{
	Set( pEntity );

	return *this;
}

entity::EntIndex_t EHandle::GetEntIndex() const
{
	return entity::GetHandleIndex( m_Handle );
}

entity::EntSerial_t EHandle::GetSerialNumber() const
{
	return entity::GetHandleSerial( m_Handle );
}