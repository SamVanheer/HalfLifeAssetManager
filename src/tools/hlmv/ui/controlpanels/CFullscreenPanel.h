#ifndef CONTROLPANELS_CFULLSCREENPANEL_H
#define CONTROLPANELS_CFULLSCREENPANEL_H

#include "CBaseControlPanel.h"

class wxNotebook;

namespace hlmv
{
class CFullscreenPanel final : public CBaseControlPanel
{
public:
	CFullscreenPanel( wxWindow* pParent, CHLMV* const pHLMV, wxNotebook* const pControlPanels );
	~CFullscreenPanel();

protected:
	wxDECLARE_EVENT_TABLE();

private:
	void GoFullscreen( wxCommandEvent& event );

private:
	wxNotebook* const m_pControlPanels;

	wxButton* m_pGoFullscreen;

private:
	CFullscreenPanel( const CFullscreenPanel& ) = delete;
	CFullscreenPanel& operator=( const CFullscreenPanel& ) = delete;
};
}

#endif //CONTROLPANELS_CFULLSCREENPANEL_H