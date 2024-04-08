#include <array>

#include <QOpenGLFunctions_1_1>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "entity/CrosshairEntity.hpp"
#include "entity/GuidelinesEntity.hpp"

#include "graphics/GraphicsUtils.hpp"
#include "graphics/SceneContext.hpp"

#include "utility/mathlib.hpp"

#include "settings/ApplicationSettings.hpp"
#include "settings/ColorSettings.hpp"

#include "plugins/halflife/studiomodel/StudioModelColors.hpp"

void GuidelinesEntity::Draw(graphics::SceneContext& sc, RenderPasses renderPass)
{
	const auto aspectRatio = GetContext()->AppSettings->GetAspectRatio();
	const float adjustedWidth = sc.WindowHeight * (aspectRatio.x / aspectRatio.y);
	const float adjustedHeight = sc.WindowWidth * (aspectRatio.y / aspectRatio.x);

	if (ShowGuidelines)
	{
		const int centerX = sc.WindowWidth / 2;
		const int centerY = sc.WindowHeight / 2;

		sc.OpenGLFunctions->glDisable(GL_CULL_FACE);

		sc.OpenGLFunctions->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		sc.OpenGLFunctions->glDisable(GL_TEXTURE_2D);

		auto colors = GetContext()->AppSettings->GetColorSettings();

		sc.OpenGLFunctions->glColor4fv(glm::value_ptr(colors->GetColor(studiomodel::CrosshairColor)));

		sc.OpenGLFunctions->glPointSize(GUIDELINES_LINE_WIDTH);
		sc.OpenGLFunctions->glLineWidth(GUIDELINES_LINE_WIDTH);

		sc.OpenGLFunctions->glBegin(GL_POINTS);

		for (int yPos = sc.WindowHeight - GUIDELINES_LINE_LENGTH;
			yPos >= centerY + CrosshairEntity::CROSSHAIR_LINE_END;
			yPos -= GUIDELINES_OFFSET)
		{
			sc.OpenGLFunctions->glVertex2f(centerX - GUIDELINES_LINE_WIDTH, yPos);
		}

		sc.OpenGLFunctions->glEnd();

		sc.OpenGLFunctions->glBegin(GL_LINES);

		for (int yPos = sc.WindowHeight - GUIDELINES_LINE_LENGTH - GUIDELINES_POINT_LINE_OFFSET - GUIDELINES_LINE_WIDTH;
			yPos >= centerY + CrosshairEntity::CROSSHAIR_LINE_END + GUIDELINES_LINE_LENGTH;
			yPos -= GUIDELINES_OFFSET)
		{
			sc.OpenGLFunctions->glVertex2f(centerX, yPos);
			sc.OpenGLFunctions->glVertex2f(centerX, yPos - GUIDELINES_LINE_LENGTH);
		}

		sc.OpenGLFunctions->glEnd();

		sc.OpenGLFunctions->glLineWidth(GUIDELINES_EDGE_WIDTH);

		sc.OpenGLFunctions->glBegin(GL_LINES);

		if (adjustedWidth <= sc.WindowWidth)
		{
			sc.OpenGLFunctions->glVertex2f((sc.WindowWidth / 2.) - (adjustedWidth / 2), 0);
			sc.OpenGLFunctions->glVertex2f((sc.WindowWidth / 2.) - (adjustedWidth / 2), sc.WindowHeight);

			sc.OpenGLFunctions->glVertex2f((sc.WindowWidth / 2.) + (adjustedWidth / 2), 0);
			sc.OpenGLFunctions->glVertex2f((sc.WindowWidth / 2.) + (adjustedWidth / 2), sc.WindowHeight);
		}
		else
		{
			sc.OpenGLFunctions->glVertex2f(0, (sc.WindowHeight / 2.) - (adjustedHeight / 2));
			sc.OpenGLFunctions->glVertex2f(sc.WindowWidth, (sc.WindowHeight / 2.) - (adjustedHeight / 2));

			sc.OpenGLFunctions->glVertex2f(0, (sc.WindowHeight / 2.) + (adjustedHeight / 2));
			sc.OpenGLFunctions->glVertex2f(sc.WindowWidth, (sc.WindowHeight / 2.) + (adjustedHeight / 2));
		}

		sc.OpenGLFunctions->glEnd();

		sc.OpenGLFunctions->glPointSize(1);
		sc.OpenGLFunctions->glLineWidth(1);
	}

	if (!ShowOffscreenAreas)
	{
		sc.OpenGLFunctions->glDisable(GL_DEPTH_TEST);
		sc.OpenGLFunctions->glColor4f(0, 0, 0, 1);

		Rect rectangles[2]{};

		rectangles[0].Left = 0;
		rectangles[0].Top = 0;
		rectangles[1].Right = sc.WindowWidth;
		rectangles[1].Bottom = sc.WindowHeight;

		if (adjustedWidth <= sc.WindowWidth)
		{
			rectangles[0].Right = (sc.WindowWidth - adjustedWidth) / 2;
			rectangles[0].Bottom = sc.WindowHeight;
			rectangles[1].Left = sc.WindowWidth - rectangles[0].Right;
			rectangles[1].Top = 0;
		}
		else
		{
			rectangles[0].Right = sc.WindowWidth;
			rectangles[0].Bottom = (sc.WindowHeight - adjustedHeight) / 2;
			rectangles[1].Left = 0;
			rectangles[1].Top = sc.WindowHeight - rectangles[0].Bottom;
		}

		for (const auto& rect : rectangles)
		{
			sc.OpenGLFunctions->glBegin(GL_TRIANGLES);
			sc.OpenGLFunctions->glVertex2f(rect.Left, rect.Top);
			sc.OpenGLFunctions->glVertex2f(rect.Right, rect.Top);
			sc.OpenGLFunctions->glVertex2f(rect.Right, rect.Bottom);
			sc.OpenGLFunctions->glVertex2f(rect.Right, rect.Bottom);
			sc.OpenGLFunctions->glVertex2f(rect.Left, rect.Bottom);
			sc.OpenGLFunctions->glVertex2f(rect.Left, rect.Top);
			sc.OpenGLFunctions->glEnd();
		}
	}
}
