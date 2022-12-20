#pragma once

#include <glm/vec3.hpp>

namespace graphics
{
struct Light
{
	glm::vec3 Direction{0, 0, -1};
	glm::vec3 Color{1};
};
}
