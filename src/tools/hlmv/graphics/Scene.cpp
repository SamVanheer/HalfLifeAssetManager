#include <GL/glew.h>

#include "graphics/Scene.hpp"

namespace graphics
{
void Scene::Draw()
{
	//TODO: remove: Clear to red as a test
	glClearColor(1.0f, 0.0f, 0.0f, 1.0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
}
