#ifndef CONTROLPANELS_CFULLSCREENPANEL_H
#define CONTROLPANELS_CFULLSCREENPANEL_H

#include "CBaseControlPanel.h"

class CFullscreenPanel final : public CBaseControlPanel
{
public:
	CFullscreenPanel( wxWindow* pParent );
	~CFullscreenPanel();

	void ModelChanged( const StudioModel& model ) override;

private:
	wxComboBox* m_pResolution;

	wxButton* m_pGoFullscreen;

private:
	CFullscreenPanel( const CFullscreenPanel& ) = delete;
	CFullscreenPanel& operator=( const CFullscreenPanel& ) = delete;
};

#endif //CONTROLPANELS_CFULLSCREENPANEL_H