#include <wx/gbsizer.h>

#include "settings/CGameConfig.h"
#include "settings/CGameConfigManager.h"

#include "CEditGameConfigsDialog.h"

#include "CGameConfigurations.h"

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

	m_pConfigs = new wxComboBox( this, wxID_SHARED_GAMECONFIGS_CONFIG_CHANGED );
	m_pConfigs->SetEditable( false );

	wxButton* pEditConfigs = new wxButton( this, wxID_SHARED_GAMECONFIGS_EDIT, "Edit" );

	m_pBasePath = new wxTextCtrl( this, wxID_ANY );
	m_pFindBasePath = new wxButton( this, wxID_SHARED_GAMECONFIGS_FINDBASEPATH, "..." );

	m_pGameDir = new wxTextCtrl( this, wxID_ANY );

	m_pModDir = new wxTextCtrl( this, wxID_ANY );

	//Layout
	wxGridBagSizer* pSizer = new wxGridBagSizer( 5, 5 );

	pSizer->Add( new wxStaticText( this, wxID_ANY, "Game Configurations:" ), wxGBPosition( 0, 0 ), wxGBSpan( 1, 5 ), wxEXPAND );
	pSizer->Add( m_pConfigs, wxGBPosition( 1, 0 ), wxGBSpan( 1, 4 ), wxEXPAND );
	pSizer->Add( pEditConfigs, wxGBPosition( 1, 4 ), wxGBSpan( 1, 1 ), wxEXPAND );

	pSizer->Add( new wxStaticText( this, wxID_ANY, "Base path:" ), wxGBPosition( 2, 0 ), wxGBSpan( 1, 5 ), wxEXPAND );
	pSizer->Add( m_pBasePath, wxGBPosition( 3, 0 ), wxGBSpan( 1, 4 ), wxEXPAND );
	pSizer->Add( m_pFindBasePath, wxGBPosition( 3, 4 ), wxGBSpan( 1, 1 ), wxEXPAND );

	pSizer->Add( new wxStaticText( this, wxID_ANY, "Game directory:" ), wxGBPosition( 4, 0 ), wxGBSpan( 1, 5 ), wxEXPAND );
	pSizer->Add( m_pGameDir, wxGBPosition( 5, 0 ), wxGBSpan( 1, 5 ), wxEXPAND );

	pSizer->Add( new wxStaticText( this, wxID_ANY, "Mod directory:" ), wxGBPosition( 6, 0 ), wxGBSpan( 1, 5 ), wxEXPAND );
	pSizer->Add( m_pModDir, wxGBPosition( 7, 0 ), wxGBSpan( 1, 5 ), wxEXPAND );

	for( size_t uiCol = 0; uiCol < 5; ++uiCol )
		pSizer->AddGrowableCol( uiCol );

	this->SetSizer( pSizer );

	Initialize();
}

CGameConfigurations::~CGameConfigurations()
{
}

void CGameConfigurations::Save()
{
	StoreConfig( m_pConfigs->GetSelection() );
}

void CGameConfigurations::Initialize( unsigned int uiIndex )
{
	m_pConfigs->Clear();

	wxArrayString configs;

	for( const auto& config : m_Manager->GetConfigs() )
	{
		configs.Add( config->GetName() );
	}

	m_pConfigs->Append( configs );

	if( uiIndex >= m_pConfigs->GetCount() )
		uiIndex = 0;

	SetCurrentConfig( uiIndex );
}

void CGameConfigurations::SetCurrentConfig( unsigned int uiIndex )
{
	if( m_pConfigs->IsListEmpty() )
	{
		m_pBasePath->SetValue( "" );
		m_pGameDir->SetValue( "" );
		m_pModDir->SetValue( "" );

		m_pFindBasePath->Enable( false );
		return;
	}

	if( uiIndex >= m_pConfigs->GetCount() )
		uiIndex = 0;

	if( m_iCurrentConfig != wxNOT_FOUND )
	{
		StoreConfig( m_iCurrentConfig );
	}

	auto config = m_Manager->GetConfig( m_pConfigs->GetString( uiIndex ).c_str() );

	if( !config )
	{
		wxMessageBox( "CGameConfigurations::SetCurrentConfig: Configuration doesn't exist yet!" );
		return;
	}

	m_pConfigs->Select( uiIndex );

	m_iCurrentConfig = uiIndex;

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
	CEditGameConfigsDialog dlg( this, m_Manager );

	dlg.ShowModal();

	int iIndex = m_pConfigs->GetSelection();

	if( iIndex == wxNOT_FOUND )
		iIndex = 0;

	Initialize( iIndex );
}

void CGameConfigurations::FindBasePath( wxCommandEvent& event )
{
	wxDirDialog dlg( this, "Select base path" );

	if( dlg.ShowModal() == wxID_CANCEL )
		return;

	m_pBasePath->SetValue( dlg.GetPath() );
}