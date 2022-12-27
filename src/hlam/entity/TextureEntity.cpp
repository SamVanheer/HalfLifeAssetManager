#include <QOpenGLFunctions_1_1>

#include "entity/HLMVStudioModelEntity.hpp"
#include "entity/TextureEntity.hpp"

#include "graphics/SceneContext.hpp"

#include "plugins/halflife/studiomodel/StudioModelAsset.hpp"

void TextureEntity::Draw(graphics::SceneContext& sc, RenderPasses renderPass)
{
	const auto model = GetContext()->Asset->GetEntity()->GetEditableModel();

	assert(model);

	if (TextureIndex < 0 || TextureIndex >= model->Textures.size())
	{
		return;
	}

	// Update image if changed.
	if (!_uvMeshImage.GetData().empty())
	{
		sc.OpenGLFunctions->glBindTexture(GL_TEXTURE_2D, _uvMeshTexture);

		sc.OpenGLFunctions->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
			_uvMeshImage.GetWidth(), _uvMeshImage.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, _uvMeshImage.GetData().data());

		//Nearest filtering causes gaps in lines, linear does not
		sc.OpenGLFunctions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		sc.OpenGLFunctions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//Prevent the texture from wrapping and spilling over on the other side
		sc.OpenGLFunctions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		sc.OpenGLFunctions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		_uvMeshImage = {};
	}

	sc.OpenGLFunctions->glMatrixMode(GL_PROJECTION);
	sc.OpenGLFunctions->glLoadIdentity();

	sc.OpenGLFunctions->glOrtho(0.0f, (float)sc.WindowWidth, (float)sc.WindowHeight, 0.0f, 1.0f, -1.0f);

	const auto& texture = *model->Textures[TextureIndex];

	const float w = texture.Data.Width * TextureScale;
	const float h = texture.Data.Height * TextureScale;

	sc.OpenGLFunctions->glMatrixMode(GL_MODELVIEW);
	sc.OpenGLFunctions->glPushMatrix();
	sc.OpenGLFunctions->glLoadIdentity();

	sc.OpenGLFunctions->glDisable(GL_CULL_FACE);
	sc.OpenGLFunctions->glDisable(GL_BLEND);

	sc.OpenGLFunctions->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	const float x = ((static_cast<float>(sc.WindowWidth) - w) / 2) + XOffset;
	const float y = ((static_cast<float>(sc.WindowHeight) - h) / 2) + YOffset;

	sc.OpenGLFunctions->glDisable(GL_DEPTH_TEST);

	if (ShowUVMap && !OverlayUVMap)
	{
		sc.OpenGLFunctions->glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
		sc.OpenGLFunctions->glDisable(GL_TEXTURE_2D);
		sc.OpenGLFunctions->glRectf(x, y, x + w, y + h);
	}

	if (!ShowUVMap || OverlayUVMap)
	{
		if (texture.Flags & STUDIO_NF_MASKED)
		{
			sc.OpenGLFunctions->glEnable(GL_ALPHA_TEST);
			sc.OpenGLFunctions->glAlphaFunc(GL_GREATER, 0.5f);
		}

		sc.OpenGLFunctions->glEnable(GL_TEXTURE_2D);
		sc.OpenGLFunctions->glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		sc.OpenGLFunctions->glBindTexture(GL_TEXTURE_2D, texture.TextureId);

		sc.OpenGLFunctions->glBegin(GL_TRIANGLE_STRIP);

		sc.OpenGLFunctions->glTexCoord2f(0, 0);
		sc.OpenGLFunctions->glVertex2f(x, y);

		sc.OpenGLFunctions->glTexCoord2f(1, 0);
		sc.OpenGLFunctions->glVertex2f(x + w, y);

		sc.OpenGLFunctions->glTexCoord2f(0, 1);
		sc.OpenGLFunctions->glVertex2f(x, y + h);

		sc.OpenGLFunctions->glTexCoord2f(1, 1);
		sc.OpenGLFunctions->glVertex2f(x + w, y + h);

		sc.OpenGLFunctions->glEnd();

		sc.OpenGLFunctions->glBindTexture(GL_TEXTURE_2D, 0);

		if (texture.Flags & STUDIO_NF_MASKED)
		{
			sc.OpenGLFunctions->glDisable(GL_ALPHA_TEST);
		}
	}

	if (ShowUVMap)
	{
		sc.OpenGLFunctions->glEnable(GL_ALPHA_TEST);
		sc.OpenGLFunctions->glAlphaFunc(GL_GREATER, 0.1f);

		sc.OpenGLFunctions->glEnable(GL_TEXTURE_2D);

		sc.OpenGLFunctions->glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		sc.OpenGLFunctions->glBindTexture(GL_TEXTURE_2D, _uvMeshTexture);

		sc.OpenGLFunctions->glBegin(GL_TRIANGLE_STRIP);

		sc.OpenGLFunctions->glTexCoord2f(0, 0);
		sc.OpenGLFunctions->glVertex2f(x, y);

		sc.OpenGLFunctions->glTexCoord2f(1, 0);
		sc.OpenGLFunctions->glVertex2f(x + w, y);

		sc.OpenGLFunctions->glTexCoord2f(0, 1);
		sc.OpenGLFunctions->glVertex2f(x, y + h);

		sc.OpenGLFunctions->glTexCoord2f(1, 1);
		sc.OpenGLFunctions->glVertex2f(x + w, y + h);

		sc.OpenGLFunctions->glEnd();

		sc.OpenGLFunctions->glBindTexture(GL_TEXTURE_2D, 0);
		sc.OpenGLFunctions->glDisable(GL_ALPHA_TEST);
	}

	sc.OpenGLFunctions->glPopMatrix();

	sc.OpenGLFunctions->glClear(GL_DEPTH_BUFFER_BIT);
}

void TextureEntity::CreateDeviceObjects(graphics::SceneContext& sc)
{
	sc.OpenGLFunctions->glGenTextures(1, &_uvMeshTexture);
}

void TextureEntity::DestroyDeviceObjects(graphics::SceneContext& sc)
{
	sc.OpenGLFunctions->glDeleteTextures(1, &_uvMeshTexture);
	_uvMeshTexture = 0;
}
