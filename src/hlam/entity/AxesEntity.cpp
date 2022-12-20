#include "entity/AxesEntity.hpp"

#include "graphics/SceneContext.hpp"

void AxesEntity::Draw(graphics::SceneContext& sc, RenderPasses renderPass)
{
	if (ShowAxes)
	{
		sc.OpenGLFunctions->glDisable(GL_TEXTURE_2D);
		sc.OpenGLFunctions->glEnable(GL_DEPTH_TEST);

		const float flLength = 50.0f;

		sc.OpenGLFunctions->glLineWidth(1.0f);

		sc.OpenGLFunctions->glBegin(GL_LINES);

		sc.OpenGLFunctions->glColor3f(1.0f, 0, 0);

		sc.OpenGLFunctions->glVertex3f(0, 0, 0);
		sc.OpenGLFunctions->glVertex3f(flLength, 0, 0);

		sc.OpenGLFunctions->glColor3f(0, 1, 0);

		sc.OpenGLFunctions->glVertex3f(0, 0, 0);
		sc.OpenGLFunctions->glVertex3f(0, flLength, 0);

		sc.OpenGLFunctions->glColor3f(0, 0, 1.0f);

		sc.OpenGLFunctions->glVertex3f(0, 0, 0);
		sc.OpenGLFunctions->glVertex3f(0, 0, flLength);

		sc.OpenGLFunctions->glEnd();
	}
}
