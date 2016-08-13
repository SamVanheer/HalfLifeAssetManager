#ifndef SPRITEVIEWER_UI_CMAINPANEL_H
#define SPRITEVIEWER_UI_CMAINPANEL_H

#include "wxSpriteViewer.h"

#include <wx/notebook.h>

#include "shared/Utility.h"

#include "controlpanels/CBaseControlPanel.h"

#include "C3DView.h"

namespace sprview
{
class CSpriteViewerApp;

class CSpriteListBox;

class CSpriteDisplayPanel;
class CSpriteInfoPanel;

class CMainPanel final : public wxPanel, public I3DViewListener
{
public:
	CMainPanel( wxWindow* pParent, CSpriteViewerApp* const pSpriteViewer );
	~CMainPanel();

	const CSpriteViewerApp* GetSpriteViewer() const { return m_pSpriteViewer; }
	CSpriteViewerApp* GetSpriteViewer() { return m_pSpriteViewer; }

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

	/*
	*	Helper function to inform all control panels.
	*	Important: pass by reference.
	*/
	template<typename FUNC, typename... ARGS>
	void ForEachPanel( FUNC func, ARGS&... args )
	{
		const size_t uiPageCount = m_pControlPanels->GetPageCount();

		for( size_t uiIndex = 0; uiIndex < uiPageCount; ++uiIndex )
		{
			CBaseControlPanel* const pPanel = static_cast<CBaseControlPanel*>( m_pControlPanels->GetPage( uiIndex ) );

			( pPanel->*func )( args... );
		}
	}

private:
	CSpriteViewerApp* const m_pSpriteViewer;

	C3DView* m_p3DView;

	CSpriteListBox* m_pFramesList;

	wxPanel* m_pControlPanel;

	wxPanel* m_pMainControlBar;

	long long m_iLastFPSUpdate = GetCurrentTick();
	unsigned int m_uiCurrentFPS = 0;

	wxStaticText* m_pFPS;

	CSpriteDisplayPanel* m_pSpriteDisplayPanel;
	CSpriteInfoPanel* m_pSpriteInfoPanel;

	wxNotebook* m_pControlPanels;

private:
	CMainPanel( const CMainPanel& ) = delete;
	CMainPanel& operator=( const CMainPanel& ) = delete;
};
}

#endif //SPRITEVIEWER_UI_CMAINPANEL_H