#include <cfloat>
#include <cstdint>

#include <wx/gbsizer.h>
#include <wx/textctrl.h>

#include "shared/renderer/studiomodel/IStudioModelRenderer.h"

#include "../CModelViewerApp.h"
#include "../../CHLMVState.h"

#include "CModelDataPanel.h"

namespace hlmv
{
CModelDataPanel::CModelDataPanel(wxWindow* pParent, CModelViewerApp* const pHLMV)
	: CBaseControlPanel(pParent, "Model Data", pHLMV)
{
	auto elemParent = GetElementParent();

	for (auto& value : m_pOrigin)
	{
		value = new wxSpinCtrlDouble(elemParent, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS);
		value->SetRange(-DBL_MAX, DBL_MAX);
		value->SetDigits(6);
		value->Bind(wxEVT_SPINCTRLDOUBLE, &CModelDataPanel::OnOriginChanged, this);
	}

	m_pSetOrigin = new wxButton(elemParent, wxID_ANY, "Set Origin");

	m_pSetOrigin->Bind(wxEVT_BUTTON, &CModelDataPanel::OnSetOrigin, this);

	m_pMeshScale = new wxSpinCtrlDouble(elemParent, wxID_ANY, "1.0");
	m_pMeshScaleButton = new wxButton(elemParent, wxID_ANY, "Scale Mesh");
	m_pMeshScaleButton->Bind(wxEVT_BUTTON, &CModelDataPanel::OnScaleMesh, this);

	m_pMeshScale->SetRange(DBL_MIN, DBL_MAX);
	m_pMeshScale->SetDigits(2);

	m_pBonesScale = new wxSpinCtrlDouble(elemParent, wxID_ANY, "1.0");
	m_pBonesScaleButton = new wxButton(elemParent, wxID_ANY, "Scale Bones");
	m_pBonesScaleButton->Bind(wxEVT_BUTTON, &CModelDataPanel::OnScaleBones, this);

	m_pBonesScale->SetRange(DBL_MIN, DBL_MAX);
	m_pBonesScale->SetDigits(2);

	//Layout
	auto sizer = new wxGridBagSizer(1, 1);

	sizer->Add(new wxStaticText(elemParent, wxID_ANY, "Origin X"), wxGBPosition(0, 0), wxDefaultSpan, wxEXPAND);
	sizer->Add(new wxStaticText(elemParent, wxID_ANY, "Origin Y"), wxGBPosition(1, 0), wxDefaultSpan, wxEXPAND);
	sizer->Add(new wxStaticText(elemParent, wxID_ANY, "Origin Z"), wxGBPosition(2, 0), wxDefaultSpan, wxEXPAND);

	sizer->Add(m_pOrigin[0], wxGBPosition(0, 1), wxDefaultSpan, wxEXPAND);
	sizer->Add(m_pOrigin[1], wxGBPosition(1, 1), wxDefaultSpan, wxEXPAND);
	sizer->Add(m_pOrigin[2], wxGBPosition(2, 1), wxDefaultSpan, wxEXPAND);

	sizer->Add(m_pSetOrigin, wxGBPosition(3, 1), wxDefaultSpan, wxEXPAND);

	auto scaleSizer = new wxGridBagSizer(1, 1);

	scaleSizer->Add(m_pMeshScale, wxGBPosition(0, 0), wxDefaultSpan, wxEXPAND);
	scaleSizer->Add(m_pMeshScaleButton, wxGBPosition(0, 1), wxDefaultSpan, wxEXPAND);

	scaleSizer->Add(m_pBonesScale, wxGBPosition(1, 0), wxDefaultSpan, wxEXPAND);
	scaleSizer->Add(m_pBonesScaleButton, wxGBPosition(1, 1), wxDefaultSpan, wxEXPAND);

	sizer->Add(scaleSizer, wxGBPosition(0, 2), wxGBSpan(4, 1), wxEXPAND, wxALL);

	GetMainSizer()->Add(sizer);
}

CModelDataPanel::~CModelDataPanel()
{
}

void CModelDataPanel::InitializeUI()
{
	bool success = false;

	m_RootBonePositions.clear();

	m_pMeshScale->SetValue(1.0);
	m_pBonesScale->SetValue(1.0);

	if (auto entity = m_pHLMV->GetState()->GetEntity(); entity)
	{
		if (auto model = entity->GetModel(); model)
		{
			for (auto value : m_pOrigin)
			{
				value->SetValue(0);
			}

			for (auto rootBone : model->GetRootBones())
			{
				m_RootBonePositions.emplace_back(
					RootBoneData
					{
						rootBone,
						{
							rootBone->value[0],
							rootBone->value[1],
							rootBone->value[2]
						}
					});
			}

			success = true;
		}
	}

	this->Enable(success);
}

void CModelDataPanel::OnPostDraw(studiomdl::IStudioModelRenderer& renderer, const studiomdl::CModelRenderInfo& info)
{
}

void CModelDataPanel::UpdateOrigin()
{
	if (auto pEntity = m_pHLMV->GetState()->GetEntity(); pEntity)
	{
		if (!m_RootBonePositions.empty())
		{
			const glm::vec3 offset{m_pOrigin[0]->GetValue(), m_pOrigin[1]->GetValue(), m_pOrigin[2]->GetValue()};

			for (auto& data : m_RootBonePositions)
			{
				const auto newPosition = data.OriginalRootBonePosition + offset;

				data.Bone->value[0] = newPosition.x;
				data.Bone->value[1] = newPosition.y;
				data.Bone->value[2] = newPosition.z;
			}

			m_pHLMV->GetState()->modelChanged = true;
		}
	}
}

void CModelDataPanel::OnOriginChanged(wxSpinDoubleEvent& event)
{
	UpdateOrigin();
}

void CModelDataPanel::OnSetOrigin(wxCommandEvent& event)
{
	UpdateOrigin();
}

void CModelDataPanel::OnScaleMesh(wxCommandEvent& event)
{
	if (auto entity = m_pHLMV->GetState()->GetEntity(); entity)
	{
		studiomdl::ScaleMeshes(entity->GetModel(), m_pMeshScale->GetValue());

		m_pHLMV->GetState()->modelChanged = true;
	}
}

void CModelDataPanel::OnScaleBones(wxCommandEvent& event)
{
	if (auto entity = m_pHLMV->GetState()->GetEntity(); entity)
	{
		studiomdl::ScaleBones(entity->GetModel(), m_pBonesScale->GetValue());

		m_pHLMV->GetState()->modelChanged = true;
	}
}
}