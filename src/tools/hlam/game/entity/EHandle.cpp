#include "game/entity/BaseEntity.hpp"
#include "game/entity/BaseEntityList.hpp"
#include "game/entity/EHandle.hpp"

BaseEntity* EHandle::Get(const BaseEntityList& entityList) const
{
	return entityList.GetEntityByHandle(*this);
}

void EHandle::Set(BaseEntity* entity)
{
	*this = entity ? entity->GetEntHandle() : EHandle{};
}
