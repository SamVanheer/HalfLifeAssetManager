#include <QOpenGLFunctions_1_1>

#include <glm/gtc/type_ptr.hpp>

#include "entity/HLMVStudioModelEntity.hpp"
#include "entity/TextureEntity.hpp"

#include "graphics/SceneContext.hpp"

#include "plugins/halflife/studiomodel/StudioModelAsset.hpp"

void TextureEntity::Draw(graphics::SceneContext& sc, RenderPasses renderPass)
{
	const auto model = GetContext()->Asset->GetEntity()->GetEditableModel();

	assert(model);

	if (_textureIndex == -1)
	{
		return;
	}

	sc.OpenGLFunctions->glMatrixMode(GL_PROJECTION);
	sc.OpenGLFunctions->glLoadIdentity();

	sc.OpenGLFunctions->glOrtho(0.0f, (float)sc.WindowWidth, (float)sc.WindowHeight, 0.0f, 1.0f, -1.0f);

	const auto& texture = *model->Textures[_textureIndex];

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
		sc.OpenGLFunctions->glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		sc.OpenGLFunctions->glDisable(GL_TEXTURE_2D);

		sc.OpenGLFunctions->glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

		if (AntiAliasLines)
		{
			sc.OpenGLFunctions->glEnable(GL_BLEND);
			sc.OpenGLFunctions->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			sc.OpenGLFunctions->glEnable(GL_LINE_SMOOTH);
		}
		else
		{
			sc.OpenGLFunctions->glDisable(GL_BLEND);
		}

		int i;

		for (const auto mesh : _meshes)
		{
			auto triCommands = mesh->Triangles.data();

			while (i = *(triCommands++))
			{
				if (i < 0)
				{
					sc.OpenGLFunctions->glBegin(GL_TRIANGLE_FAN);
					i = -i;
				}
				else
				{
					sc.OpenGLFunctions->glBegin(GL_TRIANGLE_STRIP);
				}

				for (; i > 0; i--, triCommands += 4)
				{
					// FIX: put these in as integer coords, not floats
					sc.OpenGLFunctions->glVertex2f(x + triCommands[2] * TextureScale, y + triCommands[3] * TextureScale);
				}
				sc.OpenGLFunctions->glEnd();
			}
		}

		if (AntiAliasLines)
		{
			sc.OpenGLFunctions->glDisable(GL_LINE_SMOOTH);
		}
	}

	sc.OpenGLFunctions->glPopMatrix();

	sc.OpenGLFunctions->glClear(GL_DEPTH_BUFFER_BIT);
}

void TextureEntity::SetTextureIndex(int textureIndex, int meshIndex)
{
	const auto model = GetContext()->Asset->GetEntity()->GetEditableModel();

	if (textureIndex < 0 || textureIndex >= model->Textures.size())
	{
		textureIndex = -1;
	}

	_textureIndex = textureIndex;

	SetMeshIndex(meshIndex);
}

void TextureEntity::SetMeshIndex(int meshIndex)
{
	if (_textureIndex == -1)
	{
		_meshes.clear();
		return;
	}

	const auto model = GetContext()->Asset->GetEntity()->GetEditableModel();

	_meshes = model->ComputeMeshList(_textureIndex);

	if (meshIndex != -1)
	{
		auto singleMesh = _meshes[meshIndex];
		_meshes.clear();
		_meshes.emplace_back(singleMesh);
	}
}
