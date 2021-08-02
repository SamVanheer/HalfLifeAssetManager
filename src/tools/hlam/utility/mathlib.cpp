/***
*
*	Copyright (c) 1998, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
****/

// mathlib.cpp -- math primitives

#pragma warning( disable : 4244 )

#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

#include "utility/mathlib.hpp"

constexpr double MaxAngle = 360.0;

bool VectorCompare(const glm::vec3& lhs, const glm::vec3& rhs)
{
	for (size_t i = 0; i < 3; ++i)
	{
		if (fabs(lhs[i] - rhs[i]) > EQUAL_EPSILON<double>)
		{
			return false;
		}
	}

	return true;
}

glm::vec3 VectorToAngles(const glm::vec3& vec)
{
	float yaw, pitch;

	if (vec[1] == 0 && vec[0] == 0)
	{
		yaw = 0;

		if (vec[2] > 0)
		{
			pitch = 90.0f;
		}
		else
		{
			pitch = 270.0f;
		}
	}
	else
	{
		yaw = (atan2(vec[1], vec[0]) * 180 / PI<double>);
		if (yaw < 0)
		{
			yaw += 360;
		}

		const float tmp = sqrt(vec[0] * vec[0] + vec[1] * vec[1]);
		pitch = (atan2(vec[2], tmp) * 180 / PI<double>);

		if (pitch < 0)
		{
			pitch += 360;
		}
	}

	return glm::vec3(pitch, yaw, 0);
}

void SinCos(float flRadians, float& flSin, float& flCos)
{
	flSin = sin(flRadians);
	flCos = cos(flRadians);
}

void AngleVectors(const glm::vec3& vecAngles, glm::vec3* pForward, glm::vec3* pRight, glm::vec3* pUp)
{
	float	sr, sp, sy, cr, cp, cy;

	SinCos(glm::radians(vecAngles[1]), sy, cy);
	SinCos(glm::radians(vecAngles[0]), sp, cp);
	SinCos(glm::radians(vecAngles[2]), sr, cr);

	if (pForward)
	{
		(*pForward)[0] = cp * cy;
		(*pForward)[1] = cp * sy;
		(*pForward)[2] = -sp;
	}

	if (pRight)
	{
		(*pRight)[0] = (-1.0f * sr * sp * cy + -1.0f * cr * -sy);
		(*pRight)[1] = (-1.0f * sr * sp * sy + -1.0f * cr * cy);
		(*pRight)[2] = (-1.0f * sr * cp);
	}

	if (pUp)
	{
		(*pUp)[0] = (cr * sp * cy + -sr * -sy);
		(*pUp)[1] = (cr * sp * sy + -sr * cy);
		(*pUp)[2] = (cr * cp);
	}
}

glm::vec3 AnglesToVector(const glm::vec3& angles)
{
	glm::vec3 forward;

	AngleVectors(angles, &forward, nullptr, nullptr);

	return forward;
}

glm::vec3 AnglesToAimVector(glm::vec3 angles)
{
	angles.x = -angles.x;

	return AnglesToVector(angles);
}

double FixAngle(double angle)
{
	//Use high precision when fixing angles to reduce errors
	angle = std::fmod(angle, MaxAngle);

	while (angle < 0)
	{
		angle += MaxAngle;
	}

	return static_cast<float>(angle);
}

glm::vec3 FixAngles(const glm::vec3& angles)
{
	return {FixAngle(angles.x), FixAngle(angles.y), FixAngle(angles.z)};
}
