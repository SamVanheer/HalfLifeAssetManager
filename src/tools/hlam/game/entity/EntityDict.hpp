#pragma once

#include <cassert>
#include <unordered_map>

#include "utility/StringUtils.hpp"

class BaseEntity;
class BaseEntityRegistry;

struct EntityContext;

/**
*	Class used to find entity registries by name and create/destroy entities.
*/
class EntityDict final
{
private:
	typedef std::unordered_map<const char*, const BaseEntityRegistry*, Hash_C_String<const char*>, EqualTo_C_String<const char*>> EntityDict_t;

public:
	EntityDict() = default;
	~EntityDict() = default;

	/**
	*	@return The singleton instance of this class.
	*/
	static EntityDict& GetInstance();

	/**
	*	Returns whether the requested entity type exists.
	*/
	bool HasEntity(const char* const className) const;

	/**
	*	Returns the registry for the requested entity type.
	*/
	const BaseEntityRegistry* FindEntity(const char* const className) const;

	/**
	*	Adds a registry for an entity. Never call directly, use CEntityRegistry.
	*/
	bool AddEntity(const BaseEntityRegistry* registry);

	/**
	*	Creates an entity by entity name.
	*/
	BaseEntity* CreateEntity(const char* const className, EntityContext* context) const;

	/**
	*	Destroys an entity.
	*/
	void DestroyEntity(BaseEntity* entity) const;

private:
	EntityDict_t _dict;

private:
	EntityDict(const EntityDict&) = delete;
	EntityDict& operator=(const EntityDict&) = delete;
};

/**
*	Global list of entity types. Creates/destroys entities.
*/
EntityDict& GetEntityDict();

/**
*	Base class for the entity registry.
*	Abstract.
*/
class BaseEntityRegistry
{
public:
	BaseEntityRegistry(const char* const className, const char* const internalName, const size_t sizeInBytes)
		: _className(className)
		, _internalName(internalName)
		, _sizeInBytes(sizeInBytes)
	{
		assert(className);
		assert(internalName);
		assert(sizeInBytes > 0);

		GetEntityDict().AddEntity(this);
	}

	BaseEntityRegistry(const BaseEntityRegistry&) = delete;
	BaseEntityRegistry& operator=(const BaseEntityRegistry&) = delete;

	/**
	*	Returns the entity's class name.
	*/
	const char* GetClassname() const { return _className; }

	/**
	*	Returns the C++ class name.
	*/
	const char* GetInternalname() const { return _internalName; }

	/**
	*	Gets the size of the class, in bytes.
	*/
	size_t GetSize() const { return _sizeInBytes; }

	/**
	*	Creates an instance of the entity represented by this registry.
	*/
	virtual BaseEntity* Create() const = 0;

	/**
	*	Destroys an instance of the entity represented by this registry.
	*/
	virtual void Destroy(BaseEntity* entity) const = 0;

private:
	const char* const _className;
	const char* const _internalName;
	const size_t _sizeInBytes;
};

/**
*	Class used to register entities.
*/
template<typename ENTITY>
class EntityRegistry final : public BaseEntityRegistry
{
public:
	EntityRegistry(const char* const pszClassname, const char* const pszInternalname)
		: BaseEntityRegistry(pszClassname, pszInternalname, sizeof(ENTITY))
	{
	}

	EntityRegistry(const EntityRegistry&) = delete;
	EntityRegistry& operator=(const EntityRegistry&) = delete;

	BaseEntity* Create() const override final
	{
		return static_cast<BaseEntity*>(new ENTITY());
	}

	void Destroy(BaseEntity* entity) const override final
	{
		assert(entity);

		delete entity;
	}
};

/**
*	Use this macro to register entities.
*/
#define LINK_ENTITY_TO_CLASS(className, internalName)									\
static const EntityRegistry<internalName> __g_##className(#className, #internalName)
