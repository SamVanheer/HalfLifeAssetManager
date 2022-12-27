#include <QOpenGLFunctions_1_1>

#include "entity/BackgroundEntity.hpp"

#include "graphics/GraphicsUtils.hpp"
#include "graphics/SceneContext.hpp"

void BackgroundEntity::Draw(graphics::SceneContext& sc, RenderPasses renderPass)
{
	if (ShowBackground)
	{
		// Update image if changed.
		if (!_image.GetData().empty())
		{
			sc.OpenGLFunctions->glBindTexture(GL_TEXTURE_2D, _texture);

			sc.OpenGLFunctions->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
				_image.GetWidth(), _image.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, _image.GetData().data());
			sc.OpenGLFunctions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			sc.OpenGLFunctions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			_image = {};
		}

		graphics::DrawBackground(sc.OpenGLFunctions, _texture);
	}
}

void BackgroundEntity::CreateDeviceObjects(graphics::SceneContext& sc)
{
	sc.OpenGLFunctions->glGenTextures(1, &_texture);
}

void BackgroundEntity::DestroyDeviceObjects(graphics::SceneContext& sc)
{
	sc.OpenGLFunctions->glDeleteTextures(1, &_texture);
	_texture = 0;
}
