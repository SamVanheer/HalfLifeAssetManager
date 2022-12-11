#include "entity/BackgroundEntity.hpp"

#include "graphics/GraphicsUtils.hpp"

#include "ui/settings/StudioModelSettings.hpp"

void BackgroundEntity::Draw(QOpenGLFunctions_1_1* openglFunctions, graphics::SceneContext& sc, RenderPasses renderPass)
{
	if (GetContext()->Settings->ShowBackground)
	{
		// Update image if changed.
		if (!_image.GetData().empty())
		{
			openglFunctions->glBindTexture(GL_TEXTURE_2D, _texture);

			openglFunctions->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
				_image.GetWidth(), _image.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, _image.GetData().data());
			openglFunctions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			openglFunctions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			_image = {};
		}

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
