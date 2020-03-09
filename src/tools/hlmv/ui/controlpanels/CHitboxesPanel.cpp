#include <cfloat>
#include <cstdint>

#include <wx/gbsizer.h>

#include "shared/renderer/studiomodel/IStudioModelRenderer.h"

#include "../CModelViewerApp.h"
#include "../../CHLMVState.h"

#include "CHitboxesPanel.h"

namespace hlmv
{
CHitboxesPanel::CHitboxesPanel(wxWindow* pParent, CModelViewerApp* const pHLMV)
	: CBaseControlPanel(pParent, "Hitboxes", pHLMV)
{
	auto pElemParent = GetElementParent();

	m_pHitboxes = new wxChoice(pElemParent, wxID_ANY, wxDefaultPosition, wxSize(125, wxDefaultSize.GetHeight()));

	m_pHitboxes->Bind(wxEVT_CHOICE, &CHitboxesPanel::OnHitboxChanged, this);

	m_pHitboxInfo = new wxPanel(pElemParent);

	m_pBoneName = new wxStaticText(m_pHitboxInfo, wxID_ANY, "");

	m_pBone = new wxSpinCtrl(m_pHitboxInfo, wxID_ANY);
	m_pBone->Bind(wxEVT_SPINCTRL, &CHitboxesPanel::OnBoneChanged, this);

	m_pHitgroup = new wxSpinCtrl(m_pHitboxInfo, wxID_ANY);

	//The compiler allows any integer value, so don't clamp it at all
	m_pHitgroup->SetRange(INT_MIN, INT_MAX);
	m_pHitgroup->Bind(wxEVT_SPINCTRL, &CHitboxesPanel::OnHitgroupChanged, this);

	for (int i = 0; i < 3; ++i)
	{
		m_pMins[i] = new wxSpinCtrlDouble(m_pHitboxInfo, wxID_ANY);
		m_pMins[i]->SetRange(-DBL_MAX, DBL_MAX);
		m_pMins[i]->SetDigits(6);
		m_pMins[i]->Bind(wxEVT_SPINCTRLDOUBLE, &CHitboxesPanel::OnMinsChanged, this);

		m_pMaxs[i] = new wxSpinCtrlDouble(m_pHitboxInfo, wxID_ANY);
		m_pMaxs[i]->SetRange(-DBL_MAX, DBL_MAX);
		m_pMaxs[i]->SetDigits(6);
		m_pMaxs[i]->Bind(wxEVT_SPINCTRLDOUBLE, &CHitboxesPanel::OnMaxsChanged, this);
	}

	//Layout
	auto pSizer = new wxGridBagSizer(5, 5);

	pSizer->Add(new wxStaticText(pElemParent, wxID_ANY, "Hitboxes:"), wxGBPosition(0, 0), wxDefaultSpan, wxEXPAND);

	pSizer->Add(m_pHitboxes, wxGBPosition(1, 0), wxGBSpan(1, 1), wxEXPAND);

	auto pInfoSizer = new wxGridBagSizer(5, 5);

	pInfoSizer->Add(new wxStaticText(m_pHitboxInfo, wxID_ANY, "Bone Name"), wxGBPosition(0, 0), wxDefaultSpan, wxEXPAND);
	pInfoSizer->Add(m_pBoneName, wxGBPosition(0, 1), wxDefaultSpan, wxEXPAND);
	pInfoSizer->Add(new wxStaticText(m_pHitboxInfo, wxID_ANY, "Bone"), wxGBPosition(1, 0), wxDefaultSpan, wxEXPAND);
	pInfoSizer->Add(m_pBone, wxGBPosition(1, 1), wxDefaultSpan, wxEXPAND);
	pInfoSizer->Add(new wxStaticText(m_pHitboxInfo, wxID_ANY, "Hitgroup"), wxGBPosition(2, 0), wxDefaultSpan, wxEXPAND);
	pInfoSizer->Add(m_pHitgroup, wxGBPosition(2, 1), wxDefaultSpan, wxEXPAND);

	pInfoSizer->Add(new wxStaticText(m_pHitboxInfo, wxID_ANY, "Minimum X"), wxGBPosition(0, 2), wxDefaultSpan, wxEXPAND);
	pInfoSizer->Add(new wxStaticText(m_pHitboxInfo, wxID_ANY, "Minimum Y"), wxGBPosition(1, 2), wxDefaultSpan, wxEXPAND);
	pInfoSizer->Add(new wxStaticText(m_pHitboxInfo, wxID_ANY, "Minimum Z"), wxGBPosition(2, 2), wxDefaultSpan, wxEXPAND);

	pInfoSizer->Add(new wxStaticText(m_pHitboxInfo, wxID_ANY, "Maximum X"), wxGBPosition(0, 4), wxDefaultSpan, wxEXPAND);
	pInfoSizer->Add(new wxStaticText(m_pHitboxInfo, wxID_ANY, "Maximum Y"), wxGBPosition(1, 4), wxDefaultSpan, wxEXPAND);
	pInfoSizer->Add(new wxStaticText(m_pHitboxInfo, wxID_ANY, "Maximum Z"), wxGBPosition(2, 4), wxDefaultSpan, wxEXPAND);

	for (int i = 0; i < 3; ++i)
	{
		pInfoSizer->Add(m_pMins[i], wxGBPosition(i, 3), wxDefaultSpan, wxEXPAND);
		pInfoSizer->Add(m_pMaxs[i], wxGBPosition(i, 5), wxDefaultSpan, wxEXPAND);
	}

	m_pHitboxInfo->SetSizer(pInfoSizer);

	pSizer->Add(m_pHitboxInfo, wxGBPosition(0, 1), wxGBSpan(3, 1), wxEXPAND | wxRESERVE_SPACE_EVEN_IF_HIDDEN);

	GetMainSizer()->Add(pSizer);
}

CHitboxesPanel::~CHitboxesPanel()
{
}

void CHitboxesPanel::InitializeUI()
{
	auto pEntity = m_pHLMV->GetState()->GetEntity();

	bool bSuccess = false;

	m_pHitboxes->Clear();

	if (pEntity)
	{
		if (auto pModel = pEntity->GetModel())
		{
			wxArrayString hitboxes;

			const auto pStudioHdr = pModel->GetStudioHeader();

			for (int i = 0; i < pStudioHdr->numhitboxes; ++i)
			{
				hitboxes.push_back(wxString::Format("Hitbox: %d", i + 1));
			}

			m_pHitboxes->Append(hitboxes);

			m_pBone->SetRange(0, pStudioHdr->numbones);

			//Only enable if there are hitboxes
			bSuccess = pStudioHdr->numhitboxes > 0;
		}
	}

	SetHitbox(0);

	this->Enable(bSuccess);

	if (!bSuccess)
	{
		m_pBoneName->SetLabelText("");
	}
}

void CHitboxesPanel::OnPostDraw(studiomdl::IStudioModelRenderer& renderer, const studiomdl::CModelRenderInfo& info)
{
	renderer.DrawSingleHitbox(m_pHitboxes->GetSelection());
}

void CHitboxesPanel::SetHitbox(int index)
{
	assert(index >= 0);

	auto pEntity = m_pHLMV->GetState()->GetEntity();

	if (!pEntity)
	{
		m_pHitboxInfo->Show(false);
		return;
	}

	auto pModel = pEntity->GetModel();

	const auto pStudioHdr = pModel->GetStudioHeader();

	if (pStudioHdr->numhitboxes <= 0)
	{
		m_pHitboxInfo->Show(false);
		return;
	}

	if (index < 0 || index >= pStudioHdr->numhitboxes)
		index = 0;

	const auto hitbox = pStudioHdr->GetHitBox(index);

	m_pBone->SetValue(hitbox->bone);
	m_pHitgroup->SetValue(hitbox->group);

	for (int i = 0; i < 3; ++i)
	{
		m_pMins[i]->SetValue(hitbox->bbmin[i]);
		m_pMaxs[i]->SetValue(hitbox->bbmax[i]);
	}

	m_pHitboxes->Select(index);

	m_pHitboxInfo->Show(true);

	//Set the bone name based on the bone this hitbox is tied to
	auto bone = pStudioHdr->GetBone(hitbox->bone);

	m_pBoneName->SetLabelText(bone->name);
}

void CHitboxesPanel::OnHitboxChanged(wxCommandEvent& event)
{
	SetHitbox(m_pHitboxes->GetSelection());
}

void CHitboxesPanel::OnBoneChanged(wxSpinEvent& event)
{
	auto pStudioHdr = m_pHLMV->GetState()->GetEntity()->GetModel()->GetStudioHeader();

	auto hitbox = pStudioHdr->GetHitBox(m_pHitboxes->GetSelection());

	hitbox->bone = m_pBone->GetValue();

	m_pHLMV->GetState()->modelChanged = true;
}

void CHitboxesPanel::OnHitgroupChanged(wxSpinEvent& event)
{
	auto pStudioHdr = m_pHLMV->GetState()->GetEntity()->GetModel()->GetStudioHeader();

	auto hitbox = pStudioHdr->GetHitBox(m_pHitboxes->GetSelection());

	hitbox->group = m_pHitgroup->GetValue();

	m_pHLMV->GetState()->modelChanged = true;
}

void CHitboxesPanel::OnMinsChanged(wxSpinDoubleEvent& event)
{
	auto pStudioHdr = m_pHLMV->GetState()->GetEntity()->GetModel()->GetStudioHeader();

	auto hitbox = pStudioHdr->GetHitBox(m_pHitboxes->GetSelection());

	for (int i = 0; i < 3; ++i)
	{
		hitbox->bbmin[i] = m_pMins[i]->GetValue();
	}

	m_pHLMV->GetState()->modelChanged = true;
}

void CHitboxesPanel::OnMaxsChanged(wxSpinDoubleEvent& event)
{
	auto pStudioHdr = m_pHLMV->GetState()->GetEntity()->GetModel()->GetStudioHeader();

	auto hitbox = pStudioHdr->GetHitBox(m_pHitboxes->GetSelection());

	for (int i = 0; i < 3; ++i)
	{
		hitbox->bbmax[i] = m_pMaxs[i]->GetValue();
	}

	m_pHLMV->GetState()->modelChanged = true;
}
}