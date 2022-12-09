#include "entity/ClippingBoxEntity.hpp"
#include "entity/HLMVStudioModelEntity.hpp"

#include "graphics/GraphicsUtils.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/settings/StudioModelSettings.hpp"

void ClippingBoxEntity::Draw(QOpenGLFunctions_1_1* openglFunctions, graphics::SceneContext& sc, RenderPasses renderPass)
{
	if (GetContext()->Settings->ShowCBox)
	{
		if (auto entity = GetContext()->Asset->GetEntity(); entity)
		{
			//Draw a transparent orangeish box to display the clipping box
			auto model = entity->GetEditableModel();

			const auto v = graphics::CreateBoxFromBounds(model->ClippingMin, model->ClippingMax);

			graphics::DrawOutlinedBox(openglFunctions, v, {1.0f, 0.5f, 0.0f, 0.5f}, {0.5f, 0.25f, 0.0f, 1.0f});
		}
	}
}
