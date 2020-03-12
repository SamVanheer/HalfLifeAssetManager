#pragma once

#include <vector>

#include <glm/vec3.hpp>

#include <wx/spinctrl.h>

#include "engine/shared/studiomodel/studio.h"

#include "CBaseControlPanel.h"

class wxButton;
class wxTextCtrl;

namespace hlmv
{
/**
*	@brief Allows general model data to be modified (origin, scale, etc)
*/
class CModelDataPanel final : public CBaseControlPanel
{
private:
	struct RootBoneData
	{
		mstudiobone_t* Bone;
		glm::vec3 OriginalRootBonePosition;
	};

public:
	CModelDataPanel(wxWindow* pParent, CModelViewerApp* const pHLMV);

	~CModelDataPanel();

	void InitializeUI() override final;

	void OnPostDraw(studiomdl::IStudioModelRenderer& renderer, const studiomdl::CModelRenderInfo& info) override final;

private:
	void UpdateOrigin();

	void OnOriginChanged(wxSpinDoubleEvent& event);

	void OnSetOrigin(wxCommandEvent& event);

	void OnScaleMesh(wxCommandEvent& event);

	void OnScaleBones(wxCommandEvent& event);

private:
	wxSpinCtrlDouble* m_pOrigin[3];

	wxButton* m_pSetOrigin;

	std::vector<RootBoneData> m_RootBonePositions;

	wxSpinCtrlDouble* m_pMeshScale;
	wxButton* m_pMeshScaleButton;

	wxSpinCtrlDouble* m_pBonesScale;
	wxButton* m_pBonesScaleButton;

private:
	CModelDataPanel(const CModelDataPanel&) = delete;
	CModelDataPanel& operator=(const CModelDataPanel&) = delete;
};
}
