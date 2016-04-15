#ifndef UTILITY_VECTOR_H
#define UTILITY_VECTOR_H

#include <cassert>
#include <cmath>

#include "common/Const.h"

class Vector2D;

/**
*	3D vector.
*/
class Vector final
{
public:
	static const size_t NUM_COMPONENTS = 3;

public:
	/**
	*	Default constructor. Sets all components to flValue. Default 0.
	*/
	explicit Vector( const vec_t flValue = 0.0f )
	{
		Set( flValue );
	}

	/**
	*	Constructor. Sets all components.
	*/
	Vector( const vec_t x, const vec_t y, const vec_t z )
	{
		Set( x, y, z );
	}

	/**
	*	Copy constructor.
	*/
	Vector( const Vector& other ) = default;

	/**
	*	Assignment operator.
	*/
	Vector& operator=( const Vector& other ) = default;

	/**
	*	Constructs a 3D vector using the given 2D vector's x and y components. z is set to 0.
	*/
	Vector( const Vector2D& other );

	/**
	*	Sets the x and y components to the given 2D vector's x and y components. z is set to 0.
	*/
	Vector& operator=( const Vector2D& other );

	/**
	*	Gets all 3 components.
	*/
	void Get( vec_t& x, vec_t& y, vec_t& z )
	{
		x = this->x;
		y = this->y;
		z = this->z;
	}

	/**
	*	Gets the component at the given index.
	*/
	vec_t operator[]( const size_t uiIndex ) const
	{
		assert( uiIndex < NUM_COMPONENTS );

		return data[ uiIndex ];
	}

	/**
	*	@copydoc operator[]( const size_t uiIndex ) const
	*/
	vec_t& operator[]( const size_t uiIndex )
	{
		assert( uiIndex < NUM_COMPONENTS );

		return data[ uiIndex ];
	}

	/**
	*	Sets all 3 components.
	*/
	void Set( const vec_t x, const vec_t y, const vec_t z )
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	/**
	*	Sets all 3 components to flValue. Default 0.
	*/
	void Set( const vec_t flValue = 0.0f )
	{
		x = y = z = flValue;
	}

	/**
	*	Returns the dot product between this and the given vector.
	*/
	vec_t Dot( const Vector& other ) const
	{
		return x * other.x + y * other.y + z * other.z;
	}

	/**
	*	Returns the cross product between this and the given vector.
	*/
	Vector Cross( const Vector& other ) const
	{
		return Vector(
			y * other.z - z * other.y,
			z * other.x - x * other.z,
			x * other.y - y * other.x
		);
	}

	/**
	*	Returns the vector's length, squared.
	*/
	vec_t LengthSquared() const
	{
		return Dot( *this );
	}

	/**
	*	Returns the vector's length.
	*/
	vec_t Length() const
	{
		return sqrt( LengthSquared() );
	}

	/**
	*	Normalizes the vector's length.
	*	@return Old length of the vector.
	*/
	vec_t Normalize()
	{
		const vec_t flLength = Length();

		*this /= flLength;

		return flLength;
	}

	/**
	*	Returns true if this vector and the given vector are the same, to within EQUAL_EPSILON difference.
	*/
	bool Equals( const Vector& other ) const
	{
		for( size_t i = 0; i < NUM_COMPONENTS; ++i )
		{
			if( fabs( data[ i ] - other[ i ] ) > EQUAL_EPSILON )
				return false;
		}

		return true;
	}

	/**
	*	Returns the average of all components.
	*/
	vec_t Average() const
	{
		return ( x + y + z ) / 3;
	}

	/**
	*	Multiplies vec with flValue and adds it to a copy of this vector.
	*/
	Vector MA( const vec_t flValue, const Vector& vec ) const;

	/**
	*	Inverses the vector's components.
	*/
	void Inverse()
	{
		x = -x;
		y = -y;
		z = -z;
	}

	//Operators
	/**
	*	Adds the given value to all components.
	*/
	Vector& operator+=( const vec_t flValue )
	{
		x += flValue;
		y += flValue;
		z += flValue;

		return *this;
	}

	/**
	*	Subtracts the given value from all components.
	*/
	Vector& operator-=( const vec_t flValue )
	{
		x -= flValue;
		y -= flValue;
		z -= flValue;

		return *this;
	}

	/**
	*	Multiplies all components by the given value.
	*/
	Vector& operator*=( const vec_t flValue )
	{
		x *= flValue;
		y *= flValue;
		z *= flValue;

		return *this;
	}

	/**
	*	Divides all components by the given value.
	*/
	Vector& operator/=( const vec_t flValue )
	{
		x /= flValue;
		y /= flValue;
		z /= flValue;

		return *this;
	}

	/**
	*	Adds the given vector's components to this one.
	*/
	Vector& operator+=( const Vector& other )
	{
		x += other.x;
		y += other.y;
		z += other.z;

		return *this;
	}

	/**
	*	Subtracts the given vector's components from this one.
	*/
	Vector& operator-=( const Vector& other )
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;

		return *this;
	}

	/**
	*	Adds the given value to a copy of this vector's components.
	*/
	Vector operator+( const vec_t flValue ) const
	{
		return Vector( *this ) += flValue;
	}

	/**
	*	Subtracts the given value from a copy of this vector's components.
	*/
	Vector operator-( const vec_t flValue ) const
	{
		return Vector( *this ) -= flValue;
	}

	/**
	*	Multiplies copy of this vector's components by the given value.
	*/
	Vector operator*( const vec_t flValue ) const
	{
		return Vector( *this ) *= flValue;
	}

	/**
	*	Divides copy of this vector's components by the given value.
	*/
	Vector operator/( const vec_t flValue ) const
	{
		return Vector( *this ) /= flValue;
	}

	/**
	*	Adds the given vector's components to a copy of this vector's components.
	*/
	Vector operator+( const Vector& other ) const
	{
		return Vector( *this ) += other;
	}

	/**
	*	Subtracts the given vector's components from a copy of this vector's components.
	*/
	Vector operator-( const Vector& other ) const
	{
		return Vector( *this ) -= other;
	}

	explicit operator const vec_t*() const { return data; }
	explicit operator vec_t*() { return data; }

	union
	{
		struct
		{
			/**
			*	X component.
			*/
			vec_t x;

			/**
			*	Y component.
			*/
			vec_t y;

			/**
			*	Z component.
			*/
			vec_t z;
		};

		/**
		*	All components as an array.
		*/
		vec_t data[ NUM_COMPONENTS ];
	};
};

inline Vector operator*( const vec_t flValue, const Vector& vec )
{
	return vec * flValue;
}

inline Vector operator/( const vec_t flValue, const Vector& vec )
{
	return vec / flValue;
}

inline Vector Vector::MA( const vec_t flValue, const Vector& vec ) const
{
	return ( *this + ( flValue * vec ) );
};

/**
*	2D vector.
*/
class Vector2D final
{
public:
	static const size_t NUM_COMPONENTS = 2;

public:

	/**
	*	Default constructor. Sets all components to flValue. Default 0;
	*/
	explicit Vector2D( const vec_t flValue = 0.0f )
	{
		Set( flValue );
	}

	Vector2D( const vec_t x, const vec_t y )
	{
		Set( x, y );
	}

	/**
	*	Copy constructor.
	*/
	Vector2D( const Vector2D& other ) = default;

	/**
	*	Assignment operator.
	*/
	Vector2D& operator=( const Vector2D& other ) = default;

	/**
	*	Constructs a 2D vector from a 3D vector, using the x and y components.
	*/
	Vector2D( const Vector& other )
	{
		Set( other.x, other.y );
	}

	/**
	*	Sets the x and y components to the given 3D vector's x and y components.
	*/
	Vector2D& operator=( const Vector& other )
	{
		Set( other.x, other.y );

		return *this;
	}

	/**
	*	Gets all 2 components.
	*/
	void Get( vec_t& x, vec_t& y )
	{
		x = this->x;
		y = this->y;
	}

	/**
	*	Gets the component at the given index.
	*/
	vec_t operator[]( const size_t uiIndex ) const
	{
		assert( uiIndex < NUM_COMPONENTS );

		return data[ uiIndex ];
	}

	/**
	*	@copydoc operator[]( const size_t uiIndex ) const
	*/
	vec_t& operator[]( const size_t uiIndex )
	{
		assert( uiIndex < NUM_COMPONENTS );

		return data[ uiIndex ];
	}

	/**
	*	Sets all 2 components.
	*/
	void Set( const vec_t x, const vec_t y )
	{
		this->x = x;
		this->y = y;
	}

	/**
	*	Sets all 2 components to flValue. Default 0;
	*/
	void Set( const vec_t flValue = 0.0f )
	{
		x = y = flValue;
	}

	/**
	*	Returns the dot product between this and the given vector.
	*/
	vec_t Dot( const Vector2D& other ) const
	{
		return x * other.x + y * other.y;
	}

	/**
	*	Returns the vector's length, squared.
	*/
	vec_t LengthSquared() const
	{
		return Dot( *this );
	}

	/**
	*	Returns the vector's length.
	*/
	vec_t Length() const
	{
		return sqrt( LengthSquared() );
	}

	/**
	*	Normalizes the vector's length.
	*	@return Old length of the vector.
	*/
	vec_t Normalize()
	{
		const vec_t flLength = Length();

		*this /= flLength;

		return flLength;
	}

	/**
	*	Returns true if this vector and the given vector are the same, to within EQUAL_EPSILON difference.
	*/
	bool Equals( const Vector2D& other ) const
	{
		for( size_t i = 0; i < NUM_COMPONENTS; ++i )
		{
			if( fabs( data[ i ] - other[ i ] ) > EQUAL_EPSILON )
				return false;
		}

		return true;
	}

	/**
	*	Returns the average of all components.
	*/
	vec_t Average() const
	{
		return ( x + y ) / 3;
	}

	/**
	*	Multiplies vec with flValue and adds it to a copy of this vector.
	*/
	Vector2D MA( const vec_t flValue, const Vector2D& vec ) const;

	/**
	*	Inverses the vector's components.
	*/
	void Inverse()
	{
		x = -x;
		y = -y;
	}

	//Operators
	/**
	*	Adds the given value to all components.
	*/
	Vector2D& operator+=( const vec_t flValue )
	{
		x += flValue;
		y += flValue;

		return *this;
	}

	/**
	*	Subtracts the given value from all components.
	*/
	Vector2D& operator-=( const vec_t flValue )
	{
		x -= flValue;
		y -= flValue;

		return *this;
	}

	/**
	*	Multiplies all components by the given value.
	*/
	Vector2D& operator*=( const vec_t flValue )
	{
		x *= flValue;
		y *= flValue;

		return *this;
	}

	/**
	*	Divides all components by the given value.
	*/
	Vector2D& operator/=( const vec_t flValue )
	{
		x /= flValue;
		y /= flValue;

		return *this;
	}

	/**
	*	Adds the given vector's components to this one.
	*/
	Vector2D& operator+=( const Vector2D& other )
	{
		x += other.x;
		y += other.y;

		return *this;
	}

	/**
	*	Subtracts the given vector's components from this one.
	*/
	Vector2D& operator-=( const Vector2D& other )
	{
		x -= other.x;
		y -= other.y;

		return *this;
	}

	/**
	*	Adds the given value to a copy of this vector's components.
	*/
	Vector2D operator+( const vec_t flValue ) const
	{
		return Vector2D( *this ) += flValue;
	}

	/**
	*	Subtracts the given value from a copy of this vector's components.
	*/
	Vector2D operator-( const vec_t flValue ) const
	{
		return Vector2D( *this ) -= flValue;
	}

	/**
	*	Multiplies copy of this vector's components by the given value.
	*/
	Vector2D operator*( const vec_t flValue ) const
	{
		return Vector2D( *this ) *= flValue;
	}

	/**
	*	Divides copy of this vector's components by the given value.
	*/
	Vector2D operator/( const vec_t flValue ) const
	{
		return Vector2D( *this ) /= flValue;
	}

	/**
	*	Adds the given vector's components to a copy of this vector's components.
	*/
	Vector2D operator+( const Vector2D& other ) const
	{
		return Vector2D( *this ) += other;
	}

	/**
	*	Subtracts the given vector's components from a copy of this vector's components.
	*/
	Vector2D operator-( const Vector2D& other ) const
	{
		return Vector2D( *this ) -= other;
	}

	explicit operator const vec_t*() const { return data; }
	explicit operator vec_t*() { return data; }

	union
	{
		struct
		{
			/**
			*	X component.
			*/
			vec_t x;

			/**
			*	Y component.
			*/
			vec_t y;
		};

		/**
		*	All components as an array.
		*/
		vec_t data[ NUM_COMPONENTS ];
	};
};

inline Vector2D operator*( const vec_t flValue, const Vector2D& vec )
{
	return vec * flValue;
}

inline Vector2D operator/( const vec_t flValue, const Vector2D& vec )
{
	return vec / flValue;
}

inline Vector::Vector( const Vector2D& other )
{
	Set( other.x, other.y, 0 );
}

inline Vector& Vector::operator=( const Vector2D& other )
{
	Set( other.x, other.y, 0 );

	return *this;
}

inline Vector2D Vector2D::MA( const vec_t flValue, const Vector2D& vec ) const
{
	return ( *this + ( flValue * vec ) );
}

extern const Vector vec3_origin;

#endif //UTILITY_VECTOR_H