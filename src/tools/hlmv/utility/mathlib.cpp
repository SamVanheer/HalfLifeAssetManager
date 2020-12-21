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

#include "mathlib.hpp"

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

double VectorNormalize(glm::vec3& vector)
{
	double length = 0;

	for (int i = 0; i < 3; ++i)
	{
		length += vector[i] * vector[i];
	}

	length = sqrt(length);

	if (length == 0)
	{
		return 0;
	}

	for (int i = 0; i < 3; ++i)
	{
		vector[i] /= length;
	}

	return length;
}

void VectorMA(const glm::vec3& va, double scale, const glm::vec3& vb, glm::vec3& vc)
{
	vc[0] = va[0] + scale * vb[0];
	vc[1] = va[1] + scale * vb[1];
	vc[2] = va[2] + scale * vb[2];
}

void AngleMatrix(const glm::vec3& angles, glm::mat3x4& matrix)
{
	float angle = angles[2] * (PI<double>*2 / 360);
	const float sy = sin(angle);
	const float cy = cos(angle);
	angle = angles[1] * (PI<double>*2 / 360);
	const float sp = sin(angle);
	const float cp = cos(angle);
	angle = angles[0] * (PI<double>*2 / 360);
	const float sr = sin(angle);
	const float cr = cos(angle);

	// matrix = (Z * Y) * X
	matrix[0][0] = cp * cy;
	matrix[1][0] = cp * sy;
	matrix[2][0] = -sp;
	matrix[0][1] = sr * sp * cy + cr * -sy;
	matrix[1][1] = sr * sp * sy + cr * cy;
	matrix[2][1] = sr * cp;
	matrix[0][2] = (cr * sp * cy + -sr * -sy);
	matrix[1][2] = (cr * sp * sy + -sr * cy);
	matrix[2][2] = cr * cp;
	matrix[0][3] = 0.0;
	matrix[1][3] = 0.0;
	matrix[2][3] = 0.0;
}

void AngleIMatrix(const glm::vec3& angles, glm::mat3x4& matrix)
{
	float angle = angles[2] * (PI<double>*2 / 360);
	const float sy = sin(angle);
	const float cy = cos(angle);
	angle = angles[1] * (PI<double>*2 / 360);
	const float sp = sin(angle);
	const float cp = cos(angle);
	angle = angles[0] * (PI<double>*2 / 360);
	const float sr = sin(angle);
	const float cr = cos(angle);

	// matrix = (Z * Y) * X
	matrix[0][0] = cp * cy;
	matrix[0][1] = cp * sy;
	matrix[0][2] = -sp;
	matrix[1][0] = sr * sp * cy + cr * -sy;
	matrix[1][1] = sr * sp * sy + cr * cy;
	matrix[1][2] = sr * cp;
	matrix[2][0] = (cr * sp * cy + -sr * -sy);
	matrix[2][1] = (cr * sp * sy + -sr * cy);
	matrix[2][2] = cr * cp;
	matrix[0][3] = 0.0;
	matrix[1][3] = 0.0;
	matrix[2][3] = 0.0;
}

void R_ConcatTransforms(const glm::mat3x4& in1, const glm::mat3x4& in2, glm::mat3x4& out)
{
	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] +
		in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] +
		in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] +
		in1[0][2] * in2[2][2];
	out[0][3] = in1[0][0] * in2[0][3] + in1[0][1] * in2[1][3] +
		in1[0][2] * in2[2][3] + in1[0][3];
	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] +
		in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] +
		in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] +
		in1[1][2] * in2[2][2];
	out[1][3] = in1[1][0] * in2[0][3] + in1[1][1] * in2[1][3] +
		in1[1][2] * in2[2][3] + in1[1][3];
	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] +
		in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] +
		in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] +
		in1[2][2] * in2[2][2];
	out[2][3] = in1[2][0] * in2[0][3] + in1[2][1] * in2[1][3] +
		in1[2][2] * in2[2][3] + in1[2][3];
}

void VectorRotate(const glm::vec3& vector, const glm::mat3x4& matrix, glm::vec3& outResult)
{
	outResult[0] = glm::dot(vector, *reinterpret_cast<const glm::vec3*>(&matrix[0]));
	outResult[1] = glm::dot(vector, *reinterpret_cast<const glm::vec3*>(&matrix[1]));
	outResult[2] = glm::dot(vector, *reinterpret_cast<const glm::vec3*>(&matrix[2]));
}

void VectorIRotate(const glm::vec3& vector, const glm::mat3x4& matrix, glm::vec3& outResult)
{
	outResult[0] = vector[0] * matrix[0][0] + vector[1] * matrix[1][0] + vector[2] * matrix[2][0];
	outResult[1] = vector[0] * matrix[0][1] + vector[1] * matrix[1][1] + vector[2] * matrix[2][1];
	outResult[2] = vector[0] * matrix[0][2] + vector[1] * matrix[1][2] + vector[2] * matrix[2][2];
}

