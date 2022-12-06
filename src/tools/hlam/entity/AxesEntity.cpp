#include "entity/AxesEntity.hpp"

#include "ui/settings/StudioModelSettings.hpp"

void AxesEntity::Draw(QOpenGLFunctions_1_1* openglFunctions, RenderPasses renderPass)
{
	if (GetContext()->Settings->ShowAxes)
	{
		openglFunctions->glDisable(GL_TEXTURE_2D);
		openglFunctions->glEnable(GL_DEPTH_TEST);

		const float flLength = 50.0f;

		openglFunctions->glLineWidth(1.0f);

		openglFunctions->glBegin(GL_LINES);

		openglFunctions->glColor3f(1.0f, 0, 0);

		openglFunctions->glVertex3f(0, 0, 0);
		openglFunctions->glVertex3f(flLength, 0, 0);

		openglFunctions->glColor3f(0, 1, 0);

		openglFunctions->glVertex3f(0, 0, 0);
		openglFunctions->glVertex3f(0, flLength, 0);

		openglFunctions->glColor3f(0, 0, 1.0f);

		openglFunctions->glVertex3f(0, 0, 0);
		openglFunctions->glVertex3f(0, 0, flLength);

		openglFunctions->glEnd();
	}
}
