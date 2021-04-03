/***
*
*	Copyright (c) 1998, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
****/

#pragma once

#include <algorithm>
#include <cmath>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat3x4.hpp>
#include <glm/mat4x4.hpp>

template<typename T>
constexpr T PI{3.14159265358979323846};

// Use this definition globally
template<typename T>
constexpr T ON_EPSILON{0.01};

template<typename T>
constexpr T EQUAL_EPSILON{0.001};

/**
*	Compares the given vectors.
*	@param lhs
*	@param rhs
*	@return true if the vectors are equal, to within EQUAL_EPSILON difference for all components, false otherwise.
*/
bool VectorCompare(const glm::vec3& lhs, const glm::vec3& rhs);

/**
*	Normalizes a vector in place and returns the old length.
*/
double VectorNormalize(glm::vec3& vector);

/**
*	Computes the average of the given vector's components.
*	@param vec Vector whose components to average.
*	@return Average.
*/
inline constexpr double VectorAvg(const glm::vec3& vec)
{
	return (vec.x + vec.y + vec.z) / 3;
}

/**
*	Returns the largest value out of all 3 vector components.
*/
inline double VectorMax(const glm::vec3& vec)
{
	return std::max(vec[0], std::max(vec[1], vec[2]));
}

/**
*	Performs the operation vc = va + scale * vb
*	@param va
*	@param scale
*	@param vb
*	@param vc Result.
*/
void VectorMA(const glm::vec3& va, double scale, const glm::vec3& vb, glm::vec3& vc);

/**
*	Converts an angle to a matrix representing the operation that rotates a vector by the angle.
*/
void AngleMatrix(const glm::vec3& angles, glm::mat3x4& matrix);

/**
*	Converts an angle to a matrix representing the operation that rotates a vector by the inverse of the angle.
*/
void AngleIMatrix(const glm::vec3& angles, glm::mat3x4& matrix);

/**
*	Concatenates the given matrices and stores the result in out.
*/
void R_ConcatTransforms(const glm::mat3x4& in1, const glm::mat3x4& in2, glm::mat3x4& out);

/**
*	Rotates a vector with the given matrix.
*/
void VectorRotate(const glm::vec3& vector, const glm::mat3x4& matrix, glm::vec3& outResult);

/**
*	Rotates a vector by the inverse of the given matrix.
*/
void VectorIRotate(const glm::vec3& vector, const glm::mat3x4& matrix, glm::vec3& outResult);

/**
*	Performs the operation in2 * in1 and stores the result in out.
*/
void VectorTransform(const glm::vec3& in1, const glm::mat3x4& in2, glm::vec3& out);

/**
*	Converts an angle into a quaternion.
*/
void AngleQuaternion(const glm::vec3& angles, glm::vec4& quaternion);

/**
*	Converts a quaternion to a matrix.
*/
void QuaternionMatrix(const glm::vec4& quaternion, glm::mat3x4& matrix);

/**
* performs slerp using the quaternion p.
*/
void QuaternionSlerp(const glm::vec4& p, const glm::vec4& q, float t, glm::vec4& qt);

/**
*	Converts a vector to angles.
*	@param vec Vector.
*	@return Angles.
*/
glm::vec3 VectorToAngles(const glm::vec3& vec);

void AngleVectors(const glm::vec3& vecAngles, glm::vec3* pForward, glm::vec3* pRight, glm::vec3* pUp);

/**
*	Converts angles to a vector.
*	@param angles Angles.
*	@return Vector.
*/
glm::vec3 AnglesToVector(const glm::vec3& angles);

glm::vec3 AnglesToAimVector(glm::vec3 angles);

double FixAngle(double angle);

glm::vec3 FixAngles(const glm::vec3& angles);