void VectorTransform(const glm::vec3& in1, const glm::mat3x4& in2, glm::vec3& out)
{
	out[0] = glm::dot(in1, *reinterpret_cast<const glm::vec3*>(&in2[0])) + in2[0][3];
	out[1] = glm::dot(in1, *reinterpret_cast<const glm::vec3*>(&in2[1])) + in2[1][3];
	out[2] = glm::dot(in1, *reinterpret_cast<const glm::vec3*>(&in2[2])) + in2[2][3];
}

void AngleQuaternion(const glm::vec3& angles, glm::vec4& quaternion)
{
	// FIXME: rescale the inputs to 1/2 angle
	float angle = angles[2] * 0.5;
	const float sy = sin(angle);
	const float cy = cos(angle);
	angle = angles[1] * 0.5;
	const float sp = sin(angle);
	const float cp = cos(angle);
	angle = angles[0] * 0.5;
	const float sr = sin(angle);
	const float cr = cos(angle);

	quaternion[0] = sr * cp * cy - cr * sp * sy; // X
	quaternion[1] = cr * sp * cy + sr * cp * sy; // Y
	quaternion[2] = cr * cp * sy - sr * sp * cy; // Z
	quaternion[3] = cr * cp * cy + sr * sp * sy; // W
}

void QuaternionMatrix(const glm::vec4& quaternion, glm::mat3x4& matrix)
{
	matrix[0][0] = 1.0 - 2.0 * quaternion[1] * quaternion[1] - 2.0 * quaternion[2] * quaternion[2];
	matrix[1][0] = 2.0 * quaternion[0] * quaternion[1] + 2.0 * quaternion[3] * quaternion[2];
	matrix[2][0] = 2.0 * quaternion[0] * quaternion[2] - 2.0 * quaternion[3] * quaternion[1];

	matrix[0][1] = 2.0 * quaternion[0] * quaternion[1] - 2.0 * quaternion[3] * quaternion[2];
	matrix[1][1] = 1.0 - 2.0 * quaternion[0] * quaternion[0] - 2.0 * quaternion[2] * quaternion[2];
	matrix[2][1] = 2.0 * quaternion[1] * quaternion[2] + 2.0 * quaternion[3] * quaternion[0];

	matrix[0][2] = 2.0 * quaternion[0] * quaternion[2] + 2.0 * quaternion[3] * quaternion[1];
	matrix[1][2] = 2.0 * quaternion[1] * quaternion[2] - 2.0 * quaternion[3] * quaternion[0];
	matrix[2][2] = 1.0 - 2.0 * quaternion[0] * quaternion[0] - 2.0 * quaternion[1] * quaternion[1];
}

void QuaternionSlerp(const glm::vec4& p, glm::vec4& q, float t, glm::vec4& qt)
{
	// decide if one of the quaternions is backwards
	float a = 0;
	float b = 0;

	for (int i = 0; i < 4; ++i)
	{
		a += (p[i] - q[i]) * (p[i] - q[i]);
		b += (p[i] + q[i]) * (p[i] + q[i]);
	}

	if (a > b)
	{
		for (int i = 0; i < 4; ++i)
		{
			q[i] = -q[i];
		}
	}

	const float cosom = p[0] * q[0] + p[1] * q[1] + p[2] * q[2] + p[3] * q[3];

	if ((1.0 + cosom) > 0.00000001)
	{
		float sclp, sclq;

		if ((1.0 - cosom) > 0.00000001)
		{
			const float omega = acos(cosom);
			const float sinom = sin(omega);
			sclp = sin((1.0 - t) * omega) / sinom;
			sclq = sin(t * omega) / sinom;
		}
		else
		{
			sclp = 1.0 - t;
			sclq = t;
		}

		for (int i = 0; i < 4; ++i)
		{
			qt[i] = sclp * p[i] + sclq * q[i];
		}
	}
	else
	{
		qt[0] = -p[1];
		qt[1] = p[0];
		qt[2] = -p[3];
		qt[3] = p[2];
		const float sclp = sin((1.0 - t) * 0.5 * PI<double>);
		const float sclq = sin(t * 0.5 * PI<double>);

		for (int i = 0; i < 3; ++i)
		{
			qt[i] = sclp * p[i] + sclq * qt[i];
		}
	}
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
	glm::vec3 forward, right, up;

	AngleVectors(angles, &forward, &right, &up);

	return forward + right + up;
}

glm::mat4x4 Mat4x4ModelView()
{
	return glm::mat4x4(
		0, -1, 0, 0,
		0, 0, 1, 0,
		-1, 0, 0, 0,
		0, 0, 0, 1);
}
