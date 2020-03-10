#include <cstdint>

#include <wx/gbsizer.h>
#include <wx/spinctrl.h>
#include <wx/statline.h>

#include "shared/Utility.h"

#include "CEditEventsDialog.h"

namespace ui
{
enum
{
	wxID_FIRST_ID = wxID_HIGHEST + 1,

	wxID_EVENTS = wxID_FIRST_ID,
};

wxBEGIN_EVENT_TABLE( CEditEventsDialog, wxDialog )
	EVT_LISTBOX( wxID_EVENTS, CEditEventsDialog::OnEventChanged )
	EVT_BUTTON( wxID_OK, CEditEventsDialog::OnButton )
	EVT_BUTTON( wxID_CANCEL, CEditEventsDialog::OnButton )
	EVT_BUTTON( wxID_APPLY, CEditEventsDialog::OnButton )
wxEND_EVENT_TABLE()

CEditEventsDialog::CEditEventsDialog( wxWindow *parent, wxWindowID id,
									const wxString& title,
									mstudioevent_t* const pEvents, const size_t uiNumEvents,
									const wxPoint& pos,
									const wxSize& size,
									long style,
									const wxString& name )
	: wxDialog( parent, id, title, pos, size, style, name )
	, m_pDestEvents( pEvents )
{
	m_pEvents = new wxListBox( this, wxID_EVENTS, wxDefaultPosition, wxSize( 200, 400 ) );

	m_pFrame = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 150, wxDefaultSize.GetHeight() ) );
	m_pFrame->SetRange( 0, INT_MAX );

	m_pEvent = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 150, wxDefaultSize.GetHeight() ) );
	m_pEvent->SetRange( INT_MIN, INT_MAX );

	m_pOptions = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 150, wxDefaultSize.GetHeight() ) );

	m_pOptions->SetMaxLength( STUDIO_MAX_EVENT_OPTIONS_LENGTH - 1 );

	m_pType = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 150, wxDefaultSize.GetHeight() ) );
	m_pType->SetRange( INT_MIN, INT_MAX );

	//Layout
	auto pSizer = new wxGridBagSizer( 5, 5 );

	int iRow = 0;
	int iCol = 0;

	pSizer->Add( new wxStaticText( this, wxID_ANY, "Frame:" ), wxGBPosition( iRow, iCol ), wxDefaultSpan, wxEXPAND );
	pSizer->Add( m_pFrame, wxGBPosition( iRow++, iCol + 1 ), wxDefaultSpan, wxEXPAND );

	pSizer->Add( new wxStaticText( this, wxID_ANY, "Event:" ), wxGBPosition( iRow, iCol ), wxDefaultSpan, wxEXPAND );
	pSizer->Add( m_pEvent, wxGBPosition( iRow++, iCol + 1 ), wxDefaultSpan, wxEXPAND );

	pSizer->Add( new wxStaticText( this, wxID_ANY, "Options:" ), wxGBPosition( iRow, iCol ), wxDefaultSpan, wxEXPAND );
	pSizer->Add( m_pOptions, wxGBPosition( iRow++, iCol + 1 ), wxDefaultSpan, wxEXPAND );

	pSizer->Add( new wxStaticText( this, wxID_ANY, "Type:" ), wxGBPosition( iRow, iCol ), wxDefaultSpan, wxEXPAND );
	pSizer->Add( m_pType, wxGBPosition( iRow++, iCol + 1 ), wxDefaultSpan, wxEXPAND );

	pSizer->Add( new wxStaticLine( this ), wxGBPosition( iRow++, iCol ), wxGBSpan( 1, 2 ), wxEXPAND );

	auto pMainSizer = new wxBoxSizer( wxVERTICAL );

	pMainSizer->Add( new wxStaticText( this, wxID_ANY, "Edit Events" ), wxSizerFlags().Expand().DoubleBorder( wxLEFT | wxRIGHT | wxUP ) );

	{
		auto pHorizSizer = new wxBoxSizer( wxHORIZONTAL );

		pHorizSizer->Add( m_pEvents, wxSizerFlags().Expand() );
		pHorizSizer->AddSpacer( 20 );
		pHorizSizer->Add( pSizer, wxSizerFlags().Expand() );

		pMainSizer->Add( pHorizSizer, wxSizerFlags().Expand().DoubleBorder() );
	}

	pMainSizer->Add( this->CreateSeparatedButtonSizer( wxOK | wxCANCEL | wxAPPLY ), wxSizerFlags().Expand().DoubleBorder() );

	this->SetSizer( pMainSizer );

	this->Fit();

	this->CenterOnScreen();

	InitFromEvents( pEvents, uiNumEvents );
}

CEditEventsDialog::~CEditEventsDialog()
{
}

void CEditEventsDialog::SetEvent( const int iIndex )
{
	const size_t uiIndex = static_cast<size_t>( iIndex );

	const bool bIsValid = iIndex >= 0 && uiIndex < m_Events.size();

	m_pFrame->Enable( bIsValid );
	m_pEvent->Enable( bIsValid );
	m_pOptions->Enable( bIsValid );
	m_pType->Enable( bIsValid );

	if( !bIsValid )
	{
		return;
	}

	//Save current event data.
	if( m_CurrentEvent.first )
		CopyToEvent( m_Events[ m_CurrentEvent.second ] );

	InitFromEvent( m_Events[ uiIndex ] );

	m_CurrentEvent.first = true;
	m_CurrentEvent.second = uiIndex;
}

void CEditEventsDialog::InitFromEvents( const mstudioevent_t* const pEvents, const size_t uiNumEvents )
{
	m_pEvents->Clear();

	m_Events.clear();

	m_CurrentEvent.first = false;
	m_CurrentEvent.second = 0;

	m_Events.resize( uiNumEvents );

	wxArrayString events;

	for( size_t uiIndex = 0; uiIndex < uiNumEvents; ++uiIndex )
	{
		m_Events[ uiIndex ] = pEvents[ uiIndex ];

		events.push_back( wxString::Format( "Event %u", uiIndex + 1 ) );
	}

	m_pEvents->Append( events );

	SetEvent( 0 );
}

void CEditEventsDialog::CopyToEvents( mstudioevent_t* const pEvents, const size_t uiNumEvents )
{
	if( !pEvents || !uiNumEvents )
		return;

	//Save current event data.
	if( m_CurrentEvent.first )
		CopyToEvent( m_Events[ m_CurrentEvent.second ] );

	for( size_t uiIndex = 0; uiIndex < uiNumEvents; ++uiIndex )
	{
		pEvents[ uiIndex ] = m_Events[ uiIndex ];
	}
}

void CEditEventsDialog::InitFromEvent( const mstudioevent_t& event )
{
	m_pFrame->SetValue( event.frame );
	m_pEvent->SetValue( event.event );
	m_pOptions->SetValue( event.options );
	m_pType->SetValue( event.type );
}

void CEditEventsDialog::CopyToEvent( mstudioevent_t& event )
{
	event.frame = m_pFrame->GetValue();
	event.event = m_pEvent->GetValue();

	wxString szOptions = m_pOptions->GetValue();

	szOptions.Truncate( ARRAYSIZE( event.options ) - 1 );

	m_pOptions->SetValue( szOptions );

	strcpy( event.options, szOptions.c_str().AsChar() );

	event.type = m_pType->GetValue();
}

void CEditEventsDialog::OnEventChanged( wxCommandEvent& event )
{
	SetEvent( m_pEvents->GetSelection() );
}

void CEditEventsDialog::OnButton( wxCommandEvent& event )
{
	switch( event.GetId() )
	{
	case wxID_OK:
	case wxID_APPLY:
		{
			//Save state to events.
			CopyToEvents( m_pDestEvents, m_Events.size() );

			m_bChangedSaved = true;

			break;
		}

	default:
		break;
	}

	event.Skip();
}
}