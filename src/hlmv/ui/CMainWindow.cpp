#include <wx/filename.h>
#include <wx/mimetype.h>

#include "ui/CwxOpenGL.h"

#include "ui/shared/CMessagesWindow.h"

#include "hlmv/ui/CHLMV.h"

#include "options/COptionsDialog.h"

#include "CMainPanel.h"

#include "CMainWindow.h"

namespace hlmv
{
wxBEGIN_EVENT_TABLE( CMainWindow, wxFrame )
	EVT_MENU( wxID_MAINWND_LOADMODEL, CMainWindow::LoadModel )
	EVT_MENU( wxID_MAINWND_LOADBACKGROUND, CMainWindow::LoadBackgroundTexture )
	EVT_MENU( wxID_MAINWND_LOADGROUND, CMainWindow::LoadGroundTexture )
	EVT_MENU( wxID_MAINWND_UNLOADGROUND, CMainWindow::UnloadGroundTexture )
	EVT_MENU( wxID_MAINWND_SAVEMODEL, CMainWindow::SaveModel )
	EVT_MENU( wxID_MAINWND_RECENTFILE1, CMainWindow::OpenRecentFile )
	EVT_MENU( wxID_MAINWND_RECENTFILE2, CMainWindow::OpenRecentFile )
	EVT_MENU( wxID_MAINWND_RECENTFILE3, CMainWindow::OpenRecentFile )
	EVT_MENU( wxID_MAINWND_RECENTFILE4, CMainWindow::OpenRecentFile )
	EVT_MENU( wxID_EXIT, CMainWindow::OnExit )
	EVT_MENU( wxID_MAINWND_CENTERVIEW, CMainWindow::CenterView )
	EVT_MENU( wxID_MAINWND_SAVEVIEW, CMainWindow::SaveView )
	EVT_MENU( wxID_MAINWND_RESTOREVIEW, CMainWindow::RestoreView )
	EVT_MENU( wxID_MAINWND_TAKESCREENSHOT, CMainWindow::TakeScreenshot )
	EVT_MENU( wxID_MAINWND_DUMPMODELINFO, CMainWindow::DumpModelInfo )
	EVT_MENU( wxID_MAINWND_TOGGLEMESSAGES, CMainWindow::ShowMessagesWindow )
	EVT_MENU( wxID_MAINWND_OPTIONS, CMainWindow::OpenOptionsDialog )
	EVT_MENU( wxID_ABOUT, CMainWindow::OnAbout )
wxEND_EVENT_TABLE()

CMainWindow::CMainWindow( CHLMV* const pHLMV )
	: wxFrame( nullptr, wxID_ANY, HLMV_TITLE, wxDefaultPosition, wxSize( 600, 400 ) )
	, m_pHLMV( pHLMV )
{
	pHLMV->SetMainWindow( this );

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

	for( size_t uiIndex = 0; uiIndex < CHLMVSettings::MAX_RECENT_FILES; ++uiIndex )
	{
		wxMenuItem* pItem = m_RecentFiles[ uiIndex ] = pRecentFiles->Append( wxID_MAINWND_RECENTFILE1 + static_cast<int>( uiIndex ), "(empty)" );
	}

	RefreshRecentFiles();

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

	pMenuTools->Append( wxID_MAINWND_OPTIONS, "Options" );

	wxMenu* menuHelp = new wxMenu;

	menuHelp->Append( wxID_ABOUT );

	wxMenuBar* menuBar = new wxMenuBar;

	menuBar->Append( menuFile, "&File" );
	menuBar->Append( pMenuView, "&View" );
	menuBar->Append( pMenuTools, "&Tools" );
	menuBar->Append( menuHelp, "&Help" );

	SetMenuBar( menuBar );

	CreateStatusBar();
	SetStatusText( "" );

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

void CMainWindow::OnTimer( CTimer& timer )
{
	m_pMainPanel->OnTimer( timer );
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

		RefreshRecentFiles();

		return false;
	}

	const bool bSuccess = m_pMainPanel->LoadModel( szAbsFilename );

	if( bSuccess )
	{
		const wxCStrData data = szAbsFilename.c_str();
		const char* const pszAbsFilename = data.AsChar();

		this->SetTitle( wxString::Format( "%s - %s", HLMV_TITLE, pszAbsFilename ) );

		m_pHLMV->GetSettings()->GetRecentFiles()->Add( pszAbsFilename );

		RefreshRecentFiles();

		Message( "Loaded model \"%s\"\n", pszAbsFilename );
	}
	else
		this->SetTitle( HLMV_TITLE );

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
	if( !m_pHLMV->GetState()->GetStudioModel() )
		return false;

	const bool bSuccess = m_pHLMV->GetState()->GetStudioModel()->SaveModel( szFilename.char_str( wxMBConvUTF8() ) );

	if( !bSuccess )
	{
		wxMessageBox( wxString::Format( "An error occurred while saving the model \"%s\"", szFilename.c_str() ) );
	}

	return bSuccess;
}

bool CMainWindow::PromptSaveModel()
{
	if( m_pHLMV->GetState()->GetStudioModel() == nullptr )
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
	if( !m_pHLMV->GetState()->GetStudioModel() )
	{
		wxMessageBox( "No model loaded!" );
		return;
	}

	if( m_pHLMV->GetState()->DumpModelInfo( HLMV_DUMP_MODEL_INFO_FILE ) )
	{
		//Launch the default text editor.
		wxFileType* pFileType = wxTheMimeTypesManager->GetFileTypeFromExtension( "txt" );

		bool bSuccess = false;

		if( pFileType )
		{
			wxString szOpenCommand;

			if( pFileType->GetOpenCommand( &szOpenCommand, wxFileType::MessageParameters( HLMV_DUMP_MODEL_INFO_FILE ) ) )
			{
				bSuccess = wxExecute( szOpenCommand, wxEXEC_ASYNC ) != 0;
			}

			delete pFileType;
		}

		if( !bSuccess )
		{
			wxMessageBox( "Unable to open default text editor" );
		}
	}
	else
	{
		wxMessageBox( "An error occurred while dumping model info" );
	}
}

void CMainWindow::RefreshRecentFiles()
{
	const auto& recentFiles = m_pHLMV->GetSettings()->GetRecentFiles()->GetFiles();

	auto it = recentFiles.begin();
	auto end = recentFiles.end();

	for( size_t uiIndex = 0; uiIndex < CHLMVSettings::MAX_RECENT_FILES; ++uiIndex )
	{
		wxMenuItem* const pItem = m_RecentFiles[ uiIndex ];

		if( it != end )
		{
			pItem->Enable( true );
			pItem->SetItemLabel( it->c_str() );

			++it;
		}
		else
		{
			pItem->Enable( false );
			pItem->SetItemLabel( "(empty)" );
		}
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
	if( event.GetId() < wxID_MAINWND_RECENTFILE1 || event.GetId() > wxID_MAINWND_RECENTFILE4 )
	{
		wxMessageBox( wxString::Format( "Invalid ID specified for recent files list (%d)", event.GetId() ) );
		return;
	}

	const size_t fileId = static_cast<size_t>( event.GetId() - wxID_MAINWND_RECENTFILE1 );

	const wxString szFilename = m_pHLMV->GetSettings()->GetRecentFiles()->Get( fileId );

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
					"Contains FMOD, Copyright © Firelight Technologies Pty, Ltd., 2012-2016.\n\n"
					"Contains The OpenGL Extension Wrangler Library\n"
					"Copyright( C ) 2008 - 2016, Nigel Stewart <nigels[]users sourceforge net>\n"
					"Copyright( C ) 2002 - 2008, Milan Ikits <milan ikits[]ieee org>\n"
					"Copyright( C ) 2002 - 2008, Marcelo E.Magallon <mmagallo[]debian org>\n"
					"Copyright( C ) 2002, Lev Povalahev\n"
					"All rights reserved.\n\n"
					"Uses wxWidgets %d.%d.%d\n\n"
					"Build Date: %s\n", 
					wxMAJOR_VERSION, wxMINOR_VERSION, wxRELEASE_NUMBER, 
					__DATE__ 
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
}