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
#include <glm/gtc/quaternion.hpp>

template<typename T>
constexpr T PI{3.14159265358979323846};

// Use this definition globally
template<typename T>
constexpr T ON_EPSILON{0.01};

template<typename T>
constexpr T EQUAL_EPSILON{0.001};

enum class Axis
{
	X,
	Y,
	Z
};

/**
*	Compares the given vectors.
*	@param lhs
*	@param rhs
*	@return true if the vectors are equal, to within EQUAL_EPSILON difference for all components, false otherwise.
*/
bool VectorCompare(const glm::vec3& lhs, const glm::vec3& rhs);

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
