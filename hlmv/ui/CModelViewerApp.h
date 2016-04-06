#ifndef CMODELVIEWERAPP_H
#define CMODELVIEWERAPP_H

#include <vector>

#include "wxHLMV.h"

#include <wx/vidmode.h>

class wxDisplay;

class CModelViewerApp final : public wxApp
{
public:
	typedef std::vector<wxVideoMode> VideoModes_t;

public:
	static CModelViewerApp& GetApp() { return *m_pInstance; }

	virtual bool OnInit() override;

	virtual int OnExit() override;

	virtual bool OnCmdLineParsed( wxCmdLineParser& parser ) override;

	virtual void OnInitCmdLine( wxCmdLineParser& parser ) override;

	const VideoModes_t& GetVideoModes() const { return m_VideoModes; }

private:
	static CModelViewerApp* m_pInstance;

	wxDisplay* m_pPrimaryDisplay;

	VideoModes_t m_VideoModes;

	wxString m_szModel;						//Model to load on startup, if any.
};

#endif //CMODELVIEWERAPP_H