#ifndef CMODELVIEWERAPP_H
#define CMODELVIEWERAPP_H

#include <vector>

#include "wxHLMV.h"

class CTimer;
class CMainWindow;

namespace hlmv
{
class CFullscreenWindow;
}

class CModelViewerApp final : public wxApp
{
public:
	typedef std::vector<wxVideoMode> VideoModes_t;

public:
	virtual bool OnInit() override;

	virtual int OnExit() override;

	virtual void OnInitCmdLine( wxCmdLineParser& parser ) override;

	virtual bool OnCmdLineParsed( wxCmdLineParser& parser ) override;

	CTimer* GetTimer() { return m_pTimer; }

	hlmv::CFullscreenWindow* GetFullscreenWindow() { return m_pFullscreenWindow; }

	void SetFullscreenWindow( hlmv::CFullscreenWindow* pWindow );

	void ExitApp( const bool bMainWndClosed = false );

private:
	bool Initialize();

	void Shutdown();

private:
	wxString m_szModel;						//Model to load on startup, if any.

	CTimer* m_pTimer = nullptr;

	CMainWindow* m_pMainWindow = nullptr;
	hlmv::CFullscreenWindow* m_pFullscreenWindow = nullptr;
};

wxDECLARE_APP( CModelViewerApp );

#endif //CMODELVIEWERAPP_H