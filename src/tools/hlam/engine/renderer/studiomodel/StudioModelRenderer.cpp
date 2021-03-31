#include <glm/gtc/type_ptr.hpp>

#include <algorithm>

#include "core/shared/Logging.hpp"

#include "graphics/GraphicsUtils.hpp"

#include "engine/shared/studiomodel/EditableStudioModel.hpp"

#include "engine/renderer/studiomodel/StudioModelRenderer.hpp"

#include "utility/mathlib.hpp"

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
		_studioModel = _renderInfo->Model;
	}
	else
	{
		Error("StudioModelRenderer::DrawModel: Called with null model!\n");
		return 0;
	}

	++_modelsDrawnCount; // render data cache cookie

	if (_studioModel->Bodyparts.empty())
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
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);

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

	glPopMatrix();

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

	SetUpBones();

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);

	const auto& bone = *model->Bones[iBone];

	const auto& boneTransform = _bonetransform[bone.ArrayIndex];

	if (bone.Parent)
	{
		const auto& parentBone = *bone.Parent;

		const auto& parentBoneTransform = _bonetransform[parentBone.ArrayIndex];

		glPointSize(10.0f);
		glColor3f(0, 0.7f, 1);
		glBegin(GL_LINES);
		glVertex3f(parentBoneTransform[0][3], parentBoneTransform[1][3], parentBoneTransform[2][3]);
		glVertex3f(_bonetransform[iBone][0][3], _bonetransform[iBone][1][3], _bonetransform[iBone][2][3]);
		glEnd();

		glColor3f(0, 0, 0.8f);
		glBegin(GL_POINTS);
		if (parentBone.Parent)
			glVertex3f(parentBoneTransform[0][3], parentBoneTransform[1][3], parentBoneTransform[2][3]);
		glVertex3f(boneTransform[0][3], boneTransform[1][3], boneTransform[2][3]);
		glEnd();
	}
	else
	{
		// draw parent bone node
		glPointSize(10.0f);
		glColor3f(0.8f, 0, 0);
		glBegin(GL_POINTS);
		glVertex3f(boneTransform[0][3], boneTransform[1][3], boneTransform[2][3]);
		glEnd();
	}

	glPointSize(1.0f);

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

	SetUpBones();

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	const auto& attachment = *_studioModel->Attachments[iAttachment];

	const auto& attachmentBoneTransform = _bonetransform[attachment.Bone->ArrayIndex];

	glm::vec3 v[4];
	VectorTransform(attachment.Origin, attachmentBoneTransform, v[0]);
	VectorTransform(attachment.Vectors[0], attachmentBoneTransform, v[1]);
	VectorTransform(attachment.Vectors[1], attachmentBoneTransform, v[2]);
	VectorTransform(attachment.Vectors[2], attachmentBoneTransform, v[3]);
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

	const auto& hitbox = *_studioModel->Hitboxes[hitboxIndex];

	const auto v = graphics::CreateBoxFromBounds(hitbox.Min, hitbox.Max);

	const auto& hitboxBoneTransform = _bonetransform[hitbox.Bone->ArrayIndex];

	std::array<glm::vec3, 8> v2{};

	VectorTransform(v[0], hitboxBoneTransform, v2[0]);
	VectorTransform(v[1], hitboxBoneTransform, v2[1]);
	VectorTransform(v[2], hitboxBoneTransform, v2[2]);
	VectorTransform(v[3], hitboxBoneTransform, v2[3]);
	VectorTransform(v[4], hitboxBoneTransform, v2[4]);
	VectorTransform(v[5], hitboxBoneTransform, v2[5]);
	VectorTransform(v[6], hitboxBoneTransform, v2[6]);
	VectorTransform(v[7], hitboxBoneTransform, v2[7]);

	graphics::DrawBox(v2);

	_studioModel = nullptr;
	_renderInfo = nullptr;
}

void StudioModelRenderer::DrawBones()
{
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);

	for (int i = 0; i < _studioModel->Bones.size(); i++)
	{
		const auto& bone = *_studioModel->Bones[i];

		const auto& boneTransform = _bonetransform[i];

		if (bone.Parent)
		{
			const auto& parentBoneTransform = _bonetransform[bone.Parent->ArrayIndex];

			glPointSize(3.0f);
			glColor3f(1, 0.7f, 0);
			glBegin(GL_LINES);
			glVertex3f(parentBoneTransform[0][3], parentBoneTransform[1][3], parentBoneTransform[2][3]);
			glVertex3f(_bonetransform[i][0][3], _bonetransform[i][1][3], _bonetransform[i][2][3]);
			glEnd();

			glColor3f(0, 0, 0.8f);
			glBegin(GL_POINTS);
			if (bone.Parent->Parent)
				glVertex3f(parentBoneTransform[0][3], parentBoneTransform[1][3], parentBoneTransform[2][3]);
			glVertex3f(boneTransform[0][3], boneTransform[1][3], boneTransform[2][3]);
			glEnd();
		}
		else
		{
			// draw parent bone node
			glPointSize(5.0f);
			glColor3f(0.8f, 0, 0);
			glBegin(GL_POINTS);
			glVertex3f(boneTransform[0][3], boneTransform[1][3], boneTransform[2][3]);
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

	for (int i = 0; i < _studioModel->Attachments.size(); i++)
	{
		const auto& attachment = *_studioModel->Attachments[i];

		const auto& attachmentBoneTransform = _bonetransform[attachment.Bone->ArrayIndex];

		glm::vec3 v[4];
		VectorTransform(attachment.Origin, attachmentBoneTransform, v[0]);
		VectorTransform(attachment.Vectors[0], attachmentBoneTransform, v[1]);
		VectorTransform(attachment.Vectors[1], attachmentBoneTransform, v[2]);
		VectorTransform(attachment.Vectors[2], attachmentBoneTransform, v[3]);
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
	glVertex3fv(glm::value_ptr(_studioModel->EyePosition));
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

	for (int i = 0; i < _studioModel->Hitboxes.size(); i++)
	{
		const auto& hitbox = *_studioModel->Hitboxes[i];

		const auto v = graphics::CreateBoxFromBounds(hitbox.Min, hitbox.Max);

		const auto& hitboxTransform = _bonetransform[hitbox.Bone->ArrayIndex];

		std::array<glm::vec3, 8> v2{};

		VectorTransform(v[0], hitboxTransform, v2[0]);
		VectorTransform(v[1], hitboxTransform, v2[1]);
		VectorTransform(v[2], hitboxTransform, v2[2]);
		VectorTransform(v[3], hitboxTransform, v2[3]);
		VectorTransform(v[4], hitboxTransform, v2[4]);
		VectorTransform(v[5], hitboxTransform, v2[5]);
		VectorTransform(v[6], hitboxTransform, v2[6]);
		VectorTransform(v[7], hitboxTransform, v2[7]);

		graphics::DrawBox(v2);
	}
}

void StudioModelRenderer::DrawNormals()
{
	glDisable(GL_TEXTURE_2D);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBegin(GL_LINES);

	for (int iBodyPart = 0; iBodyPart < _studioModel->Bodyparts.size(); ++iBodyPart)
	{
		SetupModel(iBodyPart);

		for (int i = 0; i < _model->Vertices.size(); i++)
		{
			VectorTransform(_model->Vertices[i].Vertex, _bonetransform[_model->Vertices[i].Bone->ArrayIndex], _xformverts[i]);
		}

		for (int i = 0; i < _model->Normals.size(); i++)
		{
			VectorRotate(_model->Normals[i].Vertex, _bonetransform[_model->Normals[i].Bone->ArrayIndex], _xformnorms[i]);
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

	if (_renderInfo->Sequence >= _studioModel->Sequences.size())
	{
		_renderInfo->Sequence = 0;
	}

	const auto& sequence = *_studioModel->Sequences[_renderInfo->Sequence];

	if (sequence.AnimationBlends.size() == 9)
	{
		const auto f = _renderInfo->Frame;

		const auto blendX = static_cast<double>(_renderInfo->Blender[0]);
		const auto blendY = static_cast<double>(_renderInfo->Blender[1]);

		double interpolantX;
		double interpolantY;

		if (blendX > 127.0)
		{
			interpolantX = blendX - 127.0 + blendX - 127.0;

			if (blendY > 127.0)
			{
				interpolantY = blendY - 127.0 + blendY - 127.0;

				CalcRotations(pos, q, sequence, sequence.AnimationBlends[4], f);
				CalcRotations(pos2, q2, sequence, sequence.AnimationBlends[5], f);
				CalcRotations(pos3, q3, sequence, sequence.AnimationBlends[7], f);
				CalcRotations(pos4, q4, sequence, sequence.AnimationBlends[8], f);
			}
			else
			{
				interpolantY = blendY + blendY;

				CalcRotations(pos, q, sequence, sequence.AnimationBlends[1], f);
				CalcRotations(pos2, q2, sequence, sequence.AnimationBlends[2], f);
				CalcRotations(pos3, q3, sequence, sequence.AnimationBlends[4], f);
				CalcRotations(pos4, q4, sequence, sequence.AnimationBlends[5], f);
			}
		}
		else
		{
			interpolantX = blendX + blendX;

			if (blendY <= 127.0)
			{
				interpolantY = blendY + blendY;

				CalcRotations(pos, q, sequence, sequence.AnimationBlends[0], f);
				CalcRotations(pos2, q2, sequence, sequence.AnimationBlends[1], f);
				CalcRotations(pos3, q3, sequence, sequence.AnimationBlends[3], f);
				CalcRotations(pos4, q4, sequence, sequence.AnimationBlends[4], f);
			}
			else
			{
				interpolantY = blendY - 127.0 + blendY - 127.0;

				CalcRotations(pos, q, sequence, sequence.AnimationBlends[3], f);
				CalcRotations(pos2, q2, sequence, sequence.AnimationBlends[4], f);
				CalcRotations(pos3, q3, sequence, sequence.AnimationBlends[6], f);
				CalcRotations(pos4, q4, sequence, sequence.AnimationBlends[7], f);
			}
		}

		const auto normalizedInterpolantX = interpolantX / 255.0;
		SlerpBones(q, pos, q2, pos2, normalizedInterpolantX);
		SlerpBones(q3, pos3, q4, pos4, normalizedInterpolantX);

		const auto normalizedInterpolantY = interpolantY / 255.0;
		SlerpBones(q, pos, q3, pos3, normalizedInterpolantY);
	}
	else
	{
		CalcRotations(pos, q, sequence, sequence.AnimationBlends[0], _renderInfo->Frame);

		if (sequence.AnimationBlends.size() > 1)
		{
			CalcRotations(pos2, q2, sequence, sequence.AnimationBlends[1], _renderInfo->Frame);
			float s = _renderInfo->Blender[0] / 255.0;

			SlerpBones(q, pos, q2, pos2, s);

			if (sequence.AnimationBlends[0].size() == 4)
			{
				CalcRotations(pos3, q3, sequence, sequence.AnimationBlends[2], _renderInfo->Frame);
				CalcRotations(pos4, q4, sequence, sequence.AnimationBlends[3], _renderInfo->Frame);

				s = _renderInfo->Blender[0] / 255.0;
				SlerpBones(q3, pos3, q4, pos4, s);

				s = _renderInfo->Blender[1] / 255.0;
				SlerpBones(q, pos, q3, pos3, s);
			}
		}
	}

	glm::mat3x4 bonematrix;

	for (int i = 0; i < _studioModel->Bones.size(); i++)
	{
		const auto& bone = *_studioModel->Bones[i];

		QuaternionMatrix(q[i], bonematrix);

		bonematrix[0][3] = pos[i][0];
		bonematrix[1][3] = pos[i][1];
		bonematrix[2][3] = pos[i][2];

		if (!bone.Parent)
		{
			//Apply scale to each root bone so only the model is scaled and mirrored, and not anything else in the scene
			bonematrix = glm::scale(glm::mat4x4{bonematrix}, _renderInfo->Scale);

			_bonetransform[i] = bonematrix;
		}
		else
		{
			R_ConcatTransforms(_bonetransform[bone.Parent->ArrayIndex], bonematrix, _bonetransform[i]);
		}
	}
}

void StudioModelRenderer::CalcRotations(glm::vec3* pos, glm::vec4* q, const Sequence& sequence, const std::vector<Animation>& anims, const float f)
{
	const int frame = (int)f;
	const float s = (f - frame);

	// add in programatic controllers
	CalcBoneAdj();

	for (int i = 0; i < _studioModel->Bones.size(); i++)
	{
		const auto& bone = *_studioModel->Bones[i];
		const auto& anim = anims[i];

		CalcBoneQuaternion(frame, s, bone, anim, q[i]);
		CalcBonePosition(frame, s, bone, anim, pos[i]);
	}

	if (sequence.MotionType & STUDIO_X)
		pos[sequence.MotionBone][0] = 0.0;
	if (sequence.MotionType & STUDIO_Y)
		pos[sequence.MotionBone][1] = 0.0;
	if (sequence.MotionType & STUDIO_Z)
		pos[sequence.MotionBone][2] = 0.0;
}

void StudioModelRenderer::CalcBoneAdj()
{
	for (int j = 0; j < _studioModel->BoneControllers.size(); j++)
	{
		const auto& boneController = *_studioModel->BoneControllers[j];

		const auto i = boneController.Index;

		float value;

		if (i <= 3)
		{
			// check for 360% wrapping
			if (boneController.Type & STUDIO_RLOOP)
			{
				value = _renderInfo->Controller[i] * (360.0 / 256.0) + boneController.Start;
			}
			else
			{
				value = _renderInfo->Controller[i] / 255.0;
				if (value < 0) value = 0;
				if (value > 1.0) value = 1.0;
				value = (1.0 - value) * boneController.Start + value * boneController.End;
			}
			// Con_DPrintf( "%d %d %f : %f\n", m_controller[j], m_prevcontroller[j], value, dadt );
		}
		else
		{
			value = _renderInfo->Mouth / 64.0;
			if (value > 1.0) value = 1.0;
			value = (1.0 - value) * boneController.Start + value * boneController.End;
			// Con_DPrintf("%d %f\n", mouthopen, value );
		}
		switch (boneController.Type & STUDIO_TYPES)
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

void StudioModelRenderer::CalcBoneQuaternion(const int frame, const float s, const Bone& bone, const Animation& anim, glm::vec4& q)
{
	glm::vec3			angle1, angle2;

	for (int j = 0; j < 3; j++)
	{
		const auto& axis = bone.Axes[j + 3];

		if (anim.Data[j + 3].empty())
		{
			angle2[j] = angle1[j] = axis.Value; // default;
		}
		else
		{
			auto panimvalue = anim.Data[j + 3].data();
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
			angle1[j] = axis.Value + angle1[j] * axis.Scale;
			angle2[j] = axis.Value + angle2[j] * axis.Scale;
		}

		if (axis.Controller)
		{
			angle1[j] += _adj[axis.Controller->ArrayIndex];
			angle2[j] += _adj[axis.Controller->ArrayIndex];
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

void StudioModelRenderer::CalcBonePosition(const int frame, const float s, const Bone& bone, const Animation& anim, glm::vec3& pos)
{
	for (int j = 0; j < 3; j++)
	{
		const auto& axis = bone.Axes[j];

		pos[j] = axis.Value; // default;
		if (!anim.Data[j].empty())
		{
			auto panimvalue = anim.Data[j].data();

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
					pos[j] += (panimvalue[k + 1].value * (1.0 - s) + s * panimvalue[k + 2].value) * axis.Scale;
				}
				else
				{
					pos[j] += panimvalue[k + 1].value * axis.Scale;
				}
			}
			else
			{
				// are we at the end of the repeating values section and there's another section with data?
				if (panimvalue->num.total <= k + 1)
				{
					pos[j] += (panimvalue[panimvalue->num.valid].value * (1.0 - s) + s * panimvalue[panimvalue->num.valid + 2].value) * axis.Scale;
				}
				else
				{
					pos[j] += panimvalue[panimvalue->num.valid].value * axis.Scale;
				}
			}
		}
		if (axis.Controller)
		{
			pos[j] += _adj[axis.Controller->ArrayIndex];
		}
	}
}

void StudioModelRenderer::SlerpBones(glm::vec4* q1, glm::vec3* pos1, glm::vec4* q2, glm::vec3* pos2, float s)
{
	glm::vec4 q3;

	if (s < 0) s = 0;
	else if (s > 1.0) s = 1.0;

	const float s1 = 1.0 - s;

	for (int i = 0; i < _studioModel->Bones.size(); i++)
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

	for (int i = 0; i < _studioModel->Bones.size(); i++)
	{
		VectorIRotate(_lightvec, _bonetransform[i], _blightvec[i]);
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
		VectorTransform(_model->Vertices[i].Vertex, _bonetransform[_model->Vertices[i].Bone->ArrayIndex], _xformverts[i]);
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

	glDepthMask(GL_TRUE);

	return uiDrawnPolys;
}

unsigned int StudioModelRenderer::DrawMeshes(const bool bWireframe, const SortedMesh* pMeshes)
{
	//Set here since it never changes. Much more efficient.
	if (bWireframe)
	{
		glColor4fv(glm::value_ptr(glm::vec4{_wireframeColor, _renderInfo->Transparency}));
	}

	unsigned int uiDrawnPolys = 0;

	//Polygons may overlap, so make sure they can blend together.
	glDepthFunc(GL_LEQUAL);

	for (int j = 0; j < _model->Meshes.size(); j++)
	{
		const auto& mesh = *pMeshes[j].Mesh;
		auto ptricmds = mesh.Triangles.data();

		const auto& texture = *_studioModel->SkinFamilies[_renderInfo->Skin][mesh.SkinRef];

		const auto s = 1.0 / (float)texture.Width;
		const auto t = 1.0 / (float)texture.Height;

		if (!bWireframe)
		{
			if (texture.Flags & STUDIO_NF_ADDITIVE)
				glDepthMask(GL_FALSE);
			else
				glDepthMask(GL_TRUE);

			if (texture.Flags & STUDIO_NF_ADDITIVE)
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
			{
				glDisable(GL_BLEND);
			}

			if (texture.Flags & STUDIO_NF_MASKED)
			{
				glEnable(GL_ALPHA_TEST);
				glAlphaFunc(GL_GREATER, 0.5f);
			}

			glBindTexture(GL_TEXTURE_2D, texture.TextureId);
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
					if (texture.Flags & STUDIO_NF_CHROME)
					{
						const auto& c = _chrome[ptricmds[1]];

						glTexCoord2f(c[0], c[1]);
					}
					else
					{
						glTexCoord2f(ptricmds[2] * s, ptricmds[3] * t);
					}

					if (texture.Flags & STUDIO_NF_ADDITIVE)
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

		if (!bWireframe)
		{
			if (texture.Flags & STUDIO_NF_ADDITIVE)
			{
				glDisable(GL_BLEND);
			}

			if (texture.Flags & STUDIO_NF_MASKED)
			{
				glDisable(GL_ALPHA_TEST);
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

		const GLboolean texture2DWasEnabled = glIsEnabled(GL_TEXTURE_2D);

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

		if (texture2DWasEnabled)
		{
			glEnable(GL_TEXTURE_2D);
		}

		glDisable(GL_BLEND);
		glColor4f(1.f, 1.f, 1.f, 1.f);

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