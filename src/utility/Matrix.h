#ifndef UTILITY_MATRIX_H
#define UTILITY_MATRIX_H

#include <cassert>
#include <memory>

#include "common/Const.h"
#include "Vector.h"

class Matrix3x4 final
{
public:
	static const size_t NUM_ROWS = 3;
	static const size_t NUM_COLS = 4;

public:
	/**
	*	Default constructor. Initializes the matrix to an identity matrix.
	*/
	explicit Matrix3x4()
	{
		MakeIdentity();
	}

	/**
	*	Constructor. Sets all values to flValue.
	*/
	Matrix3x4( const vec_t flValue )
	{
		MakeN( flValue );
	}

	/**
	*	Constructor. Sets the matrix to the given values.
	*/
	Matrix3x4( const vec_t x0, const vec_t y0, const vec_t z0, const vec_t w0,
			   const vec_t x1, const vec_t y1, const vec_t z1, const vec_t w1,
			   const vec_t x2, const vec_t y2, const vec_t z2, const vec_t w2 )
	{
		m[ 0 ][ 0 ] = x0; m[ 0 ][ 1 ] = y0; m[ 0 ][ 2 ] = z0; m[ 0 ][ 3 ] = w0;
		m[ 1 ][ 0 ] = x1; m[ 1 ][ 1 ] = y1; m[ 1 ][ 2 ] = z1; m[ 1 ][ 3 ] = w1;
		m[ 2 ][ 0 ] = x2; m[ 2 ][ 1 ] = y2; m[ 2 ][ 2 ] = z2; m[ 2 ][ 3 ] = w2;
	}

	/**
	*	Constructor. Sets the matrix to the given values.
	*/
	Matrix3x4( const vec_t matrix[ NUM_ROWS ][ NUM_COLS ] )
	{
		memcpy( m, matrix, sizeof( m ) );
	}

	/**
	*	Copy constructor.
	*/
	Matrix3x4( const Matrix3x4& other ) = default;

	/**
	*	Assignment operator.
	*/
	Matrix3x4& operator=( const Matrix3x4& other ) = default;

	/**
	*	Turns this matrix into an identity matrix.
	*/
	void MakeIdentity()
	{
		MakeZero();

		for( size_t uiIndex = 0; uiIndex < NUM_ROWS; ++uiIndex )
		{
			m[ uiIndex ][ uiIndex ] = 1;
		}
	}

	/**
	*	Sets all values to the given value.
	*/
	void MakeN( const vec_t flValue )
	{
		//MakeZero is more efficient.
		if( flValue == 0 )
		{
			MakeZero();
			return;
		}

		for( size_t uiRow = 0; uiRow < NUM_ROWS; ++uiRow )
		{
			for( size_t uiCol = 0; uiCol < NUM_COLS; ++uiCol )
			{
				m[ uiRow ][ uiCol ] = flValue;
			}
		}
	}

	/**
	*	Sets all values to 1.
	*/
	void MakeOne()
	{
		MakeN( 1 );
	}

	/**
	*	Sets all values to 0.
	*/
	void MakeZero()
	{
		memset( m, 0, sizeof( m ) );
	}

	/**
	*	Rotates the vector.
	*/
	Vector Rotate( const Vector& vec ) const
	{
		return Vector(
			vec.Dot( ( *this )[ 0 ] ),
			vec.Dot( ( *this )[ 1 ] ),
			vec.Dot( ( *this )[ 2 ] )
		);
	}

	/**
	*	Rotates the vector by the inverse of the matrix.
	*/
	Vector IRotate( const Vector& vec ) const
	{
		return Vector(
			vec[ 0 ] * m[ 0 ][ 0 ] + vec[ 1 ] * m[ 1 ][ 0 ] + vec[ 2 ] * m[ 2 ][ 0 ],
			vec[ 0 ] * m[ 0 ][ 1 ] + vec[ 1 ] * m[ 1 ][ 1 ] + vec[ 2 ] * m[ 2 ][ 1 ],
			vec[ 0 ] * m[ 0 ][ 2 ] + vec[ 1 ] * m[ 1 ][ 2 ] + vec[ 2 ] * m[ 2 ][ 2 ]
		);
	}

	/**
	*	Gets a row as a Vector.
	*/
	const Vector& operator[]( const size_t uiIndex ) const
	{
		assert( uiIndex < NUM_ROWS );

		return *reinterpret_cast<const Vector*>( &m[ uiIndex ] );
	}

	/**
	*	@copydoc operator[]( const size_t uiIndex ) const
	*/
	Vector& operator[]( const size_t uiIndex )
	{
		assert( uiIndex < NUM_ROWS );

		return *reinterpret_cast<Vector*>( &m[ uiIndex ] );
	}

	/**
	*	Adds the given matrix to this one.
	*/
	Matrix3x4& operator+=( const Matrix3x4& other )
	{
		for( size_t uiRow = 0; uiRow < NUM_ROWS; ++uiRow )
		{
			for( size_t uiCol = 0; uiCol < NUM_COLS; ++uiCol )
			{
				m[ uiRow ][ uiCol ] += other.m[ uiRow ][ uiCol ];
			}
		}

		return *this;
	}

	/**
	*	Subtracts the given matrix from this one.
	*/
	Matrix3x4& operator-=( const Matrix3x4& other )
	{
		for( size_t uiRow = 0; uiRow < NUM_ROWS; ++uiRow )
		{
			for( size_t uiCol = 0; uiCol < NUM_COLS; ++uiCol )
			{
				m[ uiRow ][ uiCol ] -= other.m[ uiRow ][ uiCol ];
			}
		}

		return *this;
	}

	/**
	*	Multiplies this matrix by the given value.
	*/
	Matrix3x4& operator*=( const vec_t flValue )
	{
		for( size_t uiRow = 0; uiRow < NUM_ROWS; ++uiRow )
		{
			for( size_t uiCol = 0; uiCol < NUM_COLS; ++uiCol )
			{
				m[ uiRow ][ uiCol ] *= flValue;
			}
		}

		return *this;
	}

	/**
	*	Divides this matrix by the given value.
	*/
	Matrix3x4& operator/=( const vec_t flValue )
	{
		for( size_t uiRow = 0; uiRow < NUM_ROWS; ++uiRow )
		{
			for( size_t uiCol = 0; uiCol < NUM_COLS; ++uiCol )
			{
				m[ uiRow ][ uiCol ] /= flValue;
			}
		}

		return *this;
	}

	/**
	*	Adds the given matrix to a copy of this matrix and returns it.
	*/
	Matrix3x4 operator+( const Matrix3x4& other ) const
	{
		return Matrix3x4( *this ) += other;
	}

	/**
	*	Subtracts the given matrix from a copy of this matrix and returns it.
	*/
	Matrix3x4 operator-( const Matrix3x4& other ) const
	{
		return Matrix3x4( *this ) -= other;
	}

	/**
	*	Multiplies a copy of this matrix by the given value and returns it.
	*/
	Matrix3x4 operator*( const vec_t flValue ) const
	{
		return Matrix3x4( *this ) *= flValue;
	}

	/**
	*	Divides a copy of this matrix by the given value and returns it.
	*/
	Matrix3x4 operator/( const vec_t flValue ) const
	{
		return Matrix3x4( *this ) /= flValue;
	}

	/**
	*	The matrix itself.
	*/
	vec_t m[ NUM_ROWS ][ NUM_COLS ];
};

/**
*	Multiplies a matrix with a vector.
*	@param vec Vector to multiply.
*	@param matrix Matrix to multiply.
*	@return Vector resulting from this operation.
*/
inline Vector operator*( const Vector& vec, const Matrix3x4& matrix )
{
	return Vector(
		vec.Dot( matrix[ 0 ] ) + matrix.m[ 0 ][ 3 ],
		vec.Dot( matrix[ 1 ] ) + matrix.m[ 1 ][ 3 ],
		vec.Dot( matrix[ 2 ] ) + matrix.m[ 2 ][ 3 ]
	);
}

/**
*	Multiplies 2 matrices with one another.
*	@param lhs Left matrix.
*	@param rhs Right matrix.
*	@return Matrix resulting from this operation.
*/
inline Matrix3x4 operator*( const Matrix3x4& lhs, const Matrix3x4& rhs )
{
	vec_t matrix[ 3 ][ 4 ];

	matrix[ 0 ][ 0 ] =	lhs.m[ 0 ][ 0 ] * rhs.m[ 0 ][ 0 ] + lhs.m[ 0 ][ 1 ] * rhs.m[ 1 ][ 0 ] +
						lhs.m[ 0 ][ 2 ] * rhs.m[ 2 ][ 0 ];
	matrix[ 0 ][ 1 ] =	lhs.m[ 0 ][ 0 ] * rhs.m[ 0 ][ 1 ] + lhs.m[ 0 ][ 1 ] * rhs.m[ 1 ][ 1 ] +
						lhs.m[ 0 ][ 2 ] * rhs.m[ 2 ][ 1 ];
	matrix[ 0 ][ 2 ] =	lhs.m[ 0 ][ 0 ] * rhs.m[ 0 ][ 2 ] + lhs.m[ 0 ][ 1 ] * rhs.m[ 1 ][ 2 ] +
						lhs.m[ 0 ][ 2 ] * rhs.m[ 2 ][ 2 ];
	matrix[ 0 ][ 3 ] =	lhs.m[ 0 ][ 0 ] * rhs.m[ 0 ][ 3 ] + lhs.m[ 0 ][ 1 ] * rhs.m[ 1 ][ 3 ] +
						lhs.m[ 0 ][ 2 ] * rhs.m[ 2 ][ 3 ] + lhs.m[ 0 ][ 3 ];
	matrix[ 1 ][ 0 ] =	lhs.m[ 1 ][ 0 ] * rhs.m[ 0 ][ 0 ] + lhs.m[ 1 ][ 1 ] * rhs.m[ 1 ][ 0 ] +
						lhs.m[ 1 ][ 2 ] * rhs.m[ 2 ][ 0 ];
	matrix[ 1 ][ 1 ] =	lhs.m[ 1 ][ 0 ] * rhs.m[ 0 ][ 1 ] + lhs.m[ 1 ][ 1 ] * rhs.m[ 1 ][ 1 ] +
						lhs.m[ 1 ][ 2 ] * rhs.m[ 2 ][ 1 ];
	matrix[ 1 ][ 2 ] =	lhs.m[ 1 ][ 0 ] * rhs.m[ 0 ][ 2 ] + lhs.m[ 1 ][ 1 ] * rhs.m[ 1 ][ 2 ] +
						lhs.m[ 1 ][ 2 ] * rhs.m[ 2 ][ 2 ];
	matrix[ 1 ][ 3 ] =	lhs.m[ 1 ][ 0 ] * rhs.m[ 0 ][ 3 ] + lhs.m[ 1 ][ 1 ] * rhs.m[ 1 ][ 3 ] +
						lhs.m[ 1 ][ 2 ] * rhs.m[ 2 ][ 3 ] + lhs.m[ 1 ][ 3 ];
	matrix[ 2 ][ 0 ] =	lhs.m[ 2 ][ 0 ] * rhs.m[ 0 ][ 0 ] + lhs.m[ 2 ][ 1 ] * rhs.m[ 1 ][ 0 ] +
						lhs.m[ 2 ][ 2 ] * rhs.m[ 2 ][ 0 ];
	matrix[ 2 ][ 1 ] =	lhs.m[ 2 ][ 0 ] * rhs.m[ 0 ][ 1 ] + lhs.m[ 2 ][ 1 ] * rhs.m[ 1 ][ 1 ] +
						lhs.m[ 2 ][ 2 ] * rhs.m[ 2 ][ 1 ];
	matrix[ 2 ][ 2 ] =	lhs.m[ 2 ][ 0 ] * rhs.m[ 0 ][ 2 ] + lhs.m[ 2 ][ 1 ] * rhs.m[ 1 ][ 2 ] +
						lhs.m[ 2 ][ 2 ] * rhs.m[ 2 ][ 2 ];
	matrix[ 2 ][ 3 ] =	lhs.m[ 2 ][ 0 ] * rhs.m[ 0 ][ 3 ] + lhs.m[ 2 ][ 1 ] * rhs.m[ 1 ][ 3 ] +
						lhs.m[ 2 ][ 2 ] * rhs.m[ 2 ][ 3 ] + lhs.m[ 2 ][ 3 ];

	return matrix;
}

#endif //UTILITY_MATRIX_H