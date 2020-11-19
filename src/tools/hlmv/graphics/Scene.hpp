#pragma once

#include <glm/vec3.hpp>

namespace graphics
{
/**
*	@brief Contains all entities to be rendered for a particular scene
*/
class Scene
{
public:
	Scene() = default;
	~Scene() = default;
	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;

	glm::vec3 GetBackgroundColor() const { return _backgroundColor; }

	void SetBackgroundColor(const glm::vec3& value)
	{
		_backgroundColor = value;
	}

	void Draw();

private:
	glm::vec3 _backgroundColor{0.5f, 0.5f, 0.5f};
};
}
