#ifndef UTILITY_BOUNDINGBOX_H
#define UTILITY_BOUNDINGBOX_H

#include <algorithm>

#include "Vector.h"

/**
*	This class represents a bounding box.
*/
class BoundingBox final
{
public:
	/**
	*	Default constructor. Constructs an invalid bounding box.
	*/
	explicit BoundingBox()
	{
		Invalidate();
	}

	/**
	*	Constructor. Constructs a bounding box with no size, centered around the given point.
	*/
	BoundingBox( const Vector& vecPoint )
	{
		m_vecMins = m_vecMaxs = vecPoint;
	}

	/**
	*	Constructor. Constructs a bounding box with the given mins and maxs.
	*/
	BoundingBox( const Vector& vecMins, const Vector& vecMaxs )
	{
		Set( vecMins, vecMaxs );
	}

	/**
	*	Copy constructor.
	*/
	BoundingBox( const BoundingBox& other ) = default;

	/**
	*	Assignment operator.
	*/
	BoundingBox& operator=( const BoundingBox& other ) = default;

	/**
	*	Returns true if the box is empty (mins == maxs)
	*/
	bool IsEmpty() const
	{
		return m_vecMins == m_vecMaxs;
	}

	/**
	*	Returns whether this box is valid (has valid mins and maxs).
	*/
	bool IsValid() const
	{
		return	m_vecMins != WORLD_INVALID_MIN && 
				m_vecMaxs != WORLD_INVALID_MAX;
	}

	/**
	*	Returns whether the given point is inside this box.
	*/
	bool Inside( const Vector& vecPoint ) const
	{
		return	m_vecMins.x <= vecPoint.x &&
			m_vecMins.y <= vecPoint.y &&
			m_vecMins.z <= vecPoint.z &&
			vecPoint.x <= m_vecMaxs.x &&
			vecPoint.y <= m_vecMaxs.y &&
			vecPoint.z <= m_vecMaxs.z;
	}

	/**
	*	Returns whether this box and the given box overlap.
	*/
	bool Overlaps( const BoundingBox& other ) const
	{
		return	other.Inside( m_vecMins ) ||
			other.Inside( m_vecMaxs ) ||
			Inside( other.m_vecMins ) ||
			Inside( other.m_vecMaxs );
	}

	/**
	*	Gets the bounds.
	*/
	void Get( Vector& vecMins, Vector& vecMaxs ) const
	{
		vecMins = m_vecMins;
		vecMaxs = m_vecMaxs;
	}

	/**
	*	Gets the minimum bounds.
	*/
	const Vector& GetMins() const
	{
		return m_vecMins;
	}

	/**
	*	Gets the maximum bounds.
	*/
	const Vector& GetMaxs() const
	{
		return m_vecMaxs;
	}

	/**
	*	Returns the center of the box.
	*/
	Vector Center() const
	{
		return m_vecMaxs - m_vecMins;
	}

	/**
	*	Returns the size of the box.
	*/
	Vector Size() const
	{
		return m_vecMaxs - m_vecMins;
	}

	/**
	*	Resets the box to have no size, and be centered around the world origin.
	*/
	void Reset()
	{
		m_vecMins = m_vecMaxs = Vector();
	}

	/**
	*	Invalidates the contents of this box.
	*/
	void Invalidate()
	{
		m_vecMins = WORLD_INVALID_MIN;
		m_vecMaxs = WORLD_INVALID_MAX;
	}

	/**
	*	Resets the box's size to 0.
	*/
	void ResetSize()
	{
		m_vecMins = m_vecMaxs = Center();
	}

	/**
	*	Centers the box around a given point.
	*/
	void Center( const Vector& vecPoint )
	{
		const Vector vecHalfSize = Size() / 2;

		m_vecMins = vecPoint - vecHalfSize;
		m_vecMaxs = vecPoint + vecHalfSize;
	}

	/**
	*	Validates that the mins and maxs are correct.
	*/
	void Validate()
	{
		for( size_t uiIndex = 0; uiIndex < Vector::NUM_COMPONENTS; ++uiIndex )
		{
			if( m_vecMaxs[ uiIndex ] < m_vecMins[ uiIndex ] )
				std::swap( m_vecMaxs[ uiIndex ], m_vecMins[ uiIndex ] );
		}
	}

	/**
	*	Sets the bounding box to the given mins and maxs.
	*/
	void Set( const Vector& vecMins, const Vector& vecMaxs )
	{
		m_vecMins = vecMins;
		m_vecMaxs = vecMaxs;

		//Make sure they're correct.
		Validate();
	}

	/**
	*	Sets the minimum bounds.
	*/
	void SetMins( const Vector& vecMins )
	{
		m_vecMins = vecMins;

		Validate();
	}

	/**
	*	Sets the maximum bounds.
	*/
	void SetMaxs( const Vector& vecMaxs )
	{
		m_vecMaxs = vecMaxs;

		Validate();
	}

	/**
	*	Adds a point to the box. This will expand the box if the point is outside of it.
	*/
	void AddPoint( const Vector& vecPoint )
	{
		for( size_t uiIndex = 0; uiIndex < Vector::NUM_COMPONENTS; ++uiIndex )
		{
			if( m_vecMins[ uiIndex ] > vecPoint[ uiIndex ] )
				m_vecMins[ uiIndex ] = vecPoint[ uiIndex ];

			if( m_vecMaxs[ uiIndex ] < vecPoint[ uiIndex ] )
				m_vecMaxs[ uiIndex ] = vecPoint[ uiIndex ];
		}
	}

	/**
	*	Equality operator.
	*/
	bool operator==( const BoundingBox& other ) const
	{
		return	m_vecMins == other.m_vecMins &&
				m_vecMaxs == other.m_vecMaxs;
	}

	/**
	*	Inequality operator.
	*/
	bool operator!=( const BoundingBox& other ) const
	{
		return !( *this == other );
	}

private:
	Vector m_vecMins;
	Vector m_vecMaxs;
};

#endif //UTILITY_BOUNDINGBOX_H