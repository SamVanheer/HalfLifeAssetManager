#include "tools/shared/CBaseTool.h"

#include "CwxBaseApp.h"

CwxBaseApp::CwxBaseApp()
{
}

CwxBaseApp::~CwxBaseApp()
{
}

bool CwxBaseApp::OnInit()
{
	if( !Initialize() )
	{
		Shutdown();

		return false;
	}

	return true;
}

int CwxBaseApp::OnExit()
{
	Shutdown();

	return wxApp::OnExit();
}

bool CwxBaseApp::Initialize()
{
	if( !wxApp::OnInit() )
		return false;

	m_pTool = CreateTool();

	const wxString& szDisplayName = m_pTool->GetDisplayName();

	if( !szDisplayName.IsEmpty() )
		this->SetAppDisplayName( szDisplayName );

	if( !m_pTool->Initialize() )
	{
		wxMessageBox( wxString::Format( "Failed to initialize %s", this->GetAppDisplayName() ), wxMessageBoxCaptionStr, wxOK | wxCENTRE | wxICON_ERROR );
		return false;
	}

	if( !PostInitialize() )
		return false;

	return true;
}

void CwxBaseApp::Shutdown()
{
	PreShutdown();

	if( m_pTool )
	{
		m_pTool->Shutdown();
		delete m_pTool;
		m_pTool = nullptr;
	}
}