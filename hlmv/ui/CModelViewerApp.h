#ifndef CMODELVIEWERAPP_H
#define CMODELVIEWERAPP_H

#include <vector>

#include "wxHLMV.h"

class CModelViewerApp final : public wxApp
{
public:
	typedef std::vector<wxVideoMode> VideoModes_t;

public:
	virtual bool OnInit() override;

	virtual int OnExit() override;

	virtual void OnInitCmdLine( wxCmdLineParser& parser ) override;

	virtual bool OnCmdLineParsed( wxCmdLineParser& parser ) override;

private:
	bool Initialize();

	void Shutdown();

private:
	wxString m_szModel;						//Model to load on startup, if any.
};

wxDECLARE_APP( CModelViewerApp );

#endif //CMODELVIEWERAPP_H