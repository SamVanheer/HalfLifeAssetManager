#include <algorithm>

#include <wx/editlbox.h>
#include <wx/sizer.h>
#include <wx/statline.h>

#include "CEditCmdLineConfigDialog.h"

namespace ui
{
enum
{
	wxID_FIRST_ID = wxID_HIGHEST + 1,

	wxID_NAME = wxID_FIRST_ID,

	wxID_ADD_PARAMETER = wxID_FIRST_ID,
	wxID_REMOVE_PARAMETER,

	wxID_SHOULD_COPY_FILES,
};

wxBEGIN_EVENT_TABLE( CEditCmdLineConfigDialog, wxDialog )
	EVT_BUTTON( wxID_ADD_PARAMETER, CEditCmdLineConfigDialog::OnAddParameter )
	EVT_BUTTON( wxID_REMOVE_PARAMETER, CEditCmdLineConfigDialog::OnRemoveParameter )
	EVT_CHECKBOX( wxID_SHOULD_COPY_FILES, CEditCmdLineConfigDialog::OnCopyFilesChanged )
	EVT_BUTTON( wxID_OK, CEditCmdLineConfigDialog::OnButtonPressed )
	EVT_BUTTON( wxID_APPLY, CEditCmdLineConfigDialog::OnButtonPressed )
wxEND_EVENT_TABLE()

CEditCmdLineConfigDialog::CEditCmdLineConfigDialog( wxWindow *parent, wxWindowID id,
											std::shared_ptr<settings::CCmdLineConfigManager> mutableManager,
											std::shared_ptr<settings::CCmdLineConfig> config,
											const wxPoint& pos,
											const wxSize& size,
											long style,
											const wxString& name )
	: wxDialog( parent, id, "", pos, size, style, name )
	, m_MutableManager( mutableManager )
	, m_Config( config )
{
	wxASSERT( mutableManager );
	wxASSERT( config );

	m_MutableConfig = std::make_shared<settings::CCmdLineConfig>( *config );

	UpdateTitle();

	m_pName = new wxTextCtrl( this, wxID_NAME );

	auto pScroll = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxSize( 400, 200 ) );

	m_pParameterGrid = new wxPropertyGrid( pScroll, wxID_ANY, wxDefaultPosition, wxSize( wxDefaultSize.GetWidth(), pScroll->GetClientSize().GetHeight() ),
										   wxPG_DEFAULT_STYLE | wxPG_STATIC_SPLITTER | wxPG_SPLITTER_AUTO_CENTER );

	//Make parameter labels editable.
	m_pParameterGrid->MakeColumnEditable( 0, true );

	auto pAddParam = new wxButton( this, wxID_ADD_PARAMETER, "Add New" );

	m_pRemoveParam = new wxButton( this, wxID_REMOVE_PARAMETER, "Remove" );

	m_pCopyFiles = new wxCheckBox( this, wxID_SHOULD_COPY_FILES, "Copy Output Files?" );
	m_pCopyFiles->Enable( false );

	m_pOutputFilters = new wxEditableListBox( this, wxID_ANY, "Filters",
											  wxDefaultPosition, wxSize( wxDefaultSize.GetWidth(), 200 ),
											  wxEL_ALLOW_NEW | wxEL_ALLOW_EDIT | wxEL_ALLOW_DELETE | wxEL_NO_REORDER );

	m_pOutputFilters->Enable( false );

	//Layout
	auto pSizer = new wxBoxSizer( wxVERTICAL );

	pSizer->Add( m_pName, wxSizerFlags().Expand().DoubleBorder() );

	pSizer->Add( new wxStaticText( this, wxID_ANY, "Command Line Parameters" ), wxSizerFlags().DoubleBorder() );

	{
		auto pScrollSizer = new wxBoxSizer( wxVERTICAL );

		pScrollSizer->Add( m_pParameterGrid, wxSizerFlags().Expand() );

		pScroll->SetSizer( pScrollSizer );
	}

	pSizer->Add( pScroll, wxSizerFlags().Expand().DoubleBorder( wxLEFT | wxRIGHT ) );

	{
		auto pButtonsSizer = new wxBoxSizer( wxHORIZONTAL );

		pButtonsSizer->Add( pAddParam, wxSizerFlags() );

		pButtonsSizer->Add( m_pRemoveParam, wxSizerFlags() );

		pSizer->Add( pButtonsSizer, wxSizerFlags().Expand().DoubleBorder() );
	}

	pSizer->Add( new wxStaticLine( this ), wxSizerFlags().Expand().DoubleBorder() );

	pSizer->Add( m_pCopyFiles, wxSizerFlags().Expand().DoubleBorder( wxLEFT | wxRIGHT ) );

	pSizer->Add( m_pOutputFilters, wxSizerFlags().Expand().DoubleBorder( wxLEFT | wxRIGHT | wxUP ) );

	pSizer->Add( new wxStaticLine( this ), wxSizerFlags().Expand().DoubleBorder() );

	pSizer->Add( this->CreateButtonSizer( wxOK | wxCANCEL | wxAPPLY ), wxSizerFlags().Expand().DoubleBorder( wxLEFT | wxRIGHT | wxDOWN ) );

	this->SetSizer( pSizer );

	this->Fit();

	this->CenterOnScreen();

	Initialize();
}

CEditCmdLineConfigDialog::~CEditCmdLineConfigDialog()
{
}

void CEditCmdLineConfigDialog::SetCopySupportEnabled( const bool bEnabled )
{
	m_pCopyFiles->Enable( bEnabled );
	m_pCopyFiles->SetValue( bEnabled && m_MutableConfig->ShouldCopyOutputFiles() );
	m_pOutputFilters->Enable( bEnabled && m_pCopyFiles->GetValue() );
}

void CEditCmdLineConfigDialog::Initialize()
{
	InitFromConfig( m_MutableConfig );
}

bool CEditCmdLineConfigDialog::Save()
{
	SaveToConfig( m_MutableConfig );

	wxString szName = m_MutableConfig->GetName();

	if( !m_MutableManager->CanRenameConfig( m_Config, szName ) )
	{
		wxMessageBox( "Please enter a unique name for the configuration", "Enter name", wxOK | wxCENTRE | wxICON_ERROR );
		return false;
	}

	*m_Config = *m_MutableConfig;

	return true;
}

void CEditCmdLineConfigDialog::InitFromConfig( const std::shared_ptr<const settings::CCmdLineConfig>& config )
{
	m_pName->SetValue( config->GetName() );

	m_pParameterGrid->Clear();

	for( const auto& param : config->GetParameters() )
	{
		m_pParameterGrid->Append( new wxStringProperty( param.first.c_str(), wxEmptyString, param.second.c_str() ) );
	}

	m_pRemoveParam->Enable( !config->GetParameters().empty() );

	m_pCopyFiles->SetValue( m_pCopyFiles->IsEnabled() && config->ShouldCopyOutputFiles() );
	m_pOutputFilters->Enable( m_pCopyFiles->IsEnabled() && m_pCopyFiles->GetValue() );

	wxArrayString filters;

	for( const auto& filter : config->GetFilters() )
	{
		filters.push_back( filter.c_str() );
	}

	m_pOutputFilters->SetStrings( filters );
}

void CEditCmdLineConfigDialog::SaveToConfig( const std::shared_ptr<settings::CCmdLineConfig>& config )
{
	config->SetName( m_pName->GetValue().c_str().AsChar() );

	config->SetParameters( GetParameters() );
	config->SetCopyOutputFiles( ShouldCopyFiles() );

	settings::CCmdLineConfig::Filters_t filters;

	for( const auto& filter : GetOutputFileFilters() )
	{
		filters.emplace_back( filter.c_str() );
	}

	config->SetFilters( filters );
}

std::vector<std::pair<std::string, std::string>> CEditCmdLineConfigDialog::GetParameters() const
{
	std::vector<std::pair<std::string, std::string>> parameters;

	for( auto it = m_pParameterGrid->GetIterator(); !it.AtEnd(); it.Next( false ) )
	{
		auto pProp = static_cast<wxStringProperty*>( it.GetProperty() );

		parameters.emplace_back( std::make_pair( pProp->GetLabel(), pProp->GetValue() ) );
	}

	return parameters;
}

bool CEditCmdLineConfigDialog::ShouldCopyFiles() const
{
	return m_pCopyFiles->GetValue();
}

wxArrayString CEditCmdLineConfigDialog::GetOutputFileFilters() const
{
	wxArrayString filters;

	m_pOutputFilters->GetStrings( filters );

	wxArrayString result;

	//Duplicate filters should be removed so avoid copying multiple times.
	for( const auto& filter : filters )
	{
		if( result.Index( filter ) == wxNOT_FOUND )
			result.Add( filter );
	}

	return result;
}

void CEditCmdLineConfigDialog::UpdateTitle()
{
	SetTitle( wxString::Format( "Editing Command Line Configuration: %s", m_MutableConfig->GetName().c_str() ) );
}

void CEditCmdLineConfigDialog::OnAddParameter( wxCommandEvent& event )
{
	wxTextEntryDialog dlg( this, "Enter a parameter name", "Enter name" );

	if( dlg.ShowModal() == wxID_CANCEL )
		return;

	m_pParameterGrid->Append( new wxStringProperty( dlg.GetValue(), wxEmptyString, wxEmptyString ) );

	m_pRemoveParam->Enable( true );
}

void CEditCmdLineConfigDialog::OnRemoveParameter( wxCommandEvent& event )
{
	auto pProp = m_pParameterGrid->GetSelectedProperty();

	if( !pProp )
	{
		wxMessageBox( "No parameter selected!" );
		return;
	}

	m_pParameterGrid->DeleteProperty( pProp );

	if( m_pParameterGrid->GetIterator().AtEnd() )
	{
		m_pRemoveParam->Enable( false );
	}
}

void CEditCmdLineConfigDialog::OnCopyFilesChanged( wxCommandEvent& event )
{
	m_pOutputFilters->Enable( m_pCopyFiles->IsEnabled() && m_pCopyFiles->GetValue() );
}

void CEditCmdLineConfigDialog::OnButtonPressed( wxCommandEvent& event )
{
	bool bShouldSkip = true;

	switch( event.GetId() )
	{
	case wxID_OK:
		{
			if( !Save() )
				bShouldSkip = false;

			break;
		}

	case wxID_APPLY:
		{
			if( Save() )
			{
				UpdateTitle();
			}

			break;
		}

	default: break;
	}

	event.Skip( bShouldSkip );
}
}