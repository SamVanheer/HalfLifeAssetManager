#pragma once

#include <algorithm>

#include <glm/vec3.hpp>

#include "core/shared/Const.hpp"

/**
*	This class represents a bounding box.
*/
class BoundingBox final
{
public:
	/**
	*	@brief Constructs an invalid bounding box.
	*/
	explicit BoundingBox()
	{
		Invalidate();
	}

	/**
	*	@brief Constructs a bounding box with no size, centered around the given point.
	*/
	BoundingBox(const glm::vec3& point)
	{
		_mins = _maxs = point;
	}

	/**
	*	@brief Constructs a bounding box with the given mins and maxs.
	*/
	BoundingBox(const glm::vec3& mins, const glm::vec3& maxs)
	{
		Set(mins, maxs);
	}

	BoundingBox(const BoundingBox&) = default;
	BoundingBox& operator=(const BoundingBox&) = default;

	/**
	*	@brief Returns true if the box is empty (mins == maxs)
	*/
	bool IsEmpty() const
	{
		return _mins == _maxs;
	}

	/**
	*	@brief Returns whether this box is valid (has valid mins and maxs).
	*/
	bool IsValid() const
	{
		return	_mins != WORLD_INVALID_MIN &&
			_maxs != WORLD_INVALID_MAX;
	}

	/**
	*	@brief Returns whether the given point is inside this box.
	*/
	bool Inside(const glm::vec3& point) const
	{
		return	_mins.x <= point.x &&
			_mins.y <= point.y &&
			_mins.z <= point.z &&
			point.x <= _maxs.x &&
			point.y <= _maxs.y &&
			point.z <= _maxs.z;
	}

	/**
	*	@brief Returns whether this box and the given box overlap.
	*/
	bool Overlaps(const BoundingBox& other) const
	{
		return	other.Inside(_mins) ||
			other.Inside(_maxs) ||
			Inside(other._mins) ||
			Inside(other._maxs);
	}

	void Get(glm::vec3& mins, glm::vec3& maxs) const
	{
		mins = _mins;
		maxs = _maxs;
	}

	const glm::vec3& GetMins() const
	{
		return _mins;
	}

	const glm::vec3& GetMaxs() const
	{
		return _maxs;
	}

	glm::vec3 Center() const
	{
		return (_maxs + _mins) / 2.f;
	}

	glm::vec3 Size() const
	{
		return _maxs - _mins;
	}

	/**
	*	@brief Resets the box to have no size, and be centered around the world origin.
	*/
	void Reset()
	{
		_mins = _maxs = glm::vec3(0);
	}

	/**
	*	@brief Invalidates the contents of this box.
	*/
	void Invalidate()
	{
		_mins = WORLD_INVALID_MIN;
		_maxs = WORLD_INVALID_MAX;
	}

	/**
	*	@brief Resets the box's size to 0.
	*/
	void ResetSize()
	{
		const auto center = Center();
		_mins = _maxs = center;
	}

	/**
	*	@brief Centers the box around a given point.
	*/
	void CenterOnPoint(const glm::vec3& point)
	{
		const glm::vec3 halfSize = Size() / 2.0f;

		_mins = point - halfSize;
		_maxs = point + halfSize;
	}

	/**
	*	@brief Validates that the mins and maxs are correct.
	*/
	void Validate()
	{
		for (std::size_t index = 0; index < glm::vec3::length(); ++index)
		{
			if (_maxs[index] < _mins[index])
				std::swap(_maxs[index], _mins[index]);
		}
	}

	void Set(const glm::vec3& mins, const glm::vec3& maxs)
	{
		_mins = mins;
		_maxs = maxs;

		//Make sure they're correct.
		Validate();
	}

	void SetMins(const glm::vec3& mins)
	{
		_mins = mins;

		Validate();
	}

	void SetMaxs(const glm::vec3& maxs)
	{
		_maxs = maxs;

		Validate();
	}

	/**
	*	@brief Adds a point to the box. This will expand the box if the point is outside of it.
	*/
	void AddPoint(const glm::vec3& point)
	{
		for (std::size_t index = 0; index < glm::vec3::length(); ++index)
		{
			if (_mins[index] > point[index])
				_mins[index] = point[index];

			if (_mins[index] < point[index])
				_maxs[index] = point[index];
		}
	}

	bool operator==(const BoundingBox& other) const
	{
		return	_mins == other._mins &&
			_maxs == other._maxs;
	}

	bool operator!=(const BoundingBox& other) const
	{
		return !(*this == other);
	}

private:
	glm::vec3 _mins;
	glm::vec3 _maxs;
};
