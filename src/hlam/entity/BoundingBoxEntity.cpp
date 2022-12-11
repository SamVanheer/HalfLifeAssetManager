#include "entity/BoundingBoxEntity.hpp"
#include "entity/HLMVStudioModelEntity.hpp"

#include "graphics/GraphicsUtils.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/settings/StudioModelSettings.hpp"

void BoundingBoxEntity::Draw(QOpenGLFunctions_1_1* openglFunctions, graphics::SceneContext& sc, RenderPasses renderPass)
{
	if (GetContext()->Settings->ShowBBox)
	{
		if (auto entity = GetContext()->Asset->GetEntity(); entity)
		{
			//Draw a transparent brownish box to display the bounding box
			auto model = entity->GetEditableModel();

			const auto v = graphics::CreateBoxFromBounds(model->BoundingMin, model->BoundingMax);

			graphics::DrawOutlinedBox(openglFunctions, v, {1.0f, 1.0f, 0.0f, 0.5f}, {0.5f, 0.5f, 0.0f, 1.0f});
		}
	}
}
