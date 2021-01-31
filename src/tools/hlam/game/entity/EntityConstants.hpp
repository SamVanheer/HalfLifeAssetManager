#pragma once

#include <cstdint>

#include "core/shared/Utility.hpp"

namespace entity
{
/**
*	Type used for entity indices.
*/
typedef uint16_t EntIndex;

/**
*	Invalid entity index.
*/
const EntIndex INVALID_ENTITY_INDEX = 0xFFFF;

/**
*	Maximum number of entities that can exist at the same time.
*/
const EntIndex MAX_ENTITIES = 8192;

/**
*	Type used for entity serial numbers.
*/
typedef uint16_t EntSerial;

/**
*	Basic type used for entity handles.
*	Packs an
*/
typedef uint32_t EntHandle;

/**
*	Invalid value for entity handles.
*/
const EntHandle INVALID_ENTITY_HANDLE = 0xFFFFFFFF;

/**
*	Takes an entity index and serial number and returns them as an entity handle.
*/
inline constexpr EntHandle MakeEntHandle(const EntIndex index, const EntSerial serial)
{
	return static_cast<EntHandle>(index) + (static_cast<EntHandle>(serial) << 16);
}

/**
*	Gets the index out of a handle.
*/
inline constexpr EntIndex GetHandleIndex(const EntHandle handle)
{
	return static_cast<EntIndex>(handle & 0xFFFF);
}

/**
*	Gets the serial number out of a handle.
*/
inline constexpr EntSerial GetHandleSerial(const EntHandle handle)
{
	return static_cast<EntSerial>((handle >> 16) & 0xFFFF);
}

/**
*	Gets the entity index and serial number out of a handle.
*/
inline void GetHandleData(const EntHandle handle, EntIndex& index, EntSerial& serial)
{
	index = GetHandleIndex(handle);
	serial = GetHandleSerial(handle);
}

typedef unsigned int Flags;

enum Flag : Flags
{
	FL_NONE = 0,

	/**
	*	Marks an entity as needing to be removed at the end of the frame.
	*/
	FL_KILLME = Bit(0),

	/**
	*	If set, the entity's think method is called every frame.
	*/
	FL_ALWAYSTHINK = Bit(1)
};
}
