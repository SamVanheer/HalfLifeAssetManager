#pragma once

#include <cstdint>

#include <glm/vec3.hpp>

namespace graphics
{
struct Light
{
	glm::vec3 Direction{0, 0, -1};
	glm::vec3 Color{1};
	std::uint8_t Ambient = 32; // ambient world light
	std::uint8_t Shade = 192; // direct world light
};
}
