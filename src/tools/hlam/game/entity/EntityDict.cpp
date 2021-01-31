#include "core/shared/Logging.hpp"

#include "game/entity/BaseEntity.hpp"
#include "game/entity/EntityDict.hpp"

EntityDict& GetEntityDict()
{
	return EntityDict::GetInstance();
}

EntityDict& EntityDict::GetInstance()
{
	//Changed to use this approach so the instance is constructed the first time something needs the dictionary.
	static EntityDict instance;

	return instance;
}

bool EntityDict::HasEntity(const char* const className) const
{
	return FindEntity(className) != nullptr;
}

const BaseEntityRegistry* EntityDict::FindEntity(const char* const className) const
{
	assert(className);

	auto it = _dict.find(className);

	return it != _dict.end() ? it->second : nullptr;
}

bool EntityDict::AddEntity(const BaseEntityRegistry* registry)
{
	assert(registry);

	if (FindEntity(registry->GetClassname()))
	{
		//This shouldn't happen, since there'd be duplicate symbols in the library.
		Error("EntityDict::AddEntity: Duplicate entity \"%s\" added!\n", registry->GetClassname());
		return false;
	}

	_dict.insert(std::make_pair(registry->GetClassname(), registry));

	return true;
}

BaseEntity* EntityDict::CreateEntity(const char* const className, EntityContext* context) const
{
	const auto registry = FindEntity(className);

	if (!registry)
	{
		return nullptr;
	}

	if (BaseEntity* entity = registry->Create(); entity)
	{
		entity->Construct(className, context);
		entity->OnCreate();

		return entity;
	}

	return nullptr;
}

void EntityDict::DestroyEntity(BaseEntity* entity) const
{
	assert(entity);

	const auto registry = FindEntity(entity->GetClassName());

	if (!registry)
	{
		Error("EntityDict::DestroyEntity: Tried to destroy unknown entity \"%s\"!\n", entity->GetClassName());
		return;
	}

	entity->OnDestroy();

	registry->Destroy(entity);
}
