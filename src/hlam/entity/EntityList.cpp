#include <algorithm>

#include "entity/BaseEntity.hpp"
#include "entity/EntityList.hpp"

#include "utility/WorldTime.hpp"

std::shared_ptr<BaseEntity> EntityList::GetEntityByIndex(std::size_t index) const
{
	if (index >= _entities.size())
	{
		return {};
	}

	return _entities[index];
}

void EntityList::RunFrame()
{
	auto time = _context->Time;

	for (auto& entity : _entities)
	{
		if (entity->AlwaysThink ||
			(entity->GetNextThinkTime() != 0 &&
				entity->GetNextThinkTime() <= time->GetTime() &&
				(time->GetTime() - time->GetFrameTime()) >= entity->GetLastThinkTime()))
		{
			//Set first so entities can do lastthink + delay.
			entity->SetLastThinkTime(time->GetTime());
			entity->SetNextThinkTime(0);

			entity->Think();
		}
	}
}

void EntityList::Add(const std::shared_ptr<BaseEntity>& entity)
{
	_entities.push_back(entity);
}

void EntityList::Destroy(const std::shared_ptr<BaseEntity>& entity)
{
	if (!entity)
	{
		return;
	}

	auto it = std::find(_entities.begin(), _entities.end(), entity);

	if (it == _entities.end())
	{
		//This shouldn't ever be hit, unless the entity was corrupted/not managed by this list
		assert(!"Invalid entity index");
		return;
	}

	// Entity will still exist until last strong reference has been cleared.
	_entities.erase(it);
}

void EntityList::DestroyAll()
{
	_entities.clear();
}
