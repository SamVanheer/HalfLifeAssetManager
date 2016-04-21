/***
*
*	Copyright (c) 1998, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/

#ifndef __MATHLIB__
#define __MATHLIB__

// mathlib.h

#include <math.h>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat3x4.hpp>

#ifdef DOUBLEVEC_T
typedef double vec_t;
#else
typedef float vec_t;
#endif
typedef vec_t vec3_t[3];	// x,y,z
typedef vec_t vec4_t[4];	// x,y,z,w

#define	SIDE_FRONT		0
#define	SIDE_ON			2
#define	SIDE_BACK		1
#define	SIDE_CROSS		-2

#define	Q_PI	3.14159265358979323846

// Use this definition globally
#define	ON_EPSILON		0.01
#define	EQUAL_EPSILON	0.001

int VectorCompare (vec3_t v1, vec3_t v2);

/**
*	Compares the given vectors.
*	@param lhs
*	@param rhs
*	@return true if the vectors are equal, to within EQUAL_EPSILON difference for all components, false otherwise.
*/
bool VectorCompare( const glm::vec3& lhs, const glm::vec3& rhs );

#define DotProduct(x,y) ((x)[0]*(y)[0]+(x)[1]*(y)[1]+(x)[2]*(y)[2])
#define VectorCopy(a,b) {(b)[0]=(a)[0];(b)[1]=(a)[1];(b)[2]=(a)[2];}
#define VectorScale(a,b,c) {(c)[0]=(b)*(a)[0];(c)[1]=(b)*(a)[1];(c)[2]=(b)*(a)[2];}

/**
*	Sets all of the given vector's components to the given value.
*	@param vec Vector to fill.
*	@param flValue Value to use.
*/
inline void VectorFill( glm::vec3& vec, const vec_t flValue )
{
	vec[ 0 ] = vec[ 1 ] = vec[ 2 ] = flValue;
}

/**
*	Normalizes a vector in place and returns the old length.
*/
vec_t VectorNormalize( glm::vec3& vector );

/**
*	Computes the average of the given vector's components.
*	@param vec Vector whose components to average.
*	@return Average.
*/
inline constexpr vec_t VectorAvg( const glm::vec3& vec )
{
	return ( vec.x + vec.y + vec.z ) / 3;
}

/**
*	Performs the operation vc = va + scale * vb
*	@param va
*	@param scale
*	@param vb
*	@param vc Result.
*/
void VectorMA( const glm::vec3& va, double scale, const glm::vec3& vb, glm::vec3& vc );

void AngleMatrix (const vec3_t angles, float matrix[3][4] );
void AngleIMatrix (const vec3_t angles, float matrix[3][4] );
void R_ConcatTransforms (const float in1[3][4], const float in2[3][4], float out[3][4]);

void VectorIRotate (const vec3_t in1, const float in2[3][4], vec3_t out);

/**
*	Rotates a vector with the given matrix.
*/
void VectorRotate( const glm::vec3& vector, const glm::mat3x4& matrix, glm::vec3& outResult );

/**
*	Rotates a vector by the inverse of the given matrix.
*/
void VectorIRotate( const glm::vec3& vector, const glm::mat3x4& matrix, glm::vec3& outResult );

void VectorTransform (const vec3_t in1, const float in2[3][4], vec3_t out);

void AngleQuaternion( const vec3_t angles, vec4_t quaternion );
void QuaternionMatrix( const vec4_t quaternion, float (*matrix)[4] );
void QuaternionSlerp( const vec4_t p, vec4_t q, float t, vec4_t qt );

#endif
