#ifndef CMODELVIEWERAPP_H
#define CMODELVIEWERAPP_H

#include "wxHLMV.h"

#include "ui/CwxBaseApp.h"

#include "hlmv/ui/CHLMV.h"

class CModelViewerApp : public CwxBaseApp
{
public:
	virtual void OnInitCmdLine( wxCmdLineParser& parser ) override;

	virtual bool OnCmdLineParsed( wxCmdLineParser& parser ) override;

	hlmv::CHLMV* GetTool() { return static_cast<hlmv::CHLMV*>( CwxBaseApp::GetTool() ); }

private:
	virtual hlmv::CHLMV* CreateTool() override { return new hlmv::CHLMV(); }

	virtual bool PostInitialize() override;

private:
	wxString m_szModel;		//Model to load on startup, if any.
};

wxDECLARE_APP( CModelViewerApp );

#endif //CMODELVIEWERAPP_H