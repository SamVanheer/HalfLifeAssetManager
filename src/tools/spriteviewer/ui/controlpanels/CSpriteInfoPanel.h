#ifndef TOOLS_SPRITEVIEWER_CONTROLPANELS_CSPRITEINFOPANEL_H
#define TOOLS_SPRITEVIEWER_CONTROLPANELS_CSPRITEINFOPANEL_H

#include "CBaseControlPanel.h"

namespace sprview
{
class CSpriteInfoPanel : public CBaseControlPanel
{
public:
	CSpriteInfoPanel( wxWindow* pParent, CSpriteViewerApp* pHLSV );

	void InitializeUI() override;

private:
	wxPanel* m_pInfoPanel;

	wxStaticText* m_pType;
	wxStaticText* m_pTextureFormat;
	wxStaticText* m_pMaxWidth;
	wxStaticText* m_pMaxHeight;
	wxStaticText* m_pNumFrames;
};
}

#endif //TOOLS_SPRITEVIEWER_CONTROLPANELS_CSPRITEINFOPANEL_H