#pragma once

#include <limits>

#include "game/entity/EntityConstants.hpp"

class BaseEntity;
class BaseEntityList;

/**
*	@brief Class that represents an entity. Can be used to safely store references to entities.
*/
class EHandle
{
public:
	/**
	*	@brief Creates an invalid handle
	*/
	constexpr EHandle() = default;

	/**
	*	@brief Creates a handle that represents the given entity.
	*/
	EHandle(BaseEntity* entity)
	{
		*this = entity;
	}

	constexpr EHandle(std::size_t index, std::size_t serial)
	{
		Set(index, serial);
	}

	constexpr EHandle(const EHandle& other) = default;

	constexpr EHandle& operator=(const EHandle& other) = default;

	BaseEntity* Get(const BaseEntityList& entityList) const;

	bool IsValid(const BaseEntityList& entityList) const { return Get(entityList) != nullptr; }

	void Set(BaseEntity* entity);

	constexpr void Set(std::size_t index, std::size_t serial)
	{
		_index = index;
		_serial = serial;
	}

	EHandle& operator=(BaseEntity* entity)
	{
		Set(entity);
		return *this;
	}

	constexpr std::size_t GetIndex() const { return _index; }

	constexpr std::size_t GetSerialNumber() const { return _serial; }

private:
	std::size_t _index = std::numeric_limits<std::size_t>::max();
	std::size_t _serial = 0;
};
