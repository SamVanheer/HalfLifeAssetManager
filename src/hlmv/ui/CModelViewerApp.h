#ifndef CMODELVIEWERAPP_H
#define CMODELVIEWERAPP_H

#include <vector>

#include "wxHLMV.h"

#include "ui/utility/CTimer.h"

class CMainWindow;

namespace hlmv
{
class CFullscreenWindow;
}

class CModelViewerApp final : public wxApp, public ITimerListener
{
public:
	typedef std::vector<wxVideoMode> VideoModes_t;

public:
	virtual bool OnInit() override;

	virtual int OnExit() override;

	virtual void OnInitCmdLine( wxCmdLineParser& parser ) override;

	virtual bool OnCmdLineParsed( wxCmdLineParser& parser ) override;

	void OnTimer( CTimer& timer ) override final;

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

	ITimerListener* m_pListener = nullptr;

	CMainWindow* m_pMainWindow = nullptr;
	hlmv::CFullscreenWindow* m_pFullscreenWindow = nullptr;
};

wxDECLARE_APP( CModelViewerApp );

#endif //CMODELVIEWERAPP_H