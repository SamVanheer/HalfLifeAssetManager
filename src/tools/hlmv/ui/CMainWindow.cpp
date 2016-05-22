#include <wx/filename.h>

#include "ui/wx/CwxOpenGL.h"

#include "ui/wx/shared/CMessagesWindow.h"
#include "ui/wx/shared/CCmdLineConfigDialog.h"
#include "ui/wx/shared/CProcessDialog.h"
#include "ui/wx/utility/wxUtil.h"

#include "CHLMV.h"

#include "options/COptionsDialog.h"

#include "tools/shared/Credits.h"

#include "CMainPanel.h"

#include "CMainWindow.h"

namespace hlmv
{
wxBEGIN_EVENT_TABLE( CMainWindow, ui::CwxBaseFrame )
	EVT_MENU( wxID_MAINWND_LOADMODEL, CMainWindow::LoadModel )
	EVT_MENU( wxID_MAINWND_LOADBACKGROUND, CMainWindow::LoadBackgroundTexture )
	EVT_MENU( wxID_MAINWND_LOADGROUND, CMainWindow::LoadGroundTexture )
	EVT_MENU( wxID_MAINWND_UNLOADGROUND, CMainWindow::UnloadGroundTexture )
	EVT_MENU( wxID_MAINWND_SAVEMODEL, CMainWindow::SaveModel )
	EVT_MENU( wxID_EXIT, CMainWindow::OnExit )
	EVT_MENU( wxID_MAINWND_CENTERVIEW, CMainWindow::CenterView )
	EVT_MENU( wxID_MAINWND_SAVEVIEW, CMainWindow::SaveView )
	EVT_MENU( wxID_MAINWND_RESTOREVIEW, CMainWindow::RestoreView )
	EVT_MENU( wxID_MAINWND_TAKESCREENSHOT, CMainWindow::TakeScreenshot )
	EVT_MENU( wxID_MAINWND_DUMPMODELINFO, CMainWindow::DumpModelInfo )
	EVT_MENU( wxID_MAINWND_TOGGLEMESSAGES, CMainWindow::ShowMessagesWindow )
	EVT_MENU( wxID_MAINWND_COMPILEMODEL, CMainWindow::OnCompileModel )
	EVT_MENU( wxID_MAINWND_DECOMPILEMODEL, CMainWindow::OnDecompileModel )
	EVT_MENU( wxID_MAINWND_EDITQC, CMainWindow::OnEditQC )
	EVT_MENU( wxID_MAINWND_OPTIONS, CMainWindow::OpenOptionsDialog )
	EVT_MENU( wxID_ABOUT, CMainWindow::OnAbout )
wxEND_EVENT_TABLE()

CMainWindow::CMainWindow( CHLMV* const pHLMV )
	: CwxBaseFrame( nullptr, wxID_ANY, HLMV_TITLE, wxDefaultPosition, wxSize( 600, 400 ) )
	, m_pHLMV( pHLMV )
	, m_RecentFiles( pHLMV->GetSettings()->GetRecentFiles() )
{
	pHLMV->SetMainWindow( this );

	SetIcon( m_pHLMV->GetToolIcon() );

	this->SetDropTarget( new CModelDropTarget( this ) );

	wxMenu* menuFile = new wxMenu;

	menuFile->Append( wxID_MAINWND_LOADMODEL, "&Load Model...",
					  "Load a model" );

	menuFile->AppendSeparator();

	menuFile->Append( wxID_MAINWND_LOADBACKGROUND, "Load Background Texture..." );

	menuFile->Append( wxID_MAINWND_LOADGROUND, "Load Ground Texture..." );

	menuFile->AppendSeparator();

	menuFile->Append( wxID_MAINWND_UNLOADGROUND, "Unload Ground Texture" );

	menuFile->AppendSeparator();

	menuFile->Append( wxID_MAINWND_SAVEMODEL, "Save Model As...", "Save model to disk" );

	menuFile->AppendSeparator();

	wxMenu* pRecentFiles = new wxMenu;

	m_RecentFiles.AddMenuItems( pRecentFiles );

	//Handle the range of items.
	this->Bind( wxEVT_MENU, &CMainWindow::OpenRecentFile, this, m_RecentFiles.GetBaseID(), m_RecentFiles.GetLastID() );

	menuFile->AppendSubMenu( pRecentFiles, "Recent Files" );

	menuFile->AppendSeparator();

	menuFile->Append( wxID_EXIT );

	wxMenu* pMenuView = new wxMenu;

	pMenuView->Append( wxID_MAINWND_CENTERVIEW, "Center View" );
	pMenuView->Append( wxID_MAINWND_SAVEVIEW, "Save View" );
	pMenuView->Append( wxID_MAINWND_RESTOREVIEW, "Restore View" );

	pMenuView->AppendSeparator();

	pMenuView->Append( wxID_MAINWND_TAKESCREENSHOT, "Take Screenshot" );

	pMenuView->Append( wxID_MAINWND_DUMPMODELINFO, "Dump Model Info" );

	wxMenu* pMenuTools = new wxMenu;

	pMenuTools->Append( wxID_MAINWND_TOGGLEMESSAGES, "Show Messages Window", "Shows or hides the messages window", true );

	pMenuTools->AppendSeparator();

	pMenuTools->Append( wxID_MAINWND_COMPILEMODEL, "Compile Model..." );
	pMenuTools->Append( wxID_MAINWND_DECOMPILEMODEL, "Decompile Model..." );
	pMenuTools->Append( wxID_MAINWND_EDITQC, "Edit QC File..." );

	pMenuTools->AppendSeparator();

	pMenuTools->Append( wxID_MAINWND_OPTIONS, "Options" );

	wxMenu* menuHelp = new wxMenu;

	menuHelp->Append( wxID_ABOUT );

	wxMenuBar* menuBar = new wxMenuBar;

	menuBar->Append( menuFile, "&File" );
	menuBar->Append( pMenuView, "&View" );
	menuBar->Append( pMenuTools, "&Tools" );
	menuBar->Append( menuHelp, "&Help" );

	SetMenuBar( menuBar );

	/*
	//Don't need a status bar right now.
	CreateStatusBar();
	SetStatusText( "" );
	*/

	m_pMainPanel = new CMainPanel( this, m_pHLMV );

	Maximize( true );

	if( ui::CMessagesWindow* pWindow = m_pHLMV->GetMessagesWindow() )
	{
		pWindow->Bind( wxEVT_CLOSE_WINDOW, &CMainWindow::OnMessagesWindowClosed, this );
	}
}

CMainWindow::~CMainWindow()
{
	//Clear the studio model here, while the context is still valid.
	m_pMainPanel->FreeModel();

	m_pHLMV->Exit( true );
}

void CMainWindow::RunFrame()
{
	m_pMainPanel->RunFrame();
}

bool CMainWindow::LoadModel( const wxString& szFilename )
{
	wxFileName file( szFilename );

	file.MakeAbsolute();

	const wxString szAbsFilename = file.GetFullPath();

	if( !file.Exists() )
	{
		wxMessageBox( wxString::Format( "The file \"%s\" does not exist.", szAbsFilename ) );

		m_pHLMV->GetSettings()->GetRecentFiles()->Remove( std::string( szFilename.c_str() ) );

		m_RecentFiles.Refresh();

		return false;
	}

	const bool bSuccess = m_pMainPanel->LoadModel( szAbsFilename );

	if( bSuccess )
	{
		const wxCStrData data = szAbsFilename.c_str();
		const char* const pszAbsFilename = data.AsChar();

		this->SetTitleContent( pszAbsFilename );

		m_pHLMV->GetSettings()->GetRecentFiles()->Add( pszAbsFilename );

		m_RecentFiles.Refresh();

		Message( "Loaded model \"%s\"\n", pszAbsFilename );
	}
	else
		this->ClearTitleContent();

	return bSuccess;
}

bool CMainWindow::PromptLoadModel()
{
	wxFileDialog dlg( this, wxFileSelectorPromptStr, wxEmptyString, wxEmptyString, "Half-Life Models (*.mdl)|*.mdl" );

	if( dlg.ShowModal() == wxID_CANCEL )
		return false;

	return LoadModel( dlg.GetPath() );
}

bool CMainWindow::SaveModel( const wxString& szFilename )
{
	if( !m_pHLMV->GetState()->GetEntity() )
		return false;

	auto pModel = m_pHLMV->GetState()->GetEntity()->GetModel();

	const bool bSuccess = studiomdl::SaveStudioModel( szFilename.c_str(), pModel );

	if( !bSuccess )
	{
		wxMessageBox( wxString::Format( "An error occurred while saving the model \"%s\"", szFilename.c_str() ) );
	}

	return bSuccess;
}

bool CMainWindow::PromptSaveModel()
{
	if( m_pHLMV->GetState()->GetEntity() == nullptr )
	{
		wxMessageBox( "No model to save!" );
		return false;
	}

	wxFileDialog dlg( this, wxFileSelectorPromptStr, wxEmptyString, wxEmptyString, "Half-Life Models (*.mdl)|*.mdl", wxFD_SAVE | wxFD_OVERWRITE_PROMPT );

	if( dlg.ShowModal() == wxID_CANCEL )
		return false;

	return SaveModel( dlg.GetPath() );
}

bool CMainWindow::LoadBackgroundTexture( const wxString& szFilename )
{
	return m_pMainPanel->LoadBackgroundTexture( szFilename );
}

bool CMainWindow::PromptLoadBackgroundTexture()
{
	wxFileDialog dlg( this, wxFileSelectorPromptStr, wxEmptyString, wxEmptyString, "*.*" );

	if( dlg.ShowModal() == wxID_CANCEL )
		return false;

	return LoadBackgroundTexture( dlg.GetPath() );
}

void CMainWindow::UnloadBackgroundTexture()
{
	m_pMainPanel->UnloadBackgroundTexture();
}

bool CMainWindow::LoadGroundTexture( const wxString& szFilename )
{
	return m_pMainPanel->LoadGroundTexture( szFilename );
}

bool CMainWindow::PromptLoadGroundTexture()
{
	wxFileDialog dlg( this, wxFileSelectorPromptStr, wxEmptyString, wxEmptyString, "*.*" );

	if( dlg.ShowModal() == wxID_CANCEL )
		return false;

	return LoadGroundTexture( dlg.GetPath() );
}

void CMainWindow::UnloadGroundTexture()
{
	m_pMainPanel->UnloadGroundTexture();
}

void CMainWindow::SaveUVMap( const wxString& szFilename, const int iTexture )
{
	m_pMainPanel->SaveUVMap( szFilename, iTexture );
}

void CMainWindow::CenterView()
{
	m_pHLMV->GetState()->CenterView();
}

void CMainWindow::SaveView()
{
	m_pHLMV->GetState()->SaveView();
}

void CMainWindow::RestoreView()
{
	m_pHLMV->GetState()->RestoreView();
}

void CMainWindow::TakeScreenshot()
{
	m_pMainPanel->TakeScreenshot();
}

void CMainWindow::DumpModelInfo()
{
	if( !m_pHLMV->GetState()->GetEntity() )
	{
		wxMessageBox( "No model loaded!" );
		return;
	}

	if( m_pHLMV->GetState()->DumpModelInfo( HLMV_DUMP_MODEL_INFO_FILE ) )
	{
		//Launch the default text editor.
		wx::LaunchDefaultTextEditor( HLMV_DUMP_MODEL_INFO_FILE );
	}
	else
	{
		wxMessageBox( "An error occurred while dumping model info" );
	}
}

void CMainWindow::LoadModel( wxCommandEvent& event )
{
	PromptLoadModel();
}

void CMainWindow::LoadBackgroundTexture( wxCommandEvent& event )
{
	PromptLoadBackgroundTexture();
}

void CMainWindow::LoadGroundTexture( wxCommandEvent& event )
{
	PromptLoadGroundTexture();
}

void CMainWindow::UnloadGroundTexture( wxCommandEvent& event )
{
	UnloadGroundTexture();
}

void CMainWindow::SaveModel( wxCommandEvent& event )
{
	PromptSaveModel();
}

void CMainWindow::OpenRecentFile( wxCommandEvent& event )
{
	wxString szFilename;

	if( !m_RecentFiles.OnOpenRecentFile( event, szFilename ) )
	{
		wxMessageBox( "An error occurred while trying to open a recent file" );
		return;
	}

	LoadModel( szFilename );
}

void CMainWindow::OnExit( wxCommandEvent& event )
{
	Close( true );
}

void CMainWindow::CenterView( wxCommandEvent& event )
{
	CenterView();
}

void CMainWindow::SaveView( wxCommandEvent& event )
{
	SaveView();
}

void CMainWindow::RestoreView( wxCommandEvent& event )
{
	RestoreView();
}

void CMainWindow::TakeScreenshot( wxCommandEvent& event )
{
	TakeScreenshot();
}

void CMainWindow::DumpModelInfo( wxCommandEvent& event )
{
	DumpModelInfo();
}

void CMainWindow::ShowMessagesWindow( wxCommandEvent& event )
{
	m_pHLMV->ShowMessagesWindow( event.IsChecked() );
}

void CMainWindow::OnCompileModel( wxCommandEvent& event )
{
	const wxString szStudioMdl = m_pHLMV->GetSettings()->GetStudioMdl().CStr();

	wxFileName fileName( szStudioMdl );

	if( !fileName.Exists() )
	{
		wxMessageBox( "Couldn't find studiomdl compiler!", wxMessageBoxCaptionStr, wxOK | wxCENTRE | wxICON_ERROR );
		return;
	}

	wxFileDialog dlg( this, "Select QC file", wxEmptyString, wxEmptyString, "QC files (*.qc)|*.qc" );

	if( dlg.ShowModal() == wxID_CANCEL )
		return;

	const wxString szPath = dlg.GetPath();

	ui::CCmdLineConfigDialog commandLineDlg( this, wxID_ANY, "Configure StudioMdl" );

	if( commandLineDlg.ShowModal() == wxID_CANCEL )
		return;

	auto parameters = commandLineDlg.GetParameters();
	parameters.emplace_back( std::make_pair( szPath, "" ) );

	ui::CProcessDialog processDlg( this, wxID_ANY, "StudioMdl Compiler" );

	processDlg.SetCommand( wx::FormatCommandLine( szStudioMdl, parameters ) );

	wxFileName cwd( szPath );

	wxExecuteEnv* pEnv = new wxExecuteEnv;

	pEnv->cwd = cwd.GetPath();

	processDlg.SetExecuteEnv( pEnv );

	const int iResult = processDlg.ShowModal();

	if( iResult != ui::CProcessDialog::SUCCESS )
	{
		wxMessageBox( processDlg.GetErrorString( iResult ), wxMessageBoxCaptionStr, wxOK | wxCENTRE | wxICON_ERROR );
	}
	else
	{
		Message( "Compiled QC file \"%s\"\n", szPath.c_str().AsChar() );
	}
}

void CMainWindow::OnDecompileModel( wxCommandEvent& event )
{
	const wxString szMdlDec = m_pHLMV->GetSettings()->GetMdlDec().CStr();

	wxFileName fileName( szMdlDec );

	if( !fileName.Exists() )
	{
		wxMessageBox( "Couldn't find mdldec decompiler!", wxMessageBoxCaptionStr, wxOK | wxCENTRE | wxICON_ERROR );
		return;
	}

	wxFileDialog dlg( this, "Select MDL file", wxEmptyString, wxEmptyString, "Half-Life MDL files (*.mdl)|*.mdl" );

	if( dlg.ShowModal() == wxID_CANCEL )
		return;

	const wxString szPath = dlg.GetPath();

	ui::CCmdLineConfigDialog commandLineDlg( this, wxID_ANY, "Configure MdlDec" );

	if( commandLineDlg.ShowModal() == wxID_CANCEL )
		return;

	auto parameters = commandLineDlg.GetParameters();
	parameters.emplace_back( std::make_pair( szPath, "" ) );

	ui::CProcessDialog processDlg( this, wxID_ANY, "MdlDec Decompiler" );

	processDlg.SetCommand( wx::FormatCommandLine( szMdlDec, parameters ) );

	wxFileName cwd( szPath );

	wxExecuteEnv* pEnv = new wxExecuteEnv;

	pEnv->cwd = cwd.GetPath();

	processDlg.SetExecuteEnv( pEnv );

	const int iResult = processDlg.ShowModal();

	if( iResult != ui::CProcessDialog::SUCCESS )
	{
		wxMessageBox( processDlg.GetErrorString( iResult ), wxMessageBoxCaptionStr, wxOK | wxCENTRE | wxICON_ERROR );
	}
	else
	{
		Message( "Decompiled MDL file \"%s\"\n", szPath.c_str().AsChar() );
	}
}

void CMainWindow::OnEditQC( wxCommandEvent& event )
{
	wxFileDialog dlg( this, "Select QC file", wxEmptyString, wxEmptyString, "QC files (*.qc)|*.qc" );

	if( dlg.ShowModal() == wxID_CANCEL )
		return;

	wx::LaunchDefaultTextEditor( dlg.GetPath() );
}

void CMainWindow::OpenOptionsDialog( wxCommandEvent& event )
{
	COptionsDialog dlg( this, m_pHLMV->GetSettings() );

	if( dlg.ShowModal() == wxID_CANCEL )
		return;
}

void CMainWindow::OnAbout( wxCommandEvent& event )
{
	//This should not be modified unless necessary (e.g. to change a date, or to correct information).

	//No tab support in message boxes, so manually align it
	wxMessageBox( wxString::Format( 
					"Half-Life Model Viewer 2.0\n"
					"2016 Sam \"Solokiller\" Vanheer\n\n"
					"Email:    sam.vanheer@outlook.com\n\n"
					"Based on Jed's Half-Life Model Viewer v1.3 © 2004 Neil \'Jed\' Jedrzejewski\n"
					"Email:    jed@wunderboy.org\n"
					"Web:      http://www.wunderboy.org/\n\n"
					"Also based on Half-Life Model Viewer v1.25 © 2002 Mete Ciragan\n"
					"Email:    mete@swissquake.ch\n"
					"Web:      http://www.milkshape3d.com/\n\n"
					"%s", 
					tools::GetSharedCredits()
					),
					"About Half-Life Model Viewer", wxOK | wxICON_INFORMATION );
}

void CMainWindow::OnMessagesWindowClosed( wxCloseEvent& event )
{
	//Whenever the messages window is closed by clicking the close button, uncheck this.
	wxMenuItem* pItem = GetMenuBar()->FindItem( wxID_MAINWND_TOGGLEMESSAGES );

	if( pItem )
	{
		pItem->Check( false );
	}

	event.Skip();
}

bool CMainWindow::OnDropFiles( wxCoord x, wxCoord y, const wxArrayString& filenames )
{
	if( filenames.empty() )
		return true;

	//Open the first model.
	for( size_t uiIndex = 0; uiIndex < filenames.size(); ++uiIndex )
	{
		if( filenames[ uiIndex ].EndsWith( ".mdl" ) )
		{
			LoadModel( filenames[ uiIndex ] );

			return true;
		}
	}

	const wxString szMessage = filenames.size() == 1 ? "Dropped file is not a model" : "Dropped files did not contain a model";

	wxMessageBox( szMessage );

	return false;
}

bool CModelDropTarget::OnDropFiles( wxCoord x, wxCoord y, const wxArrayString& filenames )
{
	return m_pMainWindow->OnDropFiles( x, y, filenames );
}
}