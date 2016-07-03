#ifndef SPRITEVIEWER_UI_CSPRITEVIEWER_H
#define SPRITEVIEWER_UI_CSPRITEVIEWER_H

#include "tools/shared/CBaseTool.h"

#include "../settings/CSpriteViewerSettings.h"
#include "../CSpriteViewerState.h"

namespace sprview
{
class CMainWindow;

/**
*	Facade class to access the entire sprite viewer program.
*/
class CSpriteViewer final : public tools::CBaseTool
{
public:
	/**
	*	Constructor.
	*/
	CSpriteViewer();

	/**
	*	Destructor.
	*/
	~CSpriteViewer();

protected:
	settings::CBaseSettings* CreateSettings() override final;

	bool InitializeTool() override final;

	bool PostInitialize() override final;

	void PreShutdown() override final;

	void RunFrame() override final;

	void OnExit( const bool bMainWndClosed ) override final;

public:
	/**
	*	Gets the settings object.
	*/
	const CSpriteViewerSettings* GetSettings() const { return static_cast<const CSpriteViewerSettings*>( CBaseTool::GetSettings() ); }

	/**
	*	@copydoc GetSettings() const
	*/
	CSpriteViewerSettings* GetSettings() { return static_cast<CSpriteViewerSettings*>( CBaseTool::GetSettings() ); }

	/**
	*	Gets the state object.
	*/
	const CSpriteViewerState* GetState() const { return m_pState; }

	/**
	*	@copydoc GetState() const
	*/
	CSpriteViewerState* GetState() { return m_pState; }

	/**
	*	Gets the main window.
	*/
	CMainWindow* GetMainWindow() { return m_pMainWindow; }

	/**
	*	Sets the main window.
	*/
	void SetMainWindow( CMainWindow* const pMainWindow );

	bool LoadSprite( const wxString& szFilename );

private:
	CSpriteViewerState* m_pState = nullptr;
	CMainWindow* m_pMainWindow = nullptr;

private:
	CSpriteViewer( const CSpriteViewer& ) = delete;
	CSpriteViewer& operator=( const CSpriteViewer& ) = delete;
};
}

#endif //SPRITEVIEWER_UI_CSPRITEVIEWER_H