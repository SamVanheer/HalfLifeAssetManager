#include "entity/BoundingBoxEntity.hpp"
#include "entity/HLMVStudioModelEntity.hpp"

#include "graphics/GraphicsUtils.hpp"
#include "graphics/SceneContext.hpp"

#include "plugins/halflife/studiomodel/StudioModelAsset.hpp"

void BoundingBoxEntity::Draw(graphics::SceneContext& sc, RenderPasses renderPass)
{
	if (ShowBBox)
	{
		if (auto entity = GetContext()->Asset->GetEntity(); entity)
		{
			//Draw a transparent brownish box to display the bounding box
			auto model = entity->GetEditableModel();

			const auto v = graphics::CreateBoxFromBounds(model->BoundingMin, model->BoundingMax);

			graphics::DrawOutlinedBox(sc.OpenGLFunctions, v, {1.0f, 1.0f, 0.0f, 0.5f}, {0.5f, 0.5f, 0.0f, 1.0f});
		}
	}
}
