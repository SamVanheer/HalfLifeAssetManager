#include <GL/glew.h>

#include "graphics/Scene.hpp"

namespace graphics
{
void Scene::Draw()
{
	glClearColor(_backgroundColor.r, _backgroundColor.g, _backgroundColor.b, 1.0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
}
