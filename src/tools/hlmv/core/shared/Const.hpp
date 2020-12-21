#pragma once

#include <limits>

#include <glm/vec3.hpp>

typedef unsigned char byte;

typedef unsigned long color32_t;

/**
*	The world boundary. The furthest away from the world origin you can go in any axis.
*	TODO: this is Sven Co-op's setting. Half-Life defines it to be 4096.
*/
constexpr double WORLD_BOUNDARY{131072};

/**
*	Invalid value for a world min coordinate.
*/
constexpr double WORLD_INVALID_MIN_COORD{static_cast<double>(std::numeric_limits<int>::max())};

/**
*	Invalid value for a world max coordinate.
*/
constexpr double WORLD_INVALID_MAX_COORD{static_cast<double>(std::numeric_limits<int>::min())};

/**
*	@brief World origin.
*/
inline const glm::vec3 vec3_origin{0};

/**
*	@brief Vector that represents an invalid min boundary.
*/
inline const glm::dvec3 WORLD_INVALID_MIN{WORLD_INVALID_MIN_COORD};

/**
*	@brief Vector that represents an invalid max boundary.
*/
inline const glm::dvec3 WORLD_INVALID_MAX{WORLD_INVALID_MAX_COORD};
