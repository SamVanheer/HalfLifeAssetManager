#include <wx/gbsizer.h>

#include "settings/CGameConfig.h"
#include "settings/CGameConfigManager.h"

#include "CEditGameConfigsDialog.h"

#include "CGameConfigurations.h"

namespace ui
{
wxBEGIN_EVENT_TABLE( CGameConfigurations, wxPanel )
	EVT_COMBOBOX( wxID_SHARED_GAMECONFIGS_CONFIG_CHANGED, CGameConfigurations::ConfigChanged )
	EVT_BUTTON( wxID_SHARED_GAMECONFIGS_EDIT, CGameConfigurations::EditConfigs )
	EVT_BUTTON( wxID_SHARED_GAMECONFIGS_FINDBASEPATH, CGameConfigurations::FindBasePath )
wxEND_EVENT_TABLE()

CGameConfigurations::CGameConfigurations( wxWindow* pParent, std::shared_ptr<settings::CGameConfigManager> manager )
	: wxPanel( pParent, wxID_ANY, wxDefaultPosition, wxSize( 600, 600 ) )
	, m_Manager( manager )
{
	wxASSERT( manager != nullptr );

	m_pActiveConfig = new wxComboBox( this, wxID_ANY );
	m_pActiveConfig->SetEditable( false );

	m_pConfigs = new wxComboBox( this, wxID_SHARED_GAMECONFIGS_CONFIG_CHANGED );
	m_pConfigs->SetEditable( false );

	wxButton* pEditConfigs = new wxButton( this, wxID_SHARED_GAMECONFIGS_EDIT, "Edit" );

	m_pBasePath = new wxTextCtrl( this, wxID_ANY );
	m_pFindBasePath = new wxButton( this, wxID_SHARED_GAMECONFIGS_FINDBASEPATH, "..." );

	m_pGameDir = new wxTextCtrl( this, wxID_ANY );

	m_pModDir = new wxTextCtrl( this, wxID_ANY );

	//Layout
	wxGridBagSizer* pSizer = new wxGridBagSizer( 5, 5 );

	int iRow = 0;

	pSizer->Add( new wxStaticText( this, wxID_ANY, "Active Configuration:" ), wxGBPosition( iRow++, 0 ), wxGBSpan( 1, 5 ), wxEXPAND );
	pSizer->Add( m_pActiveConfig, wxGBPosition( iRow++, 0 ), wxGBSpan( 1, 5 ), wxEXPAND );

	pSizer->Add( new wxStaticText( this, wxID_ANY, "Game Configurations:" ), wxGBPosition( iRow++, 0 ), wxGBSpan( 1, 5 ), wxEXPAND );
	pSizer->Add( m_pConfigs, wxGBPosition( iRow, 0 ), wxGBSpan( 1, 4 ), wxEXPAND );
	pSizer->Add( pEditConfigs, wxGBPosition( iRow++, 4 ), wxGBSpan( 1, 1 ), wxEXPAND );

	pSizer->Add( new wxStaticText( this, wxID_ANY, "Base path:" ), wxGBPosition( iRow++, 0 ), wxGBSpan( 1, 5 ), wxEXPAND );
	pSizer->Add( m_pBasePath, wxGBPosition( iRow, 0 ), wxGBSpan( 1, 4 ), wxEXPAND );
	pSizer->Add( m_pFindBasePath, wxGBPosition( iRow++, 4 ), wxGBSpan( 1, 1 ), wxEXPAND );

	pSizer->Add( new wxStaticText( this, wxID_ANY, "Game directory:" ), wxGBPosition( iRow++, 0 ), wxGBSpan( 1, 5 ), wxEXPAND );
	pSizer->Add( m_pGameDir, wxGBPosition( iRow++, 0 ), wxGBSpan( 1, 5 ), wxEXPAND );

	pSizer->Add( new wxStaticText( this, wxID_ANY, "Mod directory:" ), wxGBPosition( iRow++, 0 ), wxGBSpan( 1, 5 ), wxEXPAND );
	pSizer->Add( m_pModDir, wxGBPosition( iRow++, 0 ), wxGBSpan( 1, 5 ), wxEXPAND );

	for( size_t uiCol = 0; uiCol < 5; ++uiCol )
		pSizer->AddGrowableCol( uiCol );

	this->SetSizer( pSizer );

	Initialize();

	SetCurrentConfig( m_pActiveConfig->GetSelection() );
}

CGameConfigurations::~CGameConfigurations()
{
}

void CGameConfigurations::Save()
{
	StoreConfig( m_pConfigs->GetSelection() );

	const int iIndex = m_pActiveConfig->GetSelection();

	if( iIndex != wxNOT_FOUND )
	{
		m_Manager->SetActiveConfig( m_Manager->GetConfig( m_pActiveConfig->GetStringSelection().c_str() ) );
	}
	else
	{
		m_Manager->ClearActiveConfig();
	}
}

void CGameConfigurations::ConfigAdded( const wxString& szConfigName )
{
	wxASSERT( !szConfigName.IsEmpty() );

	m_pActiveConfig->Append( szConfigName );
	m_pConfigs->Append( szConfigName );

	if( m_pConfigs->GetSelection() == wxNOT_FOUND )
	{
		SetCurrentConfig( 0 );
	}

	if( m_pActiveConfig->GetSelection() == wxNOT_FOUND )
	{
		m_pActiveConfig->SetSelection( 0 );
	}
}

void CGameConfigurations::ConfigRenamed( const wxString& szOldName, const wxString& szNewName )
{
	wxASSERT( !szOldName.IsEmpty() );
	wxASSERT( !szNewName.IsEmpty() );

	int iIndex = m_pConfigs->FindString( szOldName );

	if( iIndex != wxNOT_FOUND )
	{
		m_pConfigs->SetString( iIndex, szNewName );
	}
	else
	{
		wxMessageBox( wxString::Format( "CGameConfigurations::ConfigRenamed: Attempted to rename a config \"%s\" that doesn't exist!", szOldName ) );
		return;
	}

	iIndex = m_pActiveConfig->FindString( szOldName );

	if( iIndex != wxNOT_FOUND )
	{
		m_pActiveConfig->SetString( iIndex, szNewName );
	}
	else
	{
		wxMessageBox( wxString::Format( "CGameConfigurations::ConfigRenamed: Attempted to rename a config \"%s\" that doesn't exist!", szOldName ) );
		return;
	}
}

void CGameConfigurations::ConfigRemoved( const wxString& szConfigName )
{
	wxASSERT( !szConfigName.IsEmpty() );

	int iIndex = m_pConfigs->FindString( szConfigName );

	if( iIndex != wxNOT_FOUND )
		m_pConfigs->Delete( iIndex );

	//This should be the same index as the one returned above. Still, can't hurt to be certain.
	iIndex = m_pActiveConfig->FindString( szConfigName );

	if( iIndex != wxNOT_FOUND )
		m_pActiveConfig->Delete( iIndex );
}

void CGameConfigurations::Initialize()
{
	m_pActiveConfig->Clear();

	wxArrayString configs;

	for( const auto& config : m_Manager->GetConfigs() )
	{
		configs.Add( config->GetName() );
	}

	m_pActiveConfig->Append( configs );

	if( auto config = m_Manager->GetActiveConfig() )
	{
		m_pActiveConfig->SetSelection( m_Manager->IndexOf( config ) );
	}
	else
	{
		//No active config
		m_pActiveConfig->SetSelection( wxNOT_FOUND );
	}

	m_pConfigs->Clear();

	m_pConfigs->Append( configs );
}

void CGameConfigurations::SetCurrentConfig( int iIndex )
{
	if( m_pConfigs->IsListEmpty() )
	{
		m_pBasePath->SetValue( "" );
		m_pGameDir->SetValue( "" );
		m_pModDir->SetValue( "" );

		m_pFindBasePath->Enable( false );
		return;
	}

	if( iIndex < 0 || static_cast<size_t>( iIndex ) >= m_pConfigs->GetCount() )
		iIndex = 0;

	if( m_iCurrentConfig != wxNOT_FOUND )
	{
		StoreConfig( m_iCurrentConfig );
	}

	auto config = m_Manager->GetConfig( m_pConfigs->GetString( iIndex ).c_str() );

	if( !config )
	{
		wxMessageBox( "CGameConfigurations::SetCurrentConfig: Configuration doesn't exist yet!" );
		return;
	}

	m_pConfigs->Select( iIndex );

	m_iCurrentConfig = iIndex;

	m_pBasePath->SetValue( config->GetBasePath() );
	m_pGameDir->SetValue( config->GetGameDir() );
	m_pModDir->SetValue( config->GetModDir() );

	m_pFindBasePath->Enable( true );
}

void CGameConfigurations::StoreConfig( const unsigned int uiIndex )
{
	if( uiIndex >= m_pConfigs->GetCount() )
		return;

	const wxString szName = m_pConfigs->GetString( uiIndex );

	auto config = m_Manager->GetConfig( szName.c_str() );

	if( !config )
	{
		//Doesn't exist yet
		config = std::make_shared<settings::CGameConfig>( szName.c_str() );

		if( !m_Manager->AddConfig( config ) )
		{
			wxMessageBox( wxString::Format( "Error adding configuration \"%s\"", szName ) );
			return;
		}
	}

	config->SetBasePath( m_pBasePath->GetValue().c_str() );
	config->SetGameDir( m_pGameDir->GetValue().c_str() );
	config->SetModDir( m_pModDir->GetValue().c_str() );
}

void CGameConfigurations::ConfigChanged( wxCommandEvent& event )
{
	int iIndex = m_pConfigs->GetSelection();

	if( iIndex == wxNOT_FOUND )
		iIndex = 0;

	SetCurrentConfig( iIndex );
}

void CGameConfigurations::EditConfigs( wxCommandEvent& event )
{
	CEditGameConfigsDialog dlg( this, m_Manager, this );

	dlg.ShowModal();

	/*
	int iIndex = m_pConfigs->GetSelection();

	if( iIndex == wxNOT_FOUND )
		iIndex = 0;

	Initialize( iIndex );
	*/
}

void CGameConfigurations::FindBasePath( wxCommandEvent& event )
{
	wxDirDialog dlg( this, "Select base path" );

	if( dlg.ShowModal() == wxID_CANCEL )
		return;

	m_pBasePath->SetValue( dlg.GetPath() );
}
}