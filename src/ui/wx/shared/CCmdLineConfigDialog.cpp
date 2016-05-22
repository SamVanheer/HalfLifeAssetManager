#include <wx/sizer.h>

#include "CCmdLineConfigDialog.h"

namespace ui
{
enum
{
	wxID_FIRST_ID = wxID_HIGHEST + 1,

	wxID_ADD_PARAMETER = wxID_FIRST_ID,
	wxID_EDIT_PARAMETER_NAME,
	wxID_REMOVE_PARAMETER,
};

wxBEGIN_EVENT_TABLE( CCmdLineConfigDialog, wxDialog )
	EVT_BUTTON( wxID_ADD_PARAMETER, CCmdLineConfigDialog::OnAddParameter )
	EVT_BUTTON( wxID_EDIT_PARAMETER_NAME, CCmdLineConfigDialog::OnEditParameterName )
	EVT_BUTTON( wxID_REMOVE_PARAMETER, CCmdLineConfigDialog::OnRemoveParameter )
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

	//Layout
	auto pSizer = new wxBoxSizer( wxVERTICAL );

	pSizer->Add( new wxStaticText( this, wxID_ANY, "Command Line Parameters" ), wxSizerFlags() );

	{
		auto pScrollSizer = new wxBoxSizer( wxVERTICAL );

		pScrollSizer->Add( m_pParameterGrid, wxSizerFlags().Expand() );

		pScroll->SetSizer( pScrollSizer );
	}

	pSizer->Add( pScroll, wxSizerFlags().Expand() );

	{
		auto pButtonsSizer = new wxBoxSizer( wxHORIZONTAL );

		pButtonsSizer->Add( pAddParam, wxSizerFlags() );

		pButtonsSizer->Add( pEditParamName, wxSizerFlags() );

		pButtonsSizer->Add( pRemoveParam, wxSizerFlags() );

		pSizer->Add( pButtonsSizer, wxSizerFlags().Expand() );
	}

	pSizer->Add( this->CreateSeparatedButtonSizer( wxOK | wxCANCEL ), wxSizerFlags().Expand() );

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
}