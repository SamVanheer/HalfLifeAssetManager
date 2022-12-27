#include <algorithm>

#include <spdlog/spdlog.h>

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "formats/studiomodel/EditableStudioModel.hpp"
#include "formats/studiomodel/StudioModelRenderer.hpp"

#include "graphics/GraphicsUtils.hpp"
#include "graphics/OpenGL.hpp"

#include "settings/ColorSettings.hpp"

#include "ui/assets/studiomodel/StudioModelColors.hpp"

#include "utility/mathlib.hpp"

//Double to float conversion
#pragma warning( disable: 4244 )

namespace studiomdl
{
StudioModelRenderer::StudioModelRenderer(const std::shared_ptr<spdlog::logger>& logger, QOpenGLFunctions_1_1* openglFunctions, ColorSettings* colorSettings)
	: _logger(logger)
	, _openglFunctions(openglFunctions)
	, _colorSettings(colorSettings)
{
	// Initialize them now so the colors don't flicker for the first fraction of a second.
	UpdateColors();
}

StudioModelRenderer::~StudioModelRenderer() = default;

void StudioModelRenderer::RunFrame()
{
	// Cache colors once per frame.
	UpdateColors();
}

unsigned int StudioModelRenderer::DrawModel(studiomdl::ModelRenderInfo& renderInfo, const renderer::DrawFlags flags)
{
	_renderInfo = &renderInfo;

	if (_renderInfo->Model)
	{
		_studioModel = _renderInfo->Model;
	}
	else
	{
		SPDLOG_LOGGER_CALL(_logger, spdlog::level::err, "Called with null model!");
		return 0;
	}

	++_modelsDrawnCount; // render data cache cookie

	_openglFunctions->glPushMatrix();

	auto origin = _renderInfo->Origin;

	//TODO: move this out of the renderer
	//The game applies a 1 unit offset to make view models look nicer
	//See https://github.com/ValveSoftware/halflife/blob/c76dd531a79a176eef7cdbca5a80811123afbbe2/cl_dll/view.cpp#L665-L668
	if (flags & renderer::DrawFlag::IS_VIEW_MODEL)
	{
		origin.z -= 1;
	}

	SetupPosition(origin, _renderInfo->Angles);

	SetUpBones();

	SetupLighting();

	unsigned int uiDrawnPolys = 0;

	const bool fixShadowZFighting = (flags & renderer::DrawFlag::FIX_SHADOW_Z_FIGHTING) != 0;

	if (!(flags & renderer::DrawFlag::NODRAW))
	{
		for (int i = 0; i < _studioModel->Bodyparts.size(); i++)
		{
			SetupModel(i);
			if (_renderInfo->Transparency > 0.0f)
			{
				uiDrawnPolys += DrawPoints(false);

				if (flags & renderer::DrawFlag::DRAW_SHADOWS)
				{
					uiDrawnPolys += DrawShadows(fixShadowZFighting, false);
				}
			}
		}
	}

	if (flags & renderer::DrawFlag::WIREFRAME_OVERLAY)
	{
		//TODO: restore render mode after this?
		_openglFunctions->glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		_openglFunctions->glDisable(GL_TEXTURE_2D);
		_openglFunctions->glDisable(GL_CULL_FACE);
		_openglFunctions->glEnable(GL_DEPTH_TEST);

		for (int i = 0; i < _studioModel->Bodyparts.size(); i++)
		{
			SetupModel(i);
			if (_renderInfo->Transparency > 0.0f)
			{
				uiDrawnPolys += DrawPoints(true);

				if (flags & renderer::DrawFlag::DRAW_SHADOWS)
				{
					uiDrawnPolys += DrawShadows(fixShadowZFighting, true);
				}
			}
		}
	}

	// draw bones
	if (flags & renderer::DrawFlag::DRAW_BONES)
	{
		DrawBones();
	}

	if (flags & renderer::DrawFlag::DRAW_ATTACHMENTS)
	{
		DrawAttachments();
	}

	if (flags & renderer::DrawFlag::DRAW_EYE_POSITION)
	{
		DrawEyePosition();
	}

	if (flags & renderer::DrawFlag::DRAW_HITBOXES)
	{
		DrawHitBoxes();
	}

	if (flags & renderer::DrawFlag::DRAW_NORMALS)
	{
		DrawNormals();
	}

	_openglFunctions->glPopMatrix();

	_drawnPolygonsCount += uiDrawnPolys;

	return uiDrawnPolys;
}

void StudioModelRenderer::DrawSingleBone(ModelRenderInfo& renderInfo, const int iBone)
{
	//TODO: rework how stuff is passed in
	auto model = renderInfo.Model;

	if (!model || iBone < 0 || iBone >= model->Bones.size())
		return;

	_renderInfo = &renderInfo;
	_studioModel = model;

	SetupPosition(_renderInfo->Origin, _renderInfo->Angles);

	SetUpBones();

	_openglFunctions->glDisable(GL_TEXTURE_2D);
	_openglFunctions->glDisable(GL_DEPTH_TEST);

	const auto& bone = *model->Bones[iBone];

	const auto& boneTransform = _bonetransform[bone.ArrayIndex];

	if (bone.Parent)
	{
		const auto& parentBone = *bone.Parent;

		const auto& parentBoneTransform = _bonetransform[parentBone.ArrayIndex];

		_openglFunctions->glPointSize(10.0f);
		_openglFunctions->glColor3f(0, 0.7f, 1);
		_openglFunctions->glBegin(GL_LINES);
		_openglFunctions->glVertex3fv(glm::value_ptr(parentBoneTransform[3]));
		_openglFunctions->glVertex3fv(glm::value_ptr(boneTransform[3]));
		_openglFunctions->glEnd();

		_openglFunctions->glColor3f(0, 0, 0.8f);
		_openglFunctions->glBegin(GL_POINTS);
		if (parentBone.Parent)
			_openglFunctions->glVertex3fv(glm::value_ptr(parentBoneTransform[3]));
		_openglFunctions->glVertex3fv(glm::value_ptr(boneTransform[3]));
		_openglFunctions->glEnd();
	}
	else
	{
		// draw parent bone node
		_openglFunctions->glPointSize(10.0f);
		_openglFunctions->glColor3f(0.8f, 0, 0);
		_openglFunctions->glBegin(GL_POINTS);
		_openglFunctions->glVertex3fv(glm::value_ptr(boneTransform[3]));
		_openglFunctions->glEnd();
	}

	_openglFunctions->glPointSize(1.0f);

	_studioModel = nullptr;
	_renderInfo = nullptr;
}

void StudioModelRenderer::DrawSingleAttachment(ModelRenderInfo& renderInfo, const int iAttachment)
{
	//TODO: rework how stuff is passed in
	auto model = renderInfo.Model;

	if (!model || iAttachment < 0 || iAttachment >= model->Attachments.size())
		return;

	_renderInfo = &renderInfo;
	_studioModel = model;

	SetupPosition(_renderInfo->Origin, _renderInfo->Angles);

	SetUpBones();

	_openglFunctions->glDisable(GL_TEXTURE_2D);
	_openglFunctions->glDisable(GL_CULL_FACE);
	_openglFunctions->glDisable(GL_DEPTH_TEST);

	const auto& attachment = *_studioModel->Attachments[iAttachment];

	const auto& attachmentBoneTransform = _bonetransform[attachment.Bone->ArrayIndex];

	glm::vec3 v[4];

	v[0] = attachmentBoneTransform * glm::vec4{attachment.Origin, 1};
	v[1] = attachmentBoneTransform * glm::vec4{attachment.Vectors[0], 1};
	v[2] = attachmentBoneTransform * glm::vec4{attachment.Vectors[1], 1};
	v[3] = attachmentBoneTransform * glm::vec4{attachment.Vectors[2], 1};

	_openglFunctions->glBegin(GL_LINES);
	_openglFunctions->glColor3f(0, 1, 1);
	_openglFunctions->glVertex3fv(glm::value_ptr(v[0]));
	_openglFunctions->glColor3f(1, 1, 1);
	_openglFunctions->glVertex3fv(glm::value_ptr(v[1]));
	_openglFunctions->glColor3f(0, 1, 1);
	_openglFunctions->glVertex3fv(glm::value_ptr(v[0]));
	_openglFunctions->glColor3f(1, 1, 1);
	_openglFunctions->glVertex3fv(glm::value_ptr(v[2]));
	_openglFunctions->glColor3f(0, 1, 1);
	_openglFunctions->glVertex3fv(glm::value_ptr(v[0]));
	_openglFunctions->glColor3f(1, 1, 1);
	_openglFunctions->glVertex3fv(glm::value_ptr(v[3]));
	_openglFunctions->glEnd();

	_openglFunctions->glPointSize(10);
	_openglFunctions->glColor3f(0, 1, 0);
	_openglFunctions->glBegin(GL_POINTS);
	_openglFunctions->glVertex3fv(glm::value_ptr(v[0]));
	_openglFunctions->glEnd();
	_openglFunctions->glPointSize(1);

	_studioModel = nullptr;
	_renderInfo = nullptr;
}

void StudioModelRenderer::DrawSingleHitbox(ModelRenderInfo& renderInfo, const int hitboxIndex)
{
	//TODO: rework how stuff is passed in
	auto model = renderInfo.Model;

	if (!model || hitboxIndex < 0 || hitboxIndex >= model->Hitboxes.size())
		return;

	_renderInfo = &renderInfo;
	_studioModel = model;

	SetupPosition(_renderInfo->Origin, _renderInfo->Angles);

	SetUpBones();

	_openglFunctions->glDisable(GL_TEXTURE_2D);
	_openglFunctions->glDisable(GL_CULL_FACE);
	if (_renderInfo->Transparency < 1.0f)
		_openglFunctions->glDisable(GL_DEPTH_TEST);
	else
		_openglFunctions->glEnable(GL_DEPTH_TEST);

	_openglFunctions->glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	_openglFunctions->glEnable(GL_BLEND);
	_openglFunctions->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	const auto& hitbox = *_studioModel->Hitboxes[hitboxIndex];

	const auto v = graphics::CreateBoxFromBounds(hitbox.Min, hitbox.Max);

	const auto& hitboxBoneTransform = _bonetransform[hitbox.Bone->ArrayIndex];

	std::array<glm::vec3, 8> v2{};

	for (std::size_t i = 0; i < v2.size(); ++i)
	{
		v2[i] = hitboxBoneTransform * glm::vec4{v[i], 1};
	}

	graphics::DrawOutlinedBox(_openglFunctions, v2,
		_colorSettings->GetColor(studiomodel::HitboxFaceColor), _colorSettings->GetColor(studiomodel::HitboxEdgeColor));

	_studioModel = nullptr;
	_renderInfo = nullptr;
}

void StudioModelRenderer::UpdateColors()
{
	_wireframeColor = _colorSettings->GetColor(studiomodel::WireframeColor);
}

void StudioModelRenderer::SetupPosition(const glm::vec3& origin, const glm::vec3& angles)
{
	_openglFunctions->glTranslatef(origin[0], origin[1], origin[2]);

	_openglFunctions->glRotatef(angles[1], 0, 0, 1);
	_openglFunctions->glRotatef(angles[0], 0, 1, 0);
	_openglFunctions->glRotatef(angles[2], 1, 0, 0);
}

void StudioModelRenderer::DrawBones()
{
	_openglFunctions->glDisable(GL_TEXTURE_2D);
	_openglFunctions->glDisable(GL_DEPTH_TEST);

	for (int i = 0; i < _studioModel->Bones.size(); i++)
	{
		const auto& bone = *_studioModel->Bones[i];

		const auto& boneTransform = _bonetransform[i];

		if (bone.Parent)
		{
			const auto& parentBoneTransform = _bonetransform[bone.Parent->ArrayIndex];

			_openglFunctions->glPointSize(3.0f);
			_openglFunctions->glColor3f(1, 0.7f, 0);
			_openglFunctions->glBegin(GL_LINES);
			_openglFunctions->glVertex3fv(glm::value_ptr(parentBoneTransform[3]));
			_openglFunctions->glVertex3fv(glm::value_ptr(boneTransform[3]));
			_openglFunctions->glEnd();

			_openglFunctions->glColor3f(0, 0, 0.8f);
			_openglFunctions->glBegin(GL_POINTS);
			if (bone.Parent->Parent)
				_openglFunctions->glVertex3fv(glm::value_ptr(parentBoneTransform[3]));
			_openglFunctions->glVertex3fv(glm::value_ptr(boneTransform[3]));
			_openglFunctions->glEnd();
		}
		else
		{
			// draw parent bone node
			_openglFunctions->glPointSize(5.0f);
			_openglFunctions->glColor3f(0.8f, 0, 0);
			_openglFunctions->glBegin(GL_POINTS);
			_openglFunctions->glVertex3fv(glm::value_ptr(boneTransform[3]));
			_openglFunctions->glEnd();
		}
	}

	_openglFunctions->glPointSize(1.0f);
}

void StudioModelRenderer::DrawAttachments()
{
	_openglFunctions->glDisable(GL_TEXTURE_2D);
	_openglFunctions->glDisable(GL_CULL_FACE);
	_openglFunctions->glDisable(GL_DEPTH_TEST);

	for (int i = 0; i < _studioModel->Attachments.size(); i++)
	{
		const auto& attachment = *_studioModel->Attachments[i];

		const auto& attachmentBoneTransform = _bonetransform[attachment.Bone->ArrayIndex];

		glm::vec3 v[4];

		v[0] = attachmentBoneTransform * glm::vec4{attachment.Origin, 1};
		v[1] = attachmentBoneTransform * glm::vec4{attachment.Vectors[0], 1};
		v[2] = attachmentBoneTransform * glm::vec4{attachment.Vectors[1], 1};
		v[3] = attachmentBoneTransform * glm::vec4{attachment.Vectors[2], 1};

		_openglFunctions->glBegin(GL_LINES);
		_openglFunctions->glColor3f(1, 0, 0);
		_openglFunctions->glVertex3fv(glm::value_ptr(v[0]));
		_openglFunctions->glColor3f(1, 1, 1);
		_openglFunctions->glVertex3fv(glm::value_ptr(v[1]));
		_openglFunctions->glColor3f(1, 0, 0);
		_openglFunctions->glVertex3fv(glm::value_ptr(v[0]));
		_openglFunctions->glColor3f(1, 1, 1);
		_openglFunctions->glVertex3fv(glm::value_ptr(v[2]));
		_openglFunctions->glColor3f(1, 0, 0);
		_openglFunctions->glVertex3fv(glm::value_ptr(v[0]));
		_openglFunctions->glColor3f(1, 1, 1);
		_openglFunctions->glVertex3fv(glm::value_ptr(v[3]));
		_openglFunctions->glEnd();

		_openglFunctions->glPointSize(5);
		_openglFunctions->glColor3f(0, 1, 0);
		_openglFunctions->glBegin(GL_POINTS);
		_openglFunctions->glVertex3fv(glm::value_ptr(v[0]));
		_openglFunctions->glEnd();
		_openglFunctions->glPointSize(1);
	}
}

void StudioModelRenderer::DrawEyePosition()
{
	_openglFunctions->glDisable(GL_TEXTURE_2D);
	_openglFunctions->glDisable(GL_CULL_FACE);
	_openglFunctions->glDisable(GL_DEPTH_TEST);

	_openglFunctions->glPointSize(7);
	_openglFunctions->glColor3f(1, 0, 1);
	_openglFunctions->glBegin(GL_POINTS);
	_openglFunctions->glVertex3fv(glm::value_ptr(_studioModel->EyePosition));
	_openglFunctions->glEnd();
	_openglFunctions->glPointSize(1);
}

void StudioModelRenderer::DrawHitBoxes()
{
	_openglFunctions->glDisable(GL_TEXTURE_2D);
	_openglFunctions->glDisable(GL_CULL_FACE);
	if (_renderInfo->Transparency < 1.0f)
		_openglFunctions->glDisable(GL_DEPTH_TEST);
	else
		_openglFunctions->glEnable(GL_DEPTH_TEST);

	_openglFunctions->glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	_openglFunctions->glEnable(GL_BLEND);
	_openglFunctions->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	const glm::vec4 faceColor = _colorSettings->GetColor(studiomodel::HitboxFaceColor);
	const glm::vec4 edgeColor = _colorSettings->GetColor(studiomodel::HitboxEdgeColor);

	for (int i = 0; i < _studioModel->Hitboxes.size(); i++)
	{
		const auto& hitbox = *_studioModel->Hitboxes[i];

		const auto v = graphics::CreateBoxFromBounds(hitbox.Min, hitbox.Max);

		const auto& hitboxTransform = _bonetransform[hitbox.Bone->ArrayIndex];

		std::array<glm::vec3, 8> v2{};

		for (std::size_t i = 0; i < v2.size(); ++i)
		{
			v2[i] = hitboxTransform * glm::vec4{v[i], 1};
		}

		graphics::DrawOutlinedBox(_openglFunctions, v2, faceColor, edgeColor);
	}
}

void StudioModelRenderer::DrawNormals()
{
	_openglFunctions->glDisable(GL_TEXTURE_2D);
	_openglFunctions->glEnable(GL_DEPTH_TEST);

	_openglFunctions->glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	_openglFunctions->glBegin(GL_LINES);

	for (int iBodyPart = 0; iBodyPart < _studioModel->Bodyparts.size(); ++iBodyPart)
	{
		SetupModel(iBodyPart);

		for (int i = 0; i < _model->Vertices.size(); i++)
		{
			_xformverts[i] = _bonetransform[_model->Vertices[i].Bone->ArrayIndex] * glm::vec4{_model->Vertices[i].Vertex, 1};
		}

		for (int i = 0; i < _model->Normals.size(); i++)
		{
			auto matrix = _bonetransform[_model->Normals[i].Bone->ArrayIndex];
			matrix[3] = glm::vec4{0, 0, 0, 1};
			_xformnorms[i] = matrix * glm::vec4{_model->Normals[i].Vertex, 1};
		}

		for (int j = 0; j < _model->Meshes.size(); j++)
		{
			const auto& mesh = _model->Meshes[j];
			auto ptricmds = mesh.Triangles.data();

			int i;

			while (i = *(ptricmds++))
			{
				if (i < 0)
				{
					i = -i;
				}

				for (; i > 0; --i, ptricmds += 4)
				{
					const auto& vertex = _xformverts[ptricmds[0]];

					const auto absoluteNormalEnd = vertex + _xformnorms[ptricmds[1]];

					_openglFunctions->glVertex3fv(glm::value_ptr(vertex));
					_openglFunctions->glVertex3fv(glm::value_ptr(absoluteNormalEnd));
				}
			}
		}
	}

	_openglFunctions->glEnd();
}

void StudioModelRenderer::SetUpBones()
{
	_bonetransform = _boneTransformer.SetUpBones(*_studioModel,
		{
			_renderInfo->Sequence,
			_renderInfo->Frame,
			_renderInfo->Scale,
			_renderInfo->Blender,
			_renderInfo->Controller,
			_renderInfo->Mouth
		}).data();
}

void StudioModelRenderer::SetupLighting()
{
	for (int i = 0; i < _studioModel->Bones.size(); i++)
	{
		auto matrix = _bonetransform[i];
		matrix[3] = glm::vec4{0, 0, 0, 1};
		matrix = glm::inverse(matrix);
		_blightvec[i] = matrix * glm::vec4{_skyLight.Direction, 1};
	}
}

void StudioModelRenderer::SetupModel(int bodypart)
{
	if (bodypart > _studioModel->Bodyparts.size())
	{
		// Con_DPrintf ("StudioModelRenderer::SetupModel: no such bodypart %d\n", bodypart);
		bodypart = 0;
	}

	_model = _studioModel->GetModelByBodyPart(_renderInfo->Bodygroup, bodypart);
}

unsigned int StudioModelRenderer::DrawPoints(const bool bWireframe)
{
	unsigned int uiDrawnPolys = 0;

	//TODO: do this earlier
	_renderInfo->Skin = std::clamp(_renderInfo->Skin, 0, static_cast<int>(_studioModel->SkinFamilies.size()));

	for (int i = 0; i < _model->Vertices.size(); i++)
	{
		_xformverts[i] = _bonetransform[_model->Vertices[i].Bone->ArrayIndex] * glm::vec4{_model->Vertices[i].Vertex, 1};
	}

	SortedMesh meshes[MAXSTUDIOMESHES]{};

	//
	// clip and draw all triangles
	//

	auto normals = _model->Normals.data();

	glm::vec3* lv = _lightvalues;
	for (int j = 0; j < _model->Meshes.size(); j++)
	{
		const auto& mesh = _model->Meshes[j];

		const int flags = _studioModel->SkinFamilies[_renderInfo->Skin][mesh.SkinRef]->Flags;

		meshes[j].Mesh = &mesh;
		meshes[j].Flags = flags;

		for (int i = 0; i < mesh.NumNorms; i++, ++lv, ++normals)
		{
			Lighting(*lv, normals->Bone->ArrayIndex, flags, normals->Vertex);

			// FIX: move this check out of the inner loop
			if (flags & STUDIO_NF_CHROME)
			{
				auto& c = _chrome[reinterpret_cast<glm::vec3*>(lv) - _lightvalues];

				Chrome(c, normals->Bone->ArrayIndex, normals->Vertex);
			}
		}
	}

	//Sort meshes by render modes so additive meshes are drawn after solid meshes.
	//Masked meshes are drawn before solid meshes.
	std::stable_sort(meshes, meshes + _model->Meshes.size(), CompareSortedMeshes);

	uiDrawnPolys += DrawMeshes(bWireframe, meshes);

	_openglFunctions->glDepthMask(GL_TRUE);

	return uiDrawnPolys;
}

unsigned int StudioModelRenderer::DrawMeshes(const bool bWireframe, const SortedMesh* pMeshes)
{
	//Set here since it never changes. Much more efficient.
	if (bWireframe)
	{
		_openglFunctions->glColor4fv(glm::value_ptr(glm::vec4{_wireframeColor, _renderInfo->Transparency}));
	}

	unsigned int uiDrawnPolys = 0;

	//Polygons may overlap, so make sure they can blend together.
	_openglFunctions->glDepthFunc(GL_LEQUAL);

	for (int j = 0; j < _model->Meshes.size(); j++)
	{
		const auto& mesh = *pMeshes[j].Mesh;
		auto ptricmds = mesh.Triangles.data();

		const auto& texture = *_studioModel->SkinFamilies[_renderInfo->Skin][mesh.SkinRef];

		const auto s = 1.0 / (float)texture.Data.Width;
		const auto t = 1.0 / (float)texture.Data.Height;

		if (!bWireframe)
		{
			if (texture.Flags & STUDIO_NF_ADDITIVE)
				_openglFunctions->glDepthMask(GL_FALSE);
			else
				_openglFunctions->glDepthMask(GL_TRUE);

			if (texture.Flags & STUDIO_NF_ADDITIVE)
			{
				_openglFunctions->glEnable(GL_BLEND);
				_openglFunctions->glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			}
			else if (_renderInfo->Transparency < 1.0f)
			{
				_openglFunctions->glEnable(GL_BLEND);
				_openglFunctions->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
			else
			{
				_openglFunctions->glDisable(GL_BLEND);
			}

			if (texture.Flags & STUDIO_NF_MASKED)
			{
				_openglFunctions->glEnable(GL_ALPHA_TEST);
				_openglFunctions->glAlphaFunc(GL_GREATER, 0.5f);
			}

			_openglFunctions->glBindTexture(GL_TEXTURE_2D, texture.TextureId);
		}

		int i;

		while (i = *(ptricmds++))
		{
			if (i < 0)
			{
				_openglFunctions->glBegin(GL_TRIANGLE_FAN);
				i = -i;
			}
			else
			{
				_openglFunctions->glBegin(GL_TRIANGLE_STRIP);
			}

			uiDrawnPolys += i - 2;

			for (; i > 0; i--, ptricmds += 4)
			{
				if (!bWireframe)
				{
					if (texture.Flags & STUDIO_NF_CHROME)
					{
						const auto& c = _chrome[ptricmds[1]];

						_openglFunctions->glTexCoord2f(c[0], c[1]);
					}
					else
					{
						_openglFunctions->glTexCoord2f(ptricmds[2] * s, ptricmds[3] * t);
					}

					if (texture.Flags & STUDIO_NF_ADDITIVE)
					{
						_openglFunctions->glColor4f(1.0f, 1.0f, 1.0f, _renderInfo->Transparency);
					}
					else
					{
						const glm::vec3& lightVec = _lightvalues[ptricmds[1]];
						_openglFunctions->glColor4f(lightVec[0], lightVec[1], lightVec[2], _renderInfo->Transparency);
					}
				}

				_openglFunctions->glVertex3fv(glm::value_ptr(_xformverts[ptricmds[0]]));
			}
			_openglFunctions->glEnd();
		}

		if (!bWireframe)
		{
			if (texture.Flags & STUDIO_NF_ADDITIVE)
			{
				_openglFunctions->glDisable(GL_BLEND);
			}

			if (texture.Flags & STUDIO_NF_MASKED)
			{
				_openglFunctions->glDisable(GL_ALPHA_TEST);
			}
		}
	}

	return uiDrawnPolys;
}

unsigned int StudioModelRenderer::DrawShadows(const bool fixZFighting, const bool wireframe)
{
	if (!(_studioModel->Flags & EF_NOSHADELIGHT))
	{
		GLint oldDepthMask;
		_openglFunctions->glGetIntegerv(GL_DEPTH_WRITEMASK, &oldDepthMask);

		if (fixZFighting)
		{
			_openglFunctions->glDepthMask(GL_FALSE);
		}
		else
		{
			_openglFunctions->glDepthMask(GL_TRUE);
		}

		const float r_blend = _renderInfo->Transparency;

		const auto alpha = 0.5 * r_blend;

		const GLboolean texture2DWasEnabled = _openglFunctions->glIsEnabled(GL_TEXTURE_2D);

		_openglFunctions->glDisable(GL_TEXTURE_2D);
		_openglFunctions->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		_openglFunctions->glEnable(GL_BLEND);

		if (wireframe)
		{
			_openglFunctions->glColor4fv(glm::value_ptr(glm::vec4{_wireframeColor, _renderInfo->Transparency}));
		}
		else
		{
			//Render shadows as black
			_openglFunctions->glColor4f(0.f, 0.f, 0.f, alpha);
		}

		_openglFunctions->glDepthFunc(GL_LESS);

		const auto drawnPolys = InternalDrawShadows();

		_openglFunctions->glDepthFunc(GL_LEQUAL);

		if (texture2DWasEnabled)
		{
			_openglFunctions->glEnable(GL_TEXTURE_2D);
		}

		_openglFunctions->glDisable(GL_BLEND);
		_openglFunctions->glColor4f(1.f, 1.f, 1.f, 1.f);

		_openglFunctions->glDepthMask(static_cast<GLboolean>(oldDepthMask));

		return drawnPolys;
	}
	else
	{
		return 0;
	}
}

unsigned int StudioModelRenderer::InternalDrawShadows()
{
	unsigned int drawnPolys = 0;

	//Always at the entity origin
	const auto lightSampleHeight = _renderInfo->Origin.z;
	const auto shadowHeight = lightSampleHeight + 1.0;

	const glm::vec3 shadeVector = -_skyLight.Direction;

	for (int i = 0; i < _model->Meshes.size(); ++i)
	{
		const auto& mesh = _model->Meshes[i];
		drawnPolys += mesh.NumTriangles;

		auto triCmds = mesh.Triangles.data();

		for (int i; (i = *triCmds++) != 0;)
		{
			if (i < 0)
			{
				i = -i;
				_openglFunctions->glBegin(GL_TRIANGLE_FAN);
			}
			else
			{
				_openglFunctions->glBegin(GL_TRIANGLE_STRIP);
			}

			for (; i > 0; --i, triCmds += 4)
			{
				const auto vertex{_xformverts[triCmds[0]]};

				const auto lightDistance = vertex.z - lightSampleHeight;

				glm::vec3 point;

				point.x = vertex.x - shadeVector.x * lightDistance;
				point.y = vertex.y - shadeVector.y * lightDistance;
				point.z = shadowHeight;

				_openglFunctions->glVertex3fv(glm::value_ptr(point));
			}

			_openglFunctions->glEnd();
		}
	}

	return drawnPolys;
}

void StudioModelRenderer::Lighting(glm::vec3& lv, int bone, int flags, const glm::vec3& normal)
{
	const float ambient = std::max(0.1f, (float)_skyLight.Ambient / 255.0f); // to avoid divison by zero
	const float shade = _skyLight.Shade / 255.0f;
	glm::vec3 illum{ambient};

	if (flags & STUDIO_NF_FULLBRIGHT)
	{
		lv = glm::vec3{1, 1, 1};
		return;
	}
	else if (flags & STUDIO_NF_FLATSHADE)
	{
		illum += 0.8f * shade;
	}
	else
	{
		auto lightcos = glm::dot(normal, _blightvec[bone]); // -1 colinear, 1 opposite

		if (lightcos > 1.0f) lightcos = 1;

		illum += shade;

		auto r = _lambert;
		if (r < 1.0f) r = 1.0f;
		lightcos = (lightcos + (r - 1.0f)) / r; // do modified hemispherical lighting

		if (lightcos > 0.0f)
		{
			illum -= lightcos * shade;
		}

		if (illum[0] <= 0) illum[0] = 0;
		if (illum[1] <= 0) illum[1] = 0;
		if (illum[2] <= 0) illum[2] = 0;
	}

	float max = std::max({illum.x, illum.y, illum.z});

	if (max > 1.0f)
		lv = illum * (1.0f / max);
	else lv = illum;

	lv *= _skyLight.Color;
}


void StudioModelRenderer::Chrome(glm::vec2& chrome, int bone, const glm::vec3& normal)
{
	if (_chromeage[bone] != _modelsDrawnCount)
	{
		// calculate vectors from the viewer to the bone. This roughly adjusts for position
		// vector pointing at bone in world reference frame
		auto tmp = _viewerOrigin * -1.0f;

		tmp += glm::vec3{_bonetransform[bone][3]};

		tmp = glm::normalize(tmp);
		// g_chrome t vector in world reference frame
		auto chromeupvec = glm::cross(tmp, _viewerRight);
		chromeupvec = glm::normalize(chromeupvec);
		// g_chrome s vector in world reference frame
		auto chromerightvec = glm::cross(tmp, chromeupvec);
		chromerightvec = glm::normalize(chromerightvec);

		auto matrix = _bonetransform[bone];
		matrix[3] = glm::vec4{0, 0, 0, 1};
		matrix = glm::inverse(matrix);

		_chromeup[bone] = matrix * glm::vec4{-chromeupvec, 1};
		_chromeright[bone] = matrix * glm::vec4{chromerightvec, 1};

		_chromeage[bone] = _modelsDrawnCount;
	}

	// calc s coord
	auto n = glm::dot(normal, _chromeright[bone]);
	chrome[0] = (n + 1.0) * 0.5;

	// calc t coord
	n = glm::dot(normal, _chromeup[bone]);
	chrome[1] = (n + 1.0) * 0.5;
}
}