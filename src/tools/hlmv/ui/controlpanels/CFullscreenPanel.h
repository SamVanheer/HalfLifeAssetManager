#ifndef CONTROLPANELS_CFULLSCREENPANEL_H
#define CONTROLPANELS_CFULLSCREENPANEL_H

#include "CBaseControlPanel.h"

namespace hlmv
{
class CMainPanel;

class CFullscreenPanel final : public CBaseControlPanel
{
public:
	CFullscreenPanel( wxWindow* pParent, CModelViewerApp* const pHLMV, CMainPanel* const pMainPanel );
	~CFullscreenPanel();

protected:
	wxDECLARE_EVENT_TABLE();

private:
	void GoFullscreen( wxCommandEvent& event );

private:
	CMainPanel* const m_pMainPanel;

	wxButton* m_pGoFullscreen;

private:
	CFullscreenPanel( const CFullscreenPanel& ) = delete;
	CFullscreenPanel& operator=( const CFullscreenPanel& ) = delete;
};
}

#endif //CONTROLPANELS_CFULLSCREENPANEL_H