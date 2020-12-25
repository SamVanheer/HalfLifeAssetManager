#pragma once

#include <cstdint>

#include "core/shared/Utility.hpp"

namespace entity
{
/**
*	Type used for entity indices.
*/
typedef uint16_t EntIndex_t;

/**
*	Invalid entity index.
*/
const EntIndex_t INVALID_ENTITY_INDEX = 0xFFFF;

/**
*	Maximum number of entities that can exist at the same time.
*/
const EntIndex_t MAX_ENTITIES = 8192;

/**
*	Type used for entity serial numbers.
*/
typedef uint16_t EntSerial_t;

/**
*	Basic type used for entity handles.
*	Packs an 
*/
typedef uint32_t EntHandle_t;

/**
*	Invalid value for entity handles.
*/
const EntHandle_t INVALID_ENTITY_HANDLE = 0xFFFFFFFF;

/**
*	Takes an entity index and serial number and returns them as an entity handle.
*/
inline constexpr EntHandle_t MakeEntHandle( const EntIndex_t index, const EntSerial_t serial )
{
	return static_cast<EntHandle_t>( index ) + ( static_cast<EntHandle_t>( serial ) << 16 );
}

/**
*	Gets the index out of a handle.
*/
inline constexpr EntIndex_t GetHandleIndex( const EntHandle_t handle )
{
	return static_cast<EntIndex_t>( handle & 0xFFFF );
}

/**
*	Gets the serial number out of a handle.
*/
inline constexpr EntSerial_t GetHandleSerial( const EntHandle_t handle )
{
	return static_cast<EntSerial_t>( ( handle >> 16 ) & 0xFFFF );
}

/**
*	Gets the entity index and serial number out of a handle.
*/
inline void GetHandleData( const EntHandle_t handle, EntIndex_t& index, EntSerial_t& serial )
{
	index = GetHandleIndex( handle );
	serial = GetHandleSerial( handle);
}

typedef unsigned int Flags_t;

enum Flag : Flags_t
{
	FL_NONE			= 0,

	/**
	*	Marks an entity as needing to be removed at the end of the frame.
	*/
	FL_KILLME		= Bit( 0 ),

	/**
	*	If set, the entity's think method is called every frame.
	*/
	FL_ALWAYSTHINK	= Bit( 1 )
};
}
