#include "entity/ClippingBoxEntity.hpp"
#include "entity/HLMVStudioModelEntity.hpp"

#include "graphics/GraphicsUtils.hpp"
#include "graphics/Scene.hpp"

#include "ui/settings/StudioModelSettings.hpp"

void ClippingBoxEntity::Draw(QOpenGLFunctions_1_1* openglFunctions, RenderPasses renderPass)
{
	if (GetContext()->Settings->ShowCBox)
	{
		if (auto entity = GetContext()->Scene->GetEntity(); entity)
		{
			//Draw a transparent orangeish box to display the clipping box
			auto model = entity->GetEditableModel();

			const auto v = graphics::CreateBoxFromBounds(model->ClippingMin, model->ClippingMax);

			graphics::DrawOutlinedBox(openglFunctions, v, {1.0f, 0.5f, 0.0f, 0.5f}, {0.5f, 0.25f, 0.0f, 1.0f});
		}
	}
}
