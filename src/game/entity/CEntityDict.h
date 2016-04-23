#ifndef GAME_ENTITY_CENTITYDICT_H
#define GAME_ENTITY_CENTITYDICT_H

#include <cassert>
#include <unordered_map>

#include "common/Utility.h"

class CBaseEntity;
class CBaseEntityRegistry;

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
	CBaseEntity* CreateEntity( const char* const pszClassName ) const;

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
	CBaseEntityRegistry( const char* const pszClassName, const char* const pszInternalName, const size_t uiSizeInBytes )
		: m_pszClassName( pszClassName )
		, m_pszInternalName( pszInternalName )
		, m_uiSizeInBytes( uiSizeInBytes )
	{
		assert( pszClassName );
		assert( pszInternalName );
		assert( uiSizeInBytes > 0 );

		GetEntityDict().AddEntity( this );
	}

	/**
	*	Returns the entity's class name.
	*/
	const char* GetClassName() const { return m_pszClassName; }

	/**
	*	Returns the C++ class name.
	*/
	const char* GetInternalName() const { return m_pszInternalName; }

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
	const char* const m_pszClassName;
	const char* const m_pszInternalName;
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
	CEntityRegistry( const char* const pszClassName, const char* const pszInternalName )
		: CBaseEntityRegistry( pszClassName, pszInternalName, sizeof( ENTITY ) )
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

#endif //GAME_ENTITY_CENTITYDICT_H