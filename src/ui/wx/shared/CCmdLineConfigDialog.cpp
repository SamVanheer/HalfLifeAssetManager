#include <wx/editlbox.h>
#include <wx/sizer.h>
#include <wx/statline.h>

#include "CCmdLineConfigDialog.h"

namespace ui
{
enum
{
	wxID_FIRST_ID = wxID_HIGHEST + 1,

	wxID_ADD_PARAMETER = wxID_FIRST_ID,
	wxID_EDIT_PARAMETER_NAME,
	wxID_REMOVE_PARAMETER,
	
	wxID_SHOULD_COPY_FILES,
};

wxBEGIN_EVENT_TABLE( CCmdLineConfigDialog, wxDialog )
	EVT_BUTTON( wxID_ADD_PARAMETER, CCmdLineConfigDialog::OnAddParameter )
	EVT_BUTTON( wxID_EDIT_PARAMETER_NAME, CCmdLineConfigDialog::OnEditParameterName )
	EVT_BUTTON( wxID_REMOVE_PARAMETER, CCmdLineConfigDialog::OnRemoveParameter )
	EVT_CHECKBOX( wxID_SHOULD_COPY_FILES, CCmdLineConfigDialog::OnCopyFilesChanged )
wxEND_EVENT_TABLE()

CCmdLineConfigDialog::CCmdLineConfigDialog( wxWindow *parent, wxWindowID id,
											const wxString& title,
											const wxPoint& pos,
											const wxSize& size,
											long style,
											const wxString& name )
	: wxDialog( parent, id, title, pos, size, style, name )
{
	auto pScroll = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxSize( 400, 200 ) );

	m_pParameterGrid = new wxPropertyGrid( pScroll, wxID_ANY, wxDefaultPosition, wxSize( wxDefaultSize.GetWidth(), pScroll->GetClientSize().GetHeight() ) );

	auto pAddParam = new wxButton( this, wxID_ADD_PARAMETER, "Add New" );

	auto pEditParamName = new wxButton( this, wxID_EDIT_PARAMETER_NAME, "Edit Name" );

	auto pRemoveParam = new wxButton( this, wxID_REMOVE_PARAMETER, "Remove" );

	m_pCopyFiles = new wxCheckBox( this, wxID_SHOULD_COPY_FILES, "Copy Output Files?" );
	m_pCopyFiles->Enable( false );

	m_pOutputFilters = new wxEditableListBox( this, wxID_ANY, "Filters",
											  wxDefaultPosition, wxSize( wxDefaultSize.GetWidth(), 200 ), 
											  wxEL_ALLOW_NEW | wxEL_ALLOW_EDIT | wxEL_ALLOW_DELETE | wxEL_NO_REORDER );

	m_pOutputFilters->Enable( false );

	//Layout
	auto pSizer = new wxBoxSizer( wxVERTICAL );

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

		pButtonsSizer->Add( pEditParamName, wxSizerFlags() );

		pButtonsSizer->Add( pRemoveParam, wxSizerFlags() );

		pSizer->Add( pButtonsSizer, wxSizerFlags().Expand().DoubleBorder() );
	}

	pSizer->Add( new wxStaticLine( this ), wxSizerFlags().Expand().DoubleBorder() );

	pSizer->Add( m_pCopyFiles, wxSizerFlags().Expand().DoubleBorder( wxLEFT | wxRIGHT ) );

	pSizer->Add( m_pOutputFilters, wxSizerFlags().Expand().DoubleBorder( wxLEFT | wxRIGHT | wxUP ) );

	pSizer->Add( new wxStaticLine( this ), wxSizerFlags().Expand().DoubleBorder() );

	pSizer->Add( this->CreateButtonSizer( wxOK | wxCANCEL ), wxSizerFlags().Expand().DoubleBorder( wxLEFT | wxRIGHT | wxDOWN ) );

	this->SetSizer( pSizer );

	this->Fit();

	this->CenterOnScreen();
}

CCmdLineConfigDialog::~CCmdLineConfigDialog()
{
}

std::vector<std::pair<std::string, std::string>> CCmdLineConfigDialog::GetParameters() const
{
	std::vector<std::pair<std::string, std::string>> parameters;

	for( auto it = m_pParameterGrid->GetIterator(); !it.AtEnd(); it.Next( false ) )
	{
		auto pProp = static_cast<wxStringProperty*>( it.GetProperty() );

		parameters.emplace_back( std::make_pair( pProp->GetLabel(), pProp->GetValue() ) );
	}

	return parameters;
}

bool CCmdLineConfigDialog::ShouldCopyFiles() const
{
	return m_pCopyFiles->GetValue();
}

void CCmdLineConfigDialog::SetCopySupportEnabled( const bool bEnabled )
{
	m_pCopyFiles->Enable( bEnabled );
	m_pOutputFilters->Enable( bEnabled && m_pCopyFiles->GetValue() );
}

wxArrayString CCmdLineConfigDialog::GetOutputFileFilters() const
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

void CCmdLineConfigDialog::OnAddParameter( wxCommandEvent& event )
{
	wxTextEntryDialog dlg( this, "Enter a parameter name", "Enter name" );

	if( dlg.ShowModal() == wxID_CANCEL )
		return;

	m_pParameterGrid->Append( new wxStringProperty( dlg.GetValue(), wxEmptyString, wxEmptyString ) );
}

void CCmdLineConfigDialog::OnEditParameterName( wxCommandEvent& event )
{
	auto pProp = m_pParameterGrid->GetSelectedProperty();

	if( !pProp )
	{
		wxMessageBox( "No parameter selected!" );
		return;
	}

	m_pParameterGrid->BeginLabelEdit( 0 );
}

void CCmdLineConfigDialog::OnRemoveParameter( wxCommandEvent& event )
{
	auto pProp = m_pParameterGrid->GetSelectedProperty();

	if( !pProp )
	{
		wxMessageBox( "No parameter selected!" );
		return;
	}

	m_pParameterGrid->DeleteProperty( pProp );
}

void CCmdLineConfigDialog::OnCopyFilesChanged( wxCommandEvent& event )
{
	m_pOutputFilters->Enable( m_pCopyFiles->IsEnabled() && m_pCopyFiles->GetValue() );
}
}