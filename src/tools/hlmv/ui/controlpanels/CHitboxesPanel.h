#ifndef HLMV_UI_CONTROLPANELS_CHITBOXESPANEL_H
#define HLMV_UI_CONTROLPANELS_CHITBOXESPANEL_H

#include <wx/spinctrl.h>

#include "engine/shared/studiomodel/studio.h"

#include "CBaseControlPanel.h"

namespace hlmv
{
/**
*	@brief Shows the list of hitboxes, allows modification of values
*/
class CHitboxesPanel final : public CBaseControlPanel
{
public:
	CHitboxesPanel(wxWindow* pParent, CModelViewerApp* const pHLMV);

	~CHitboxesPanel();

	void InitializeUI() override final;

	void OnPostDraw(studiomdl::IStudioModelRenderer& renderer, const studiomdl::CModelRenderInfo& info) override final;

	void SetHitbox(int index);

private:
	void OnHitboxChanged(wxCommandEvent& event);

	void OnBoneChanged(wxSpinEvent& event);

	void OnHitgroupChanged(wxSpinEvent& event);

	void OnMinsChanged(wxSpinDoubleEvent& event);

	void OnMaxsChanged(wxSpinDoubleEvent& event);

private:
	wxChoice* m_pHitboxes;

	wxPanel* m_pHitboxInfo;

	wxStaticText* m_pBoneName;

	wxSpinCtrl* m_pBone;
	wxSpinCtrl* m_pHitgroup;

	wxSpinCtrlDouble* m_pMins[3];
	wxSpinCtrlDouble* m_pMaxs[3];

private:
	CHitboxesPanel(const CHitboxesPanel&) = delete;
	CHitboxesPanel& operator=(const CHitboxesPanel&) = delete;
};
}

#endif //HLMV_UI_CONTROLPANELS_CHITBOXESPANEL_H
