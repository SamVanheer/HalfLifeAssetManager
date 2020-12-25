#pragma once

#include <cassert>
#include <unordered_map>

#include "utility/StringUtils.hpp"

class CBaseEntity;
class CBaseEntityRegistry;

struct EntityContext;

/**
*	Class used to find entity registries by name and create/destroy entities.
*/
class CEntityDict final
{
private:
	typedef std::unordered_map<const char*, const CBaseEntityRegistry*, Hash_C_String<const char*>, EqualTo_C_String<const char*>> EntityDict_t;

public:
	CEntityDict() = default;
	~CEntityDict() = default;

	/**
	*	@return The singleton instance of this class.
	*/
	static CEntityDict& GetInstance();

	/**
	*	Returns whether the requested entity type exists.
	*/
	bool HasEntity( const char* const pszClassName ) const;

	/**
	*	Returns the registry for the requested entity type.
	*/
	const CBaseEntityRegistry* FindEntity( const char* const pszClassName ) const;

	/**
	*	Adds a registry for an entity. Never call directly, use CEntityRegistry.
	*/
	bool AddEntity( const CBaseEntityRegistry* pRegistry );

	/**
	*	Creates an entity by entity name.
	*/
	CBaseEntity* CreateEntity( const char* const pszClassName, EntityContext* context ) const;

	/**
	*	Destroys an entity.
	*/
	void DestroyEntity( CBaseEntity* pEntity ) const;

private:
	EntityDict_t m_Dict;

private:
	CEntityDict( const CEntityDict& ) = delete;
	CEntityDict& operator=( const CEntityDict& ) = delete;
};

/**
*	Global list of entity types. Creates/destroys entities.
*/
CEntityDict& GetEntityDict();

/**
*	Base class for the entity registry.
*	Abstract.
*/
class CBaseEntityRegistry
{
public:
	CBaseEntityRegistry( const char* const pszClassname, const char* const pszInternalname, const size_t uiSizeInBytes )
		: m_pszClassname( pszClassname )
		, m_pszInternalname( pszInternalname )
		, m_uiSizeInBytes( uiSizeInBytes )
	{
		assert( pszClassname );
		assert( pszInternalname );
		assert( uiSizeInBytes > 0 );

		GetEntityDict().AddEntity( this );
	}

	/**
	*	Returns the entity's class name.
	*/
	const char* GetClassname() const { return m_pszClassname; }

	/**
	*	Returns the C++ class name.
	*/
	const char* GetInternalname() const { return m_pszInternalname; }

	/**
	*	Gets the size of the class, in bytes.
	*/
	size_t GetSize() const { return m_uiSizeInBytes; }

	/**
	*	Creates an instance of the entity represented by this registry.
	*/
	virtual CBaseEntity* Create() const = 0;

	/**
	*	Destroys an instance of the entity represented by this registry.
	*/
	virtual void Destroy( CBaseEntity* pEntity ) const = 0;

private:
	const char* const m_pszClassname;
	const char* const m_pszInternalname;
	const size_t m_uiSizeInBytes;

private:
	CBaseEntityRegistry( const CBaseEntityRegistry& ) = delete;
	CBaseEntityRegistry& operator=( const CBaseEntityRegistry& ) = delete;
};

/**
*	Class used to register entities.
*/
template<typename ENTITY>
class CEntityRegistry final : public CBaseEntityRegistry
{
public:
	CEntityRegistry( const char* const pszClassname, const char* const pszInternalname )
		: CBaseEntityRegistry( pszClassname, pszInternalname, sizeof( ENTITY ) )
	{
	}

	CBaseEntity* Create() const override final
	{
		return static_cast<CBaseEntity*>( new ENTITY() );
	}

	void Destroy( CBaseEntity* pEntity ) const override final
	{
		assert( pEntity );

		delete pEntity;
	}

private:
	CEntityRegistry( const CEntityRegistry& ) = delete;
	CEntityRegistry& operator=( const CEntityRegistry& ) = delete;
};

/**
*	Use this macro to register entities.
*/
#define LINK_ENTITY_TO_CLASS( className, internalName )									\
static const CEntityRegistry<internalName> __g_##className( #className, #internalName )
