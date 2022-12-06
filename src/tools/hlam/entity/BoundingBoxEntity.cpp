#include "entity/BoundingBoxEntity.hpp"
#include "entity/HLMVStudioModelEntity.hpp"

#include "graphics/GraphicsUtils.hpp"
#include "graphics/Scene.hpp"

#include "ui/settings/StudioModelSettings.hpp"

void BoundingBoxEntity::Draw(QOpenGLFunctions_1_1* openglFunctions, RenderPasses renderPass)
{
	if (GetContext()->Settings->ShowBBox)
	{
		if (auto entity = GetContext()->Scene->GetEntity(); entity)
		{
			//Draw a transparent brownish box to display the bounding box
			auto model = entity->GetEditableModel();

			const auto v = graphics::CreateBoxFromBounds(model->BoundingMin, model->BoundingMax);

			graphics::DrawOutlinedBox(openglFunctions, v, {1.0f, 1.0f, 0.0f, 0.5f}, {0.5f, 0.5f, 0.0f, 1.0f});
		}
	}
}
