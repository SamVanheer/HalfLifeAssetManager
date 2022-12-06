#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "entity/CrosshairEntity.hpp"
#include "entity/GuidelinesEntity.hpp"

#include "graphics/GraphicsUtils.hpp"
#include "graphics/Scene.hpp"

#include "ui/settings/StudioModelSettings.hpp"

void GuidelinesEntity::Draw(QOpenGLFunctions_1_1* openglFunctions, RenderPasses renderPass)
{
	if (GetContext()->Settings->ShowGuidelines)
	{
		auto scene = GetContext()->Scene;

		const int centerX = scene->GetWindowWidth() / 2;
		const int centerY = scene->GetWindowHeight() / 2;

		openglFunctions->glMatrixMode(GL_PROJECTION);
		openglFunctions->glLoadIdentity();

		openglFunctions->glOrtho(0.0f, (float)scene->GetWindowWidth(), (float)scene->GetWindowHeight(), 0.0f, 1.0f, -1.0f);

		openglFunctions->glMatrixMode(GL_MODELVIEW);
		openglFunctions->glPushMatrix();
		openglFunctions->glLoadIdentity();

		openglFunctions->glDisable(GL_CULL_FACE);

		openglFunctions->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		openglFunctions->glDisable(GL_TEXTURE_2D);

		openglFunctions->glColor4fv(glm::value_ptr(glm::vec4{scene->CrosshairColor, 1}));

		openglFunctions->glPointSize(GUIDELINES_LINE_WIDTH);
		openglFunctions->glLineWidth(GUIDELINES_LINE_WIDTH);

		openglFunctions->glBegin(GL_POINTS);

		for (int yPos = scene->GetWindowHeight() - GUIDELINES_LINE_LENGTH;
			yPos >= centerY + CrosshairEntity::CROSSHAIR_LINE_END;
			yPos -= GUIDELINES_OFFSET)
		{
			openglFunctions->glVertex2f(centerX - GUIDELINES_LINE_WIDTH, yPos);
		}

		openglFunctions->glEnd();

		openglFunctions->glBegin(GL_LINES);

		for (int yPos = scene->GetWindowHeight() - GUIDELINES_LINE_LENGTH - GUIDELINES_POINT_LINE_OFFSET - GUIDELINES_LINE_WIDTH;
			yPos >= centerY + CrosshairEntity::CROSSHAIR_LINE_END + GUIDELINES_LINE_LENGTH;
			yPos -= GUIDELINES_OFFSET)
		{
			openglFunctions->glVertex2f(centerX, yPos);
			openglFunctions->glVertex2f(centerX, yPos - GUIDELINES_LINE_LENGTH);
		}

		openglFunctions->glEnd();

		const float flWidth = scene->GetWindowHeight() * (16 / 9.0);

		openglFunctions->glLineWidth(GUIDELINES_EDGE_WIDTH);

		openglFunctions->glBegin(GL_LINES);

		openglFunctions->glVertex2f((scene->GetWindowWidth() / 2.) - (flWidth / 2), 0);
		openglFunctions->glVertex2f((scene->GetWindowWidth() / 2.) - (flWidth / 2), scene->GetWindowHeight());

		openglFunctions->glVertex2f((scene->GetWindowWidth() / 2.) + (flWidth / 2), 0);
		openglFunctions->glVertex2f((scene->GetWindowWidth() / 2.) + (flWidth / 2), scene->GetWindowHeight());

		openglFunctions->glEnd();

		openglFunctions->glPointSize(1);
		openglFunctions->glLineWidth(1);

		openglFunctions->glPopMatrix();
	}
}
