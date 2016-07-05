#ifndef TOOLS_SPRITEVIEWER_CONTROLPANELS_CSPRITEDISPLAYPANEL_H
#define TOOLS_SPRITEVIEWER_CONTROLPANELS_CSPRITEDISPLAYPANEL_H

#include "CBaseControlPanel.h"

namespace sprview
{
class CSpriteDisplayPanel : public CBaseControlPanel
{
public:
	CSpriteDisplayPanel( wxWindow* pParent, CSpriteViewer* pHLSV );

	void InitializeUI() override;

protected:
	wxDECLARE_EVENT_TABLE();

	void OnTexFormatChanged( wxCommandEvent& event );

private:
	wxChoice* m_pTextureFormat;
};
}

#endif //TOOLS_SPRITEVIEWER_CONTROLPANELS_CSPRITEDISPLAYPANEL_H