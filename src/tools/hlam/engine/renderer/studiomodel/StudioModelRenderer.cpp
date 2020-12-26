#include <glm/gtc/type_ptr.hpp>

#include <algorithm>

#include "core/shared/Logging.hpp"

#include "graphics/GraphicsUtils.hpp"

#include "engine/shared/studiomodel/StudioModel.hpp"

#include "engine/renderer/studiomodel/StudioModelRenderer.hpp"

//Double to float conversion
#pragma warning( disable: 4244 )

namespace studiomdl
{
StudioModelRenderer::StudioModelRenderer() = default;
StudioModelRenderer::~StudioModelRenderer() = default;

bool StudioModelRenderer::Initialize()
{
	_modelsDrawnCount = 0;
	_drawnPolygonsCount = 0;

	return true;
}

void StudioModelRenderer::Shutdown()
{
}

void StudioModelRenderer::RunFrame()
{
}

unsigned int StudioModelRenderer::DrawModel(studiomdl::ModelRenderInfo* const renderInfo, const renderer::DrawFlags flags)
{
	if (!renderInfo)
	{
		Error("StudioModelRenderer::DrawModel: Called with null render info!\n");
		return 0;
	}

	_renderInfo = renderInfo;

	if (_renderInfo->Model)
	{
		_studioHeader = _renderInfo->Model->GetStudioHeader();
		_textureHeader = _renderInfo->Model->GetTextureHeader();
	}
	else
	{
		Error("StudioModelRenderer::DrawModel: Called with null model!\n");
		return 0;
	}

	++_modelsDrawnCount; // render data cache cookie

	if (_studioHeader->numbodyparts == 0)
	{
		return 0;
	}

	glPushMatrix();

	auto origin = _renderInfo->Origin;

	//TODO: move this out of the renderer
	//The game applies a 1 unit offset to make view models look nicer
	//See https://github.com/ValveSoftware/halflife/blob/c76dd531a79a176eef7cdbca5a80811123afbbe2/cl_dll/view.cpp#L665-L668
	if (flags & renderer::DrawFlag::IS_VIEW_MODEL)
	{
		origin.z -= 1;
	}

	glTranslatef(origin[0], origin[1], origin[2]);

	glRotatef(_renderInfo->Angles[1], 0, 0, 1);
	glRotatef(_renderInfo->Angles[0], 0, 1, 0);
	glRotatef(_renderInfo->Angles[2], 1, 0, 0);

	glScalef(_renderInfo->Scale.x, _renderInfo->Scale.y, _renderInfo->Scale.z);

	SetUpBones();

	SetupLighting();

	unsigned int uiDrawnPolys = 0;

	const bool fixShadowZFighting = (flags & renderer::DrawFlag::FIX_SHADOW_Z_FIGHTING) != 0;

	if (!(flags & renderer::DrawFlag::NODRAW))
	{
		for (int i = 0; i < _studioHeader->numbodyparts; i++)
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
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);

		for (int i = 0; i < _studioHeader->numbodyparts; i++)
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

	glPopMatrix();

	_drawnPolygonsCount += uiDrawnPolys;

	return uiDrawnPolys;
}

void StudioModelRenderer::DrawSingleBone(ModelRenderInfo& renderInfo, const int iBone)
{
	//TODO: rework how stuff is passed in
	auto header = renderInfo.Model->GetStudioHeader();

	if (!header || iBone < 0 || iBone >= header->numbones)
		return;

	_renderInfo = &renderInfo;
	_studioHeader = header;
	_textureHeader = renderInfo.Model->GetTextureHeader();

	SetUpBones();

	const mstudiobone_t* const pbones = _studioHeader->GetBones();
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);

	if (pbones[iBone].parent >= 0)
	{
		glPointSize(10.0f);
		glColor3f(0, 0.7f, 1);
		glBegin(GL_LINES);
		glVertex3f(_bonetransform[pbones[iBone].parent][0][3], _bonetransform[pbones[iBone].parent][1][3], _bonetransform[pbones[iBone].parent][2][3]);
		glVertex3f(_bonetransform[iBone][0][3], _bonetransform[iBone][1][3], _bonetransform[iBone][2][3]);
		glEnd();

		glColor3f(0, 0, 0.8f);
		glBegin(GL_POINTS);
		if (pbones[pbones[iBone].parent].parent != -1)
			glVertex3f(_bonetransform[pbones[iBone].parent][0][3], _bonetransform[pbones[iBone].parent][1][3], _bonetransform[pbones[iBone].parent][2][3]);
		glVertex3f(_bonetransform[iBone][0][3], _bonetransform[iBone][1][3], _bonetransform[iBone][2][3]);
		glEnd();
	}
	else
	{
		// draw parent bone node
		glPointSize(10.0f);
		glColor3f(0.8f, 0, 0);
		glBegin(GL_POINTS);
		glVertex3f(_bonetransform[iBone][0][3], _bonetransform[iBone][1][3], _bonetransform[iBone][2][3]);
		glEnd();
	}

	glPointSize(1.0f);

	_studioHeader = nullptr;
	_textureHeader = nullptr;
	_renderInfo = nullptr;
}

void StudioModelRenderer::DrawSingleAttachment(ModelRenderInfo& renderInfo, const int iAttachment)
{
	//TODO: rework how stuff is passed in
	auto header = renderInfo.Model->GetStudioHeader();

	if (!header || iAttachment < 0 || iAttachment >= header->numattachments)
		return;

	_renderInfo = &renderInfo;
	_studioHeader = header;
	_textureHeader = renderInfo.Model->GetTextureHeader();

	SetUpBones();

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	mstudioattachment_t* pattachments = _studioHeader->GetAttachments();
	glm::vec3 v[4];
	VectorTransform(pattachments[iAttachment].org, _bonetransform[pattachments[iAttachment].bone], v[0]);
	VectorTransform(pattachments[iAttachment].vectors[0], _bonetransform[pattachments[iAttachment].bone], v[1]);
	VectorTransform(pattachments[iAttachment].vectors[1], _bonetransform[pattachments[iAttachment].bone], v[2]);
	VectorTransform(pattachments[iAttachment].vectors[2], _bonetransform[pattachments[iAttachment].bone], v[3]);
	glBegin(GL_LINES);
	glColor3f(0, 1, 1);
	glVertex3fv(glm::value_ptr(v[0]));
	glColor3f(1, 1, 1);
	glVertex3fv(glm::value_ptr(v[1]));
	glColor3f(0, 1, 1);
	glVertex3fv(glm::value_ptr(v[0]));
	glColor3f(1, 1, 1);
	glVertex3fv(glm::value_ptr(v[2]));
	glColor3f(0, 1, 1);
	glVertex3fv(glm::value_ptr(v[0]));
	glColor3f(1, 1, 1);
	glVertex3fv(glm::value_ptr(v[3]));
	glEnd();

	glPointSize(10);
	glColor3f(0, 1, 0);
	glBegin(GL_POINTS);
	glVertex3fv(glm::value_ptr(v[0]));
	glEnd();
	glPointSize(1);

	_studioHeader = nullptr;
	_textureHeader = nullptr;
	_renderInfo = nullptr;
}

void StudioModelRenderer::DrawSingleHitbox(ModelRenderInfo& renderInfo, const int hitboxIndex)
{
	//TODO: rework how stuff is passed in
	auto header = renderInfo.Model->GetStudioHeader();

	if (!header || hitboxIndex < 0 || hitboxIndex >= header->numhitboxes)
		return;

	_renderInfo = &renderInfo;
	_studioHeader = header;
	_textureHeader = renderInfo.Model->GetTextureHeader();

	SetUpBones();

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);
	if (_renderInfo->Transparency < 1.0f)
		glDisable(GL_DEPTH_TEST);
	else
		glEnable(GL_DEPTH_TEST);

	glColor4f(1, 0, 0, 0.5f);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	mstudiobbox_t* hitbox = _studioHeader->GetHitBox(hitboxIndex);
	glm::vec3 v[8], v2[8];

	glm::vec3 bbmin = hitbox->bbmin;
	glm::vec3 bbmax = hitbox->bbmax;

	v[0][0] = bbmin[0];
	v[0][1] = bbmax[1];
	v[0][2] = bbmin[2];

	v[1][0] = bbmin[0];
	v[1][1] = bbmin[1];
	v[1][2] = bbmin[2];

	v[2][0] = bbmax[0];
	v[2][1] = bbmax[1];
	v[2][2] = bbmin[2];

	v[3][0] = bbmax[0];
	v[3][1] = bbmin[1];
	v[3][2] = bbmin[2];

	v[4][0] = bbmax[0];
	v[4][1] = bbmax[1];
	v[4][2] = bbmax[2];

	v[5][0] = bbmax[0];
	v[5][1] = bbmin[1];
	v[5][2] = bbmax[2];

	v[6][0] = bbmin[0];
	v[6][1] = bbmax[1];
	v[6][2] = bbmax[2];

	v[7][0] = bbmin[0];
	v[7][1] = bbmin[1];
	v[7][2] = bbmax[2];

	VectorTransform(v[0], _bonetransform[hitbox->bone], v2[0]);
	VectorTransform(v[1], _bonetransform[hitbox->bone], v2[1]);
	VectorTransform(v[2], _bonetransform[hitbox->bone], v2[2]);
	VectorTransform(v[3], _bonetransform[hitbox->bone], v2[3]);
	VectorTransform(v[4], _bonetransform[hitbox->bone], v2[4]);
	VectorTransform(v[5], _bonetransform[hitbox->bone], v2[5]);
	VectorTransform(v[6], _bonetransform[hitbox->bone], v2[6]);
	VectorTransform(v[7], _bonetransform[hitbox->bone], v2[7]);

	graphics::DrawBox(v2);

	_studioHeader = nullptr;
	_textureHeader = nullptr;
	_renderInfo = nullptr;
}

void StudioModelRenderer::DrawBones()
{
	const mstudiobone_t* const pbones = _studioHeader->GetBones();
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);

	for (int i = 0; i < _studioHeader->numbones; i++)
	{
		if (pbones[i].parent >= 0)
		{
			glPointSize(3.0f);
			glColor3f(1, 0.7f, 0);
			glBegin(GL_LINES);
			glVertex3f(_bonetransform[pbones[i].parent][0][3], _bonetransform[pbones[i].parent][1][3], _bonetransform[pbones[i].parent][2][3]);
			glVertex3f(_bonetransform[i][0][3], _bonetransform[i][1][3], _bonetransform[i][2][3]);
			glEnd();

			glColor3f(0, 0, 0.8f);
			glBegin(GL_POINTS);
			if (pbones[pbones[i].parent].parent != -1)
				glVertex3f(_bonetransform[pbones[i].parent][0][3], _bonetransform[pbones[i].parent][1][3], _bonetransform[pbones[i].parent][2][3]);
			glVertex3f(_bonetransform[i][0][3], _bonetransform[i][1][3], _bonetransform[i][2][3]);
			glEnd();
		}
		else
		{
			// draw parent bone node
			glPointSize(5.0f);
			glColor3f(0.8f, 0, 0);
			glBegin(GL_POINTS);
			glVertex3f(_bonetransform[i][0][3], _bonetransform[i][1][3], _bonetransform[i][2][3]);
			glEnd();
		}
	}

	glPointSize(1.0f);
}

void StudioModelRenderer::DrawAttachments()
{
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	for (int i = 0; i < _studioHeader->numattachments; i++)
	{
		mstudioattachment_t* pattachments = _studioHeader->GetAttachments();
		glm::vec3 v[4];
		VectorTransform(pattachments[i].org, _bonetransform[pattachments[i].bone], v[0]);
		VectorTransform(pattachments[i].vectors[0], _bonetransform[pattachments[i].bone], v[1]);
		VectorTransform(pattachments[i].vectors[1], _bonetransform[pattachments[i].bone], v[2]);
		VectorTransform(pattachments[i].vectors[2], _bonetransform[pattachments[i].bone], v[3]);
		glBegin(GL_LINES);
		glColor3f(1, 0, 0);
		glVertex3fv(glm::value_ptr(v[0]));
		glColor3f(1, 1, 1);
		glVertex3fv(glm::value_ptr(v[1]));
		glColor3f(1, 0, 0);
		glVertex3fv(glm::value_ptr(v[0]));
		glColor3f(1, 1, 1);
		glVertex3fv(glm::value_ptr(v[2]));
		glColor3f(1, 0, 0);
		glVertex3fv(glm::value_ptr(v[0]));
		glColor3f(1, 1, 1);
		glVertex3fv(glm::value_ptr(v[3]));
		glEnd();

		glPointSize(5);
		glColor3f(0, 1, 0);
		glBegin(GL_POINTS);
		glVertex3fv(glm::value_ptr(v[0]));
		glEnd();
		glPointSize(1);
	}
}

void StudioModelRenderer::DrawEyePosition()
{
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	glPointSize(7);
	glColor3f(1, 0, 1);
	glBegin(GL_POINTS);
	glVertex3fv(glm::value_ptr(_studioHeader->eyeposition));
	glEnd();
	glPointSize(1);
}

void StudioModelRenderer::DrawHitBoxes()
{
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);
	if (_renderInfo->Transparency < 1.0f)
		glDisable(GL_DEPTH_TEST);
	else
		glEnable(GL_DEPTH_TEST);

	glColor4f(1, 0, 0, 0.5f);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for (int i = 0; i < _studioHeader->numhitboxes; i++)
	{
		mstudiobbox_t* pbboxes = _studioHeader->GetHitBoxes();
		glm::vec3 v[8], v2[8];

		glm::vec3 bbmin = pbboxes[i].bbmin;
		glm::vec3 bbmax = pbboxes[i].bbmax;

		v[0][0] = bbmin[0];
		v[0][1] = bbmax[1];
		v[0][2] = bbmin[2];

		v[1][0] = bbmin[0];
		v[1][1] = bbmin[1];
		v[1][2] = bbmin[2];

		v[2][0] = bbmax[0];
		v[2][1] = bbmax[1];
		v[2][2] = bbmin[2];

		v[3][0] = bbmax[0];
		v[3][1] = bbmin[1];
		v[3][2] = bbmin[2];

		v[4][0] = bbmax[0];
		v[4][1] = bbmax[1];
		v[4][2] = bbmax[2];

		v[5][0] = bbmax[0];
		v[5][1] = bbmin[1];
		v[5][2] = bbmax[2];

		v[6][0] = bbmin[0];
		v[6][1] = bbmax[1];
		v[6][2] = bbmax[2];

		v[7][0] = bbmin[0];
		v[7][1] = bbmin[1];
		v[7][2] = bbmax[2];

		VectorTransform(v[0], _bonetransform[pbboxes[i].bone], v2[0]);
		VectorTransform(v[1], _bonetransform[pbboxes[i].bone], v2[1]);
		VectorTransform(v[2], _bonetransform[pbboxes[i].bone], v2[2]);
		VectorTransform(v[3], _bonetransform[pbboxes[i].bone], v2[3]);
		VectorTransform(v[4], _bonetransform[pbboxes[i].bone], v2[4]);
		VectorTransform(v[5], _bonetransform[pbboxes[i].bone], v2[5]);
		VectorTransform(v[6], _bonetransform[pbboxes[i].bone], v2[6]);
		VectorTransform(v[7], _bonetransform[pbboxes[i].bone], v2[7]);

		graphics::DrawBox(v2);
	}
}

void StudioModelRenderer::DrawNormals()
{
	glDisable(GL_TEXTURE_2D);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBegin(GL_LINES);

	for (int iBodyPart = 0; iBodyPart < _studioHeader->numbodyparts; ++iBodyPart)
	{
		SetupModel(iBodyPart);

		auto pvertbone = (const byte*)(_studioHeader->GetData() + _model->vertinfoindex);
		auto pnormbone = (const byte*)(_studioHeader->GetData() + _model->norminfoindex);

		auto pMeshes = (const mstudiomesh_t*)(_studioHeader->GetData() + _model->meshindex);

		auto pstudioverts = (const glm::vec3*)(_studioHeader->GetData() + _model->vertindex);
		auto pstudionorms = (const glm::vec3*)(_studioHeader->GetData() + _model->normindex);

		for (int i = 0; i < _model->numverts; i++)
		{
			VectorTransform(pstudioverts[i], _bonetransform[pvertbone[i]], _xformverts[i]);
		}

		for (int i = 0; i < _model->numnorms; i++)
		{
			VectorRotate(pstudionorms[i], _bonetransform[pnormbone[i]], _xformnorms[i]);
		}

		for (int j = 0; j < _model->nummesh; j++)
		{
			auto& mesh = pMeshes[j];
			auto ptricmds = (const short*)(_studioHeader->GetData() + mesh.triindex);

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

					glVertex3fv(glm::value_ptr(vertex));
					glVertex3fv(glm::value_ptr(absoluteNormalEnd));
				}
			}
		}
	}

	glEnd();
}

void StudioModelRenderer::SetUpBones()
{
	static glm::vec3 pos[MAXSTUDIOBONES];
	static glm::vec4 q[MAXSTUDIOBONES];

	static glm::vec3 pos2[MAXSTUDIOBONES];
	static glm::vec4 q2[MAXSTUDIOBONES];
	static glm::vec3 pos3[MAXSTUDIOBONES];
	static glm::vec4 q3[MAXSTUDIOBONES];
	static glm::vec3 pos4[MAXSTUDIOBONES];
	static glm::vec4 q4[MAXSTUDIOBONES];

	if (_renderInfo->Sequence >= _studioHeader->numseq)
	{
		_renderInfo->Sequence = 0;
	}

	mstudioseqdesc_t* const pseqdesc = _studioHeader->GetSequence(_renderInfo->Sequence);

	const mstudioanim_t* panim = _renderInfo->Model->GetAnim(pseqdesc);

	if (pseqdesc->numblends == 9)
	{
		const auto f = _renderInfo->Frame;

		const auto blendX = static_cast<double>(_renderInfo->Blender[0]);
		const auto blendY = static_cast<double>(_renderInfo->Blender[1]);

		const mstudioanim_t* lastanim;

		double interpolantX;
		double interpolantY;

		if (blendX > 127.0)
		{
			interpolantX = blendX - 127.0 + blendX - 127.0;

			if (blendY > 127.0)
			{
				interpolantY = blendY - 127.0 + blendY - 127.0;

				auto panim4 = panim;
				if (pseqdesc->numblends > 4)
					panim4 += 4 * _studioHeader->numbones;
				CalcRotations(pos, q, pseqdesc, panim4, f);

				auto panim5 = panim;
				if (pseqdesc->numblends > 5)
					panim5 += 5 * _studioHeader->numbones;
				CalcRotations(pos2, q2, pseqdesc, panim5, f);

				auto panim7 = panim;
				if (pseqdesc->numblends > 7)
					panim7 += 7 * _studioHeader->numbones;
				CalcRotations(pos3, q3, pseqdesc, panim7, f);

				lastanim = panim;
				if (pseqdesc->numblends > 8)
					lastanim += 8 * _studioHeader->numbones;
			}
			else
			{
				interpolantY = blendY + blendY;

				auto panim1 = panim;
				if (pseqdesc->numblends > 1)
					panim1 += _studioHeader->numbones;
				CalcRotations(pos, q, pseqdesc, panim1, f);

				auto panim2 = panim;
				if (pseqdesc->numblends > 2)
					panim2 += 2 * _studioHeader->numbones;
				CalcRotations(pos2, q2, pseqdesc, panim2, f);

				auto panim4 = panim;
				if (pseqdesc->numblends > 4)
					panim4 += 4 * _studioHeader->numbones;
				CalcRotations(pos3, q3, pseqdesc, panim4, f);

				lastanim = panim;
				if (pseqdesc->numblends > 5)
					lastanim += 5 * _studioHeader->numbones;
			}
		}
		else
		{
			interpolantX = blendX + blendX;

			if (blendY <= 127.0)
			{
				interpolantY = blendY + blendY;

				CalcRotations(pos, q, pseqdesc, panim, f);

				auto panim1 = panim;
				if (pseqdesc->numblends > 1)
					panim1 += _studioHeader->numbones;
				CalcRotations(pos2, q2, pseqdesc, panim1, f);

				auto panim3 = panim;
				if (pseqdesc->numblends > 3)
					panim3 += 3 * _studioHeader->numbones;
				CalcRotations(pos3, q3, pseqdesc, panim3, f);

				lastanim = panim;
				if (pseqdesc->numblends > 4)
					lastanim += 4 * _studioHeader->numbones;
			}
			else
			{
				interpolantY = blendY - 127.0 + blendY - 127.0;

				auto panim3 = panim;
				if (pseqdesc->numblends > 3)
					panim3 += 3 * _studioHeader->numbones;
				CalcRotations(pos, q, pseqdesc, panim3, f);

				auto panim4 = panim;
				if (pseqdesc->numblends > 4)
					panim4 += 4 * _studioHeader->numbones;
				CalcRotations(pos2, q2, pseqdesc, panim4, f);

				auto panim6 = panim;
				if (pseqdesc->numblends > 6)
					panim6 += 6 * _studioHeader->numbones;
				CalcRotations(pos3, q3, pseqdesc, panim6, f);

				lastanim = panim;
				if (pseqdesc->numblends > 7)
					lastanim += 7 * _studioHeader->numbones;
			}
		}

		CalcRotations(pos4, q4, pseqdesc, lastanim, f);

		const auto normalizedInterpolantX = interpolantX / 255.0;
		SlerpBones(q, pos, q2, pos2, normalizedInterpolantX);
		SlerpBones(q3, pos3, q4, pos4, normalizedInterpolantX);

		const auto normalizedInterpolantY = interpolantY / 255.0;
		SlerpBones(q, pos, q3, pos3, normalizedInterpolantY);
	}
	else
	{
		CalcRotations(pos, q, pseqdesc, panim, _renderInfo->Frame);

		if (pseqdesc->numblends > 1)
		{
			panim += _studioHeader->numbones;
			CalcRotations(pos2, q2, pseqdesc, panim, _renderInfo->Frame);
			float s = _renderInfo->Blender[0] / 255.0;

			SlerpBones(q, pos, q2, pos2, s);

			if (pseqdesc->numblends == 4)
			{
				panim += _studioHeader->numbones;
				CalcRotations(pos3, q3, pseqdesc, panim, _renderInfo->Frame);

				panim += _studioHeader->numbones;
				CalcRotations(pos4, q4, pseqdesc, panim, _renderInfo->Frame);

				s = _renderInfo->Blender[0] / 255.0;
				SlerpBones(q3, pos3, q4, pos4, s);

				s = _renderInfo->Blender[1] / 255.0;
				SlerpBones(q, pos, q3, pos3, s);
			}
		}
	}

	const mstudiobone_t* const pbones = _studioHeader->GetBones();

	glm::mat3x4 bonematrix;

	for (int i = 0; i < _studioHeader->numbones; i++)
	{
		QuaternionMatrix(q[i], bonematrix);

		bonematrix[0][3] = pos[i][0];
		bonematrix[1][3] = pos[i][1];
		bonematrix[2][3] = pos[i][2];

		if (pbones[i].parent == -1)
		{
			_bonetransform[i] = bonematrix;
		}
		else
		{
			R_ConcatTransforms(_bonetransform[pbones[i].parent], bonematrix, _bonetransform[i]);
		}
	}
}

void StudioModelRenderer::CalcRotations(glm::vec3* pos, glm::vec4* q, const mstudioseqdesc_t* const pseqdesc, const mstudioanim_t* panim, const float f)
{
	const int frame = (int)f;
	const float s = (f - frame);

	// add in programatic controllers
	CalcBoneAdj();

	auto pbone = _studioHeader->GetBones();

	for (int i = 0; i < _studioHeader->numbones; i++, pbone++, panim++)
	{
		CalcBoneQuaternion(frame, s, pbone, panim, q[i]);
		CalcBonePosition(frame, s, pbone, panim, pos[i]);
	}

	if (pseqdesc->motiontype & STUDIO_X)
		pos[pseqdesc->motionbone][0] = 0.0;
	if (pseqdesc->motiontype & STUDIO_Y)
		pos[pseqdesc->motionbone][1] = 0.0;
	if (pseqdesc->motiontype & STUDIO_Z)
		pos[pseqdesc->motionbone][2] = 0.0;
}

void StudioModelRenderer::CalcBoneAdj()
{
	const auto* const pbonecontroller = _studioHeader->GetBoneControllers();

	for (int j = 0; j < _studioHeader->numbonecontrollers; j++)
	{
		const auto i = pbonecontroller[j].index;

		float value;

		if (i <= 3)
		{
			// check for 360% wrapping
			if (pbonecontroller[j].type & STUDIO_RLOOP)
			{
				value = _renderInfo->Controller[i] * (360.0 / 256.0) + pbonecontroller[j].start;
			}
			else
			{
				value = _renderInfo->Controller[i] / 255.0;
				if (value < 0) value = 0;
				if (value > 1.0) value = 1.0;
				value = (1.0 - value) * pbonecontroller[j].start + value * pbonecontroller[j].end;
			}
			// Con_DPrintf( "%d %d %f : %f\n", m_controller[j], m_prevcontroller[j], value, dadt );
		}
		else
		{
			value = _renderInfo->Mouth / 64.0;
			if (value > 1.0) value = 1.0;
			value = (1.0 - value) * pbonecontroller[j].start + value * pbonecontroller[j].end;
			// Con_DPrintf("%d %f\n", mouthopen, value );
		}
		switch (pbonecontroller[j].type & STUDIO_TYPES)
		{
		case STUDIO_XR:
		case STUDIO_YR:
		case STUDIO_ZR:
			_adj[j] = value * (PI<double> / 180.0);
			break;
		case STUDIO_X:
		case STUDIO_Y:
		case STUDIO_Z:
			_adj[j] = value;
			break;
		}
	}
}

void StudioModelRenderer::CalcBoneQuaternion(const int frame, const float s, const mstudiobone_t* const pbone, const mstudioanim_t* const panim, glm::vec4& q)
{
	glm::vec3			angle1, angle2;

	for (int j = 0; j < 3; j++)
	{
		if (panim->offset[j + 3] == 0)
		{
			angle2[j] = angle1[j] = pbone->value[j + 3]; // default;
		}
		else
		{
			auto panimvalue = (const mstudioanimvalue_t*)((const byte*)panim + panim->offset[j + 3]);
			auto k = frame;
			while (panimvalue->num.total <= k)
			{
				k -= panimvalue->num.total;
				panimvalue += panimvalue->num.valid + 1;
			}
			// Bah, missing blend!
			if (panimvalue->num.valid > k)
			{
				angle1[j] = panimvalue[k + 1].value;

				if (panimvalue->num.valid > k + 1)
				{
					angle2[j] = panimvalue[k + 2].value;
				}
				else
				{
					if (panimvalue->num.total > k + 1)
						angle2[j] = angle1[j];
					else
						angle2[j] = panimvalue[panimvalue->num.valid + 2].value;
				}
			}
			else
			{
				angle1[j] = panimvalue[panimvalue->num.valid].value;
				if (panimvalue->num.total > k + 1)
				{
					angle2[j] = angle1[j];
				}
				else
				{
					angle2[j] = panimvalue[panimvalue->num.valid + 2].value;
				}
			}
			angle1[j] = pbone->value[j + 3] + angle1[j] * pbone->scale[j + 3];
			angle2[j] = pbone->value[j + 3] + angle2[j] * pbone->scale[j + 3];
		}

		if (pbone->bonecontroller[j + 3] != -1)
		{
			angle1[j] += _adj[pbone->bonecontroller[j + 3]];
			angle2[j] += _adj[pbone->bonecontroller[j + 3]];
		}
	}

	if (!VectorCompare(angle1, angle2))
	{
		glm::vec4 q1, q2;

		AngleQuaternion(angle1, q1);
		AngleQuaternion(angle2, q2);
		QuaternionSlerp(q1, q2, s, q);
	}
	else
	{
		AngleQuaternion(angle1, q);
	}
}

void StudioModelRenderer::CalcBonePosition(const int frame, const float s, const mstudiobone_t* const pbone, const mstudioanim_t* const panim, glm::vec3& pos)
{
	for (int j = 0; j < 3; j++)
	{
		pos[j] = pbone->value[j]; // default;
		if (panim->offset[j] != 0)
		{
			auto panimvalue = (mstudioanimvalue_t*)((byte*)panim + panim->offset[j]);

			auto k = frame;
			// find span of values that includes the frame we want
			while (panimvalue->num.total <= k)
			{
				k -= panimvalue->num.total;
				panimvalue += panimvalue->num.valid + 1;
			}
			// if we're inside the span
			if (panimvalue->num.valid > k)
			{
				// and there's more data in the span
				if (panimvalue->num.valid > k + 1)
				{
					pos[j] += (panimvalue[k + 1].value * (1.0 - s) + s * panimvalue[k + 2].value) * pbone->scale[j];
				}
				else
				{
					pos[j] += panimvalue[k + 1].value * pbone->scale[j];
				}
			}
			else
			{
				// are we at the end of the repeating values section and there's another section with data?
				if (panimvalue->num.total <= k + 1)
				{
					pos[j] += (panimvalue[panimvalue->num.valid].value * (1.0 - s) + s * panimvalue[panimvalue->num.valid + 2].value) * pbone->scale[j];
				}
				else
				{
					pos[j] += panimvalue[panimvalue->num.valid].value * pbone->scale[j];
				}
			}
		}
		if (pbone->bonecontroller[j] != -1)
		{
			pos[j] += _adj[pbone->bonecontroller[j]];
		}
	}
}

void StudioModelRenderer::SlerpBones(glm::vec4* q1, glm::vec3* pos1, glm::vec4* q2, glm::vec3* pos2, float s)
{
	glm::vec4 q3;

	if (s < 0) s = 0;
	else if (s > 1.0) s = 1.0;

	const float s1 = 1.0 - s;

	for (int i = 0; i < _studioHeader->numbones; i++)
	{
		QuaternionSlerp(q1[i], q2[i], s, q3);
		q1[i] = q3;

		pos1[i] = pos1[i] * s1 + pos2[i] * s;
	}
}

void StudioModelRenderer::SetupLighting()
{
	_ambientlight = 32;
	_shadelight = 192;

	for (int i = 0; i < _studioHeader->numbones; i++)
	{
		VectorIRotate(_lightvec, _bonetransform[i], _blightvec[i]);
	}
}

