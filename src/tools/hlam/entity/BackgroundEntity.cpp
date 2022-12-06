#include "entity/BackgroundEntity.hpp"

#include "graphics/GraphicsUtils.hpp"
#include "graphics/Scene.hpp"

#include "ui/settings/StudioModelSettings.hpp"

void BackgroundEntity::Draw(QOpenGLFunctions_1_1* openglFunctions, RenderPasses renderPass)
{
	if (GetContext()->Settings->ShowBackground && GetContext()->Scene->BackgroundTexture != GL_INVALID_TEXTURE_ID)
	{
		graphics::DrawBackground(openglFunctions, GetContext()->Scene->BackgroundTexture);
	}
}
