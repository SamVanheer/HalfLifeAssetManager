#ifndef CONTROLPANELS_CFULLSCREENPANEL_H
#define CONTROLPANELS_CFULLSCREENPANEL_H

#include "CBaseControlPanel.h"

class CFullscreenPanel final : public CBaseControlPanel
{
public:
	CFullscreenPanel( wxWindow* pParent, hlmv::CHLMVState* const pSettings );
	~CFullscreenPanel();

protected:
	wxDECLARE_EVENT_TABLE();

private:
	void GoFullscreen( wxCommandEvent& event );

private:
	wxButton* m_pGoFullscreen;

private:
	CFullscreenPanel( const CFullscreenPanel& ) = delete;
	CFullscreenPanel& operator=( const CFullscreenPanel& ) = delete;
};

#endif //CONTROLPANELS_CFULLSCREENPANEL_H