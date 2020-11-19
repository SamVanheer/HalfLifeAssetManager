#pragma once

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

	void Draw();

private:
};
}
