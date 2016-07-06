#ifndef TOOLS_SPRITEVIEWER_CONTROLPANELS_CSPRITEDISPLAYPANEL_H
#define TOOLS_SPRITEVIEWER_CONTROLPANELS_CSPRITEDISPLAYPANEL_H

#include "CBaseControlPanel.h"

namespace sprview
{
class CSpriteDisplayPanel : public CBaseControlPanel
{
private:
	static const int SCALE_MIN = 1;

	static const int SCALE_MAX = 20;

	static const int SCALE_DEFAULT = 4;

public:
	CSpriteDisplayPanel( wxWindow* pParent, CSpriteViewer* pHLSV );

	void InitializeUI() override;

protected:
	wxDECLARE_EVENT_TABLE();

	void OnTypeChanged( wxCommandEvent& event );

	void OnTexFormatChanged( wxCommandEvent& event );

	void OnScaleChanged( wxCommandEvent& event );

private:
	wxChoice* m_pType;
	wxChoice* m_pTextureFormat;

	wxSlider* m_pScale;
};
}

#endif //TOOLS_SPRITEVIEWER_CONTROLPANELS_CSPRITEDISPLAYPANEL_H