#include "entity/HLMVStudioModelEntity.hpp"
#include "entity/TextureEntity.hpp"

#include "graphics/SceneContext.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"

void TextureEntity::Draw(QOpenGLFunctions_1_1* openglFunctions, graphics::SceneContext& sc, RenderPasses renderPass)
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
		openglFunctions->glBindTexture(GL_TEXTURE_2D, _uvMeshTexture);

		openglFunctions->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
			_uvMeshImage.GetWidth(), _uvMeshImage.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, _uvMeshImage.GetData().data());

		//Nearest filtering causes gaps in lines, linear does not
		openglFunctions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		openglFunctions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//Prevent the texture from wrapping and spilling over on the other side
		openglFunctions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		openglFunctions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		_uvMeshImage = {};
	}

	openglFunctions->glMatrixMode(GL_PROJECTION);
	openglFunctions->glLoadIdentity();

	openglFunctions->glOrtho(0.0f, (float)sc.WindowWidth, (float)sc.WindowHeight, 0.0f, 1.0f, -1.0f);

	const auto& texture = *model->Textures[TextureIndex];

	const float w = texture.Data.Width * TextureScale;
	const float h = texture.Data.Height * TextureScale;

	openglFunctions->glMatrixMode(GL_MODELVIEW);
	openglFunctions->glPushMatrix();
	openglFunctions->glLoadIdentity();

	openglFunctions->glDisable(GL_CULL_FACE);
	openglFunctions->glDisable(GL_BLEND);

	openglFunctions->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	const float x = ((static_cast<float>(sc.WindowWidth) - w) / 2) + XOffset;
	const float y = ((static_cast<float>(sc.WindowHeight) - h) / 2) + YOffset;

	openglFunctions->glDisable(GL_DEPTH_TEST);

	if (ShowUVMap && !OverlayUVMap)
	{
		openglFunctions->glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
		openglFunctions->glDisable(GL_TEXTURE_2D);
		openglFunctions->glRectf(x, y, x + w, y + h);
	}

	if (!ShowUVMap || OverlayUVMap)
	{
		if (texture.Flags & STUDIO_NF_MASKED)
		{
			openglFunctions->glEnable(GL_ALPHA_TEST);
			openglFunctions->glAlphaFunc(GL_GREATER, 0.5f);
		}

		openglFunctions->glEnable(GL_TEXTURE_2D);
		openglFunctions->glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		openglFunctions->glBindTexture(GL_TEXTURE_2D, texture.TextureId);

		openglFunctions->glBegin(GL_TRIANGLE_STRIP);

		openglFunctions->glTexCoord2f(0, 0);
		openglFunctions->glVertex2f(x, y);

		openglFunctions->glTexCoord2f(1, 0);
		openglFunctions->glVertex2f(x + w, y);

		openglFunctions->glTexCoord2f(0, 1);
		openglFunctions->glVertex2f(x, y + h);

		openglFunctions->glTexCoord2f(1, 1);
		openglFunctions->glVertex2f(x + w, y + h);

		openglFunctions->glEnd();

		openglFunctions->glBindTexture(GL_TEXTURE_2D, 0);

		if (texture.Flags & STUDIO_NF_MASKED)
		{
			openglFunctions->glDisable(GL_ALPHA_TEST);
		}
	}

	if (ShowUVMap)
	{
		openglFunctions->glEnable(GL_ALPHA_TEST);
		openglFunctions->glAlphaFunc(GL_GREATER, 0.1f);

		openglFunctions->glEnable(GL_TEXTURE_2D);

		openglFunctions->glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		openglFunctions->glBindTexture(GL_TEXTURE_2D, _uvMeshTexture);

		openglFunctions->glBegin(GL_TRIANGLE_STRIP);

		openglFunctions->glTexCoord2f(0, 0);
		openglFunctions->glVertex2f(x, y);

		openglFunctions->glTexCoord2f(1, 0);
		openglFunctions->glVertex2f(x + w, y);

		openglFunctions->glTexCoord2f(0, 1);
		openglFunctions->glVertex2f(x, y + h);

		openglFunctions->glTexCoord2f(1, 1);
		openglFunctions->glVertex2f(x + w, y + h);

		openglFunctions->glEnd();

		openglFunctions->glBindTexture(GL_TEXTURE_2D, 0);
		openglFunctions->glDisable(GL_ALPHA_TEST);
	}

	openglFunctions->glPopMatrix();

	openglFunctions->glClear(GL_DEPTH_BUFFER_BIT);
}

void TextureEntity::CreateDeviceObjects(QOpenGLFunctions_1_1* openglFunctions, graphics::TextureLoader& textureLoader)
{
	openglFunctions->glGenTextures(1, &_uvMeshTexture);
}

void TextureEntity::DestroyDeviceObjects(QOpenGLFunctions_1_1* openglFunctions, graphics::TextureLoader& textureLoader)
{
	openglFunctions->glDeleteTextures(1, &_uvMeshTexture);
	_uvMeshTexture = 0;
}
