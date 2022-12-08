#include "entity/BackgroundEntity.hpp"

#include "graphics/GraphicsUtils.hpp"

#include "ui/settings/StudioModelSettings.hpp"

void BackgroundEntity::Draw(QOpenGLFunctions_1_1* openglFunctions, RenderPasses renderPass)
{
	if (GetContext()->Settings->ShowBackground)
	{
		graphics::DrawBackground(openglFunctions, _texture);
	}
}

void BackgroundEntity::CreateDeviceObjects(QOpenGLFunctions_1_1* openglFunctions, graphics::TextureLoader& textureLoader)
{
	openglFunctions->glGenTextures(1, &_texture);
}

void BackgroundEntity::DestroyDeviceObjects(QOpenGLFunctions_1_1* openglFunctions, graphics::TextureLoader& textureLoader)
{
	openglFunctions->glDeleteTextures(1, &_texture);
	_texture = 0;
}

void BackgroundEntity::SetImage(QOpenGLFunctions_1_1* openglFunctions, int width, int height, const std::byte* data)
{
	openglFunctions->glBindTexture(GL_TEXTURE_2D, _texture);

	openglFunctions->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	openglFunctions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	openglFunctions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

