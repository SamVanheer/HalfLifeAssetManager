#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "entity/CrosshairEntity.hpp"
#include "entity/HLMVStudioModelEntity.hpp"

#include "graphics/GraphicsUtils.hpp"
#include "graphics/SceneContext.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelColors.hpp"
#include "ui/EditorContext.hpp"
#include "ui/settings/ColorSettings.hpp"
#include "ui/settings/StudioModelSettings.hpp"

void CrosshairEntity::Draw(QOpenGLFunctions_1_1* openglFunctions, graphics::SceneContext& sc, RenderPasses renderPass)
{
	if (GetContext()->Settings->ShowCrosshair)
	{
		const int centerX = sc.WindowWidth / 2;
		const int centerY = sc.WindowHeight / 2;

		openglFunctions->glMatrixMode(GL_PROJECTION);
		openglFunctions->glLoadIdentity();

		openglFunctions->glOrtho(0.0f, (float)sc.WindowWidth, (float)sc.WindowHeight, 0.0f, 1.0f, -1.0f);

		openglFunctions->glMatrixMode(GL_MODELVIEW);
		openglFunctions->glPushMatrix();
		openglFunctions->glLoadIdentity();

		openglFunctions->glDisable(GL_CULL_FACE);

		openglFunctions->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		openglFunctions->glDisable(GL_TEXTURE_2D);

		auto colors = GetContext()->Asset->GetEditorContext()->GetColorSettings();

		openglFunctions->glColor4fv(glm::value_ptr(glm::vec4{colors->GetColor(studiomodel::CrosshairColor.Name), 1}));

		openglFunctions->glPointSize(CROSSHAIR_LINE_WIDTH);
		openglFunctions->glLineWidth(CROSSHAIR_LINE_WIDTH);

		openglFunctions->glBegin(GL_POINTS);

		openglFunctions->glVertex2f(centerX - CROSSHAIR_LINE_WIDTH / 2, centerY + 1);

		openglFunctions->glEnd();

		openglFunctions->glBegin(GL_LINES);

		openglFunctions->glVertex2f(centerX - CROSSHAIR_LINE_START, centerY);
		openglFunctions->glVertex2f(centerX - CROSSHAIR_LINE_END, centerY);

		openglFunctions->glVertex2f(centerX + CROSSHAIR_LINE_START, centerY);
		openglFunctions->glVertex2f(centerX + CROSSHAIR_LINE_END, centerY);

		openglFunctions->glVertex2f(centerX, centerY - CROSSHAIR_LINE_START);
		openglFunctions->glVertex2f(centerX, centerY - CROSSHAIR_LINE_END);

		openglFunctions->glVertex2f(centerX, centerY + CROSSHAIR_LINE_START);
		openglFunctions->glVertex2f(centerX, centerY + CROSSHAIR_LINE_END);

		openglFunctions->glEnd();

		openglFunctions->glPointSize(1);
		openglFunctions->glLineWidth(1);

		openglFunctions->glPopMatrix();
	}
}
