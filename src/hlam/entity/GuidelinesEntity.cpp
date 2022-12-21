#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "entity/CrosshairEntity.hpp"
#include "entity/GuidelinesEntity.hpp"

#include "graphics/GraphicsUtils.hpp"
#include "graphics/SceneContext.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelColors.hpp"
#include "ui/EditorContext.hpp"
#include "ui/settings/ColorSettings.hpp"
#include "ui/settings/GeneralSettings.hpp"

constexpr float GetAspectRatio(GuidelinesAspectRatio aspectRatio)
{
	switch (aspectRatio)
	{
	case GuidelinesAspectRatio::FourThree: return 4.f / 3.f;
	default: return 16.f / 9.f;
	case GuidelinesAspectRatio::SixteenTen: return 16.f / 10.f;
	}
}

void GuidelinesEntity::Draw(graphics::SceneContext& sc, RenderPasses renderPass)
{
	if (ShowGuidelines)
	{
		const int centerX = sc.WindowWidth / 2;
		const int centerY = sc.WindowHeight / 2;

		sc.OpenGLFunctions->glMatrixMode(GL_PROJECTION);
		sc.OpenGLFunctions->glLoadIdentity();

		sc.OpenGLFunctions->glOrtho(0.0f, (float)sc.WindowWidth, (float)sc.WindowHeight, 0.0f, 1.0f, -1.0f);

		sc.OpenGLFunctions->glMatrixMode(GL_MODELVIEW);
		sc.OpenGLFunctions->glPushMatrix();
		sc.OpenGLFunctions->glLoadIdentity();

		sc.OpenGLFunctions->glDisable(GL_CULL_FACE);

		sc.OpenGLFunctions->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		sc.OpenGLFunctions->glDisable(GL_TEXTURE_2D);

		auto colors = GetContext()->Asset->GetEditorContext()->GetColorSettings();

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

		const float flWidth = sc.WindowHeight * GetAspectRatio(GetContext()->GeneralSettings->GuidelinesAspectRatio);

		sc.OpenGLFunctions->glLineWidth(GUIDELINES_EDGE_WIDTH);

		sc.OpenGLFunctions->glBegin(GL_LINES);

		sc.OpenGLFunctions->glVertex2f((sc.WindowWidth / 2.) - (flWidth / 2), 0);
		sc.OpenGLFunctions->glVertex2f((sc.WindowWidth / 2.) - (flWidth / 2), sc.WindowHeight);

		sc.OpenGLFunctions->glVertex2f((sc.WindowWidth / 2.) + (flWidth / 2), 0);
		sc.OpenGLFunctions->glVertex2f((sc.WindowWidth / 2.) + (flWidth / 2), sc.WindowHeight);

		sc.OpenGLFunctions->glEnd();

		sc.OpenGLFunctions->glPointSize(1);
		sc.OpenGLFunctions->glLineWidth(1);

		sc.OpenGLFunctions->glPopMatrix();
	}
}