void StudioModelRenderer::SetupModel(int bodypart)
{
	if (bodypart > _studioHeader->numbodyparts)
	{
		// Con_DPrintf ("StudioModelRenderer::SetupModel: no such bodypart %d\n", bodypart);
		bodypart = 0;
	}

	_model = _renderInfo->Model->GetModelByBodyPart(_renderInfo->Bodygroup, bodypart);
}

unsigned int StudioModelRenderer::DrawPoints(const bool bWireframe)
{
	unsigned int uiDrawnPolys = 0;

	auto pvertbone = ((byte*)_studioHeader + _model->vertinfoindex);
	auto pnormbone = ((byte*)_studioHeader + _model->norminfoindex);
	auto ptexture = _textureHeader->GetTextures();

	auto pmesh = (mstudiomesh_t*)((byte*)_studioHeader + _model->meshindex);

	auto pstudioverts = (const glm::vec3*)((const byte*)_studioHeader + _model->vertindex);
	auto pstudionorms = (const glm::vec3*)((const byte*)_studioHeader + _model->normindex);

	auto pskinref = _textureHeader->GetSkins();

	if (_renderInfo->Skin != 0 && _renderInfo->Skin < _textureHeader->numskinfamilies)
		pskinref += (_renderInfo->Skin * _textureHeader->numskinref);

	for (int i = 0; i < _model->numverts; i++)
	{
		VectorTransform(pstudioverts[i], _bonetransform[pvertbone[i]], _xformverts[i]);
	}

	SortedMesh meshes[MAXSTUDIOMESHES]{};

	//
	// clip and draw all triangles
	//

	glm::vec3* lv = _lightvalues;
	for (int j = 0; j < _model->nummesh; j++)
	{
		int flags = ptexture[pskinref[pmesh[j].skinref]].flags;

		meshes[j].Mesh = &pmesh[j];
		meshes[j].Flags = flags;

		for (int i = 0; i < pmesh[j].numnorms; i++, ++lv, ++pstudionorms, pnormbone++)
		{
			Lighting(*lv, *pnormbone, flags, *pstudionorms);

			// FIX: move this check out of the inner loop
			if (flags & STUDIO_NF_CHROME)
			{
				auto& c = _chrome[reinterpret_cast<glm::vec3*>(lv) - _lightvalues];

				Chrome(c, *pnormbone, *pstudionorms);
			}
		}
	}

	//Sort meshes by render modes so additive meshes are drawn after solid meshes.
	//Masked meshes are drawn before solid meshes.
	std::stable_sort(meshes, meshes + _model->nummesh, CompareSortedMeshes);

	uiDrawnPolys += DrawMeshes(bWireframe, meshes, ptexture, pskinref);

	glDepthMask(GL_TRUE);

	return uiDrawnPolys;
}

unsigned int StudioModelRenderer::DrawMeshes(const bool bWireframe, const SortedMesh* pMeshes, const mstudiotexture_t* pTextures, const short* pSkinRef)
{
	//Set here since it never changes. Much more efficient.
	if (bWireframe)
	{
		glColor4fv(glm::value_ptr(glm::vec4{_wireframeColor, _renderInfo->Transparency}));
	}

	unsigned int uiDrawnPolys = 0;

	//Polygons may overlap, so make sure they can blend together.
	glDepthFunc(GL_LEQUAL);

	for (int j = 0; j < _model->nummesh; j++)
	{
		auto pmesh = pMeshes[j].Mesh;
		auto ptricmds = (short*)((byte*)_studioHeader + pmesh->triindex);

		const mstudiotexture_t& texture = pTextures[pSkinRef[pmesh->skinref]];

		const auto s = 1.0 / (float)texture.width;
		const auto t = 1.0 / (float)texture.height;

		if (texture.flags & STUDIO_NF_ADDITIVE)
			glDepthMask(GL_FALSE);
		else
			glDepthMask(GL_TRUE);

		if (texture.flags & STUDIO_NF_ADDITIVE)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		}
		else if (_renderInfo->Transparency < 1.0f)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		else
			glDisable(GL_BLEND);

		if (texture.flags & STUDIO_NF_MASKED)
		{
			glEnable(GL_ALPHA_TEST);
			glAlphaFunc(GL_GREATER, 0.5f);
		}

		if (!bWireframe)
		{
			glBindTexture(GL_TEXTURE_2D, _renderInfo->Model->GetTextureId(pSkinRef[pmesh->skinref]));
		}

		int i;

		while (i = *(ptricmds++))
		{
			if (i < 0)
			{
				glBegin(GL_TRIANGLE_FAN);
				i = -i;
			}
			else
			{
				glBegin(GL_TRIANGLE_STRIP);
			}

			uiDrawnPolys += i - 2;

			for (; i > 0; i--, ptricmds += 4)
			{
				if (!bWireframe)
				{
					if (texture.flags & STUDIO_NF_CHROME)
					{
						const auto& c = _chrome[ptricmds[1]];

						glTexCoord2f(c[0], c[1]);
					}
					else
					{
						glTexCoord2f(ptricmds[2] * s, ptricmds[3] * t);
					}

					if (texture.flags & STUDIO_NF_ADDITIVE)
					{
						glColor4f(1.0f, 1.0f, 1.0f, _renderInfo->Transparency);
					}
					else
					{
						const glm::vec3& lightVec = _lightvalues[ptricmds[1]];
						glColor4f(lightVec[0], lightVec[1], lightVec[2], _renderInfo->Transparency);
					}
				}

				glVertex3fv(glm::value_ptr(_xformverts[ptricmds[0]]));
			}
			glEnd();
		}

		if (texture.flags & STUDIO_NF_MASKED)
			glDisable(GL_ALPHA_TEST);
	}

	return uiDrawnPolys;
}

unsigned int StudioModelRenderer::DrawShadows(const bool fixZFighting, const bool wireframe)
{
	if (!(_studioHeader->flags & EF_NOSHADELIGHT))
	{
		GLint oldDepthMask;
		glGetIntegerv(GL_DEPTH_WRITEMASK, &oldDepthMask);

		if (fixZFighting)
		{
			glDepthMask(GL_FALSE);
		}
		else
		{
			glDepthMask(GL_TRUE);
		}

		const float r_blend = _renderInfo->Transparency;

		const auto alpha = 0.5 * r_blend;

		glDisable(GL_TEXTURE_2D);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);

		if (wireframe)
		{
			glColor4fv(glm::value_ptr(glm::vec4{_wireframeColor, _renderInfo->Transparency}));
		}
		else
		{
			//Render shadows as black
			glColor4f(0.f, 0.f, 0.f, alpha);
		}

		glDepthFunc(GL_LESS);

		const auto drawnPolys = InternalDrawShadows();

		glDepthFunc(GL_LEQUAL);

		glEnable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
		glColor4f(1.f, 1.f, 1.f, 1.f);
		glShadeModel(GL_SMOOTH);

		glDepthMask(static_cast<GLboolean>(oldDepthMask));

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

	for (int mesh = 0; mesh < _model->nummesh; ++mesh)
	{
		auto v4 = reinterpret_cast<mstudiomesh_t*>(_studioHeader->GetData() + _model->meshindex) + mesh;
		drawnPolys += v4->numtris;

		auto triCmds = reinterpret_cast<short*>(_studioHeader->GetData() + v4->triindex);

		for (int i; (i = *triCmds++) != 0;)
		{
			if (i < 0)
			{
				i = -i;
				glBegin(GL_TRIANGLE_FAN);
			}
			else
			{
				glBegin(GL_TRIANGLE_STRIP);
			}

			for (; i > 0; --i, triCmds += 4)
			{
				const auto vertex{_xformverts[triCmds[0]]};

				const auto lightDistance = vertex.z - lightSampleHeight;

				glm::vec3 point;

				point.x = vertex.x - _lightvec.x * lightDistance;
				point.y = vertex.y - _lightvec.y * lightDistance;
				point.z = shadowHeight;

				glVertex3fv(glm::value_ptr(point));
			}

			glEnd();
		}
	}

	return drawnPolys;
}

void StudioModelRenderer::Lighting(glm::vec3& lv, int bone, int flags, const glm::vec3& normal)
{
	const float ambient = std::max(0.1f, (float)_ambientlight / 255.0f); // to avoid divison by zero
	const float shade = _shadelight / 255.0f;
	glm::vec3 illum{ambient};

	if (flags & STUDIO_NF_FULLBRIGHT)
	{
		lv = glm::vec3{1, 1, 1};
		return;
	}
	else if (flags & STUDIO_NF_FLATSHADE)
	{
		VectorMA(illum, 0.8f, glm::vec3{shade}, illum);
	}
	else
	{
		auto lightcos = glm::dot(normal, _blightvec[bone]); // -1 colinear, 1 opposite

		if (lightcos > 1.0f) lightcos = 1;

		illum += _shadelight / 255.0f;

		auto r = _lambert;
		if (r < 1.0f) r = 1.0f;
		lightcos = (lightcos + (r - 1.0f)) / r; // do modified hemispherical lighting
		if (lightcos > 0.0f) VectorMA(illum, -lightcos, glm::vec3{shade}, illum);

		if (illum[0] <= 0) illum[0] = 0;
		if (illum[1] <= 0) illum[1] = 0;
		if (illum[2] <= 0) illum[2] = 0;
	}

	float max = VectorMax(illum);

	if (max > 1.0f)
		lv = illum * (1.0f / max);
	else lv = illum;

	lv *= _lightcolor;
}


void StudioModelRenderer::Chrome(glm::vec2& chrome, int bone, const glm::vec3& normal)
{
	if (_chromeage[bone] != _modelsDrawnCount)
	{
		// calculate vectors from the viewer to the bone. This roughly adjusts for position
		// vector pointing at bone in world reference frame
		auto tmp = _viewerOrigin * -1.0f;

		tmp[0] += _bonetransform[bone][0][3];
		tmp[1] += _bonetransform[bone][1][3];
		tmp[2] += _bonetransform[bone][2][3];

		VectorNormalize(tmp);
		// g_chrome t vector in world reference frame
		auto chromeupvec = glm::cross(tmp, _viewerRight);
		VectorNormalize(chromeupvec);
		// g_chrome s vector in world reference frame
		auto chromerightvec = glm::cross(tmp, chromeupvec);
		VectorNormalize(chromerightvec);

		VectorIRotate(-chromeupvec, _bonetransform[bone], _chromeup[bone]);
		VectorIRotate(chromerightvec, _bonetransform[bone], _chromeright[bone]);

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