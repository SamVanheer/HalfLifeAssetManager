#include "game/entity/BaseEntity.hpp"
#include "game/entity/BaseEntityList.hpp"
#include "game/entity/EHandle.hpp"

EHandle::EHandle(BaseEntity* entity)
{
	*this = entity;
}

void EHandle::Invalidate()
{
	_handle = entity::INVALID_ENTITY_HANDLE;
}

BaseEntity* EHandle::Get(const BaseEntityList& entityList) const
{
	return entityList.GetEntityByHandle(*this);
}

void EHandle::Set(BaseEntity* entity)
{
	if (entity)
	{
		*this = entity->GetEntHandle();
	}
	else
	{
		_handle = entity::INVALID_ENTITY_HANDLE;
	}
}

EHandle& EHandle::operator=(BaseEntity* entity)
{
	Set(entity);

	return *this;
}

entity::EntIndex_t EHandle::GetEntIndex() const
{
	return entity::GetHandleIndex(_handle);
}

entity::EntSerial_t EHandle::GetSerialNumber() const
{
	return entity::GetHandleSerial(_handle);
}
