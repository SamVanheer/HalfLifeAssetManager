#ifndef SPRITEVIEWER_UI_CMAINPANEL_H
#define SPRITEVIEWER_UI_CMAINPANEL_H

#include "wxSpriteViewer.h"

#include "shared/Utility.h"

#include "C3DView.h"

class wxNotebook;
class wxBookCtrlEvent;

namespace sprview
{
class CSpriteViewer;

class CMainPanel final : public wxPanel, public I3DViewListener
{
public:
	CMainPanel( wxWindow* pParent, CSpriteViewer* const pSpriteViewer );
	~CMainPanel();

	const CSpriteViewer* GetSpriteViewer() const { return m_pSpriteViewer; }
	CSpriteViewer* GetSpriteViewer() { return m_pSpriteViewer; }

	void RunFrame();

	void Draw3D( const wxSize& size ) override final;

	bool LoadSprite( const wxString& szFilename );

	void FreeSprite();

	void InitializeUI();

	void PageChanged( wxBookCtrlEvent& event );

	bool LoadBackgroundTexture( const wxString& szFilename );
	void UnloadBackgroundTexture();

	void TakeScreenshot();

protected:
	wxDECLARE_EVENT_TABLE();

private:
	CSpriteViewer* const m_pSpriteViewer;

	C3DView* m_p3DView;

	wxPanel* m_pControlPanel;

	wxPanel* m_pMainControlBar;

	long long m_iLastFPSUpdate = GetCurrentTick();
	unsigned int m_uiCurrentFPS = 0;

	wxStaticText* m_pFPS;

	wxNotebook* m_pControlPanels;

private:
	CMainPanel( const CMainPanel& ) = delete;
	CMainPanel& operator=( const CMainPanel& ) = delete;
};
}

#endif //SPRITEVIEWER_UI_CMAINPANEL_H