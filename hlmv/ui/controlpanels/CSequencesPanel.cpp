#include <wx/sizer.h>
#include <wx/gbsizer.h>

#include "model/studiomodel/StudioModel.h"

#include "hlmv/CHLMVOptions.h"

#include "CSequencesPanel.h"

wxBEGIN_EVENT_TABLE( CSequencesPanel, CBaseSequencesPanel )
	EVT_COMBOBOX( wxID_SEQUENCE_EVENT, CSequencesPanel::EventChanged )
wxEND_EVENT_TABLE()

CSequencesPanel::CSequencesPanel( wxWindow* pParent )
	: CBaseSequencesPanel( pParent, "Sequences" )
{
	wxWindow* const pElemParent = GetBox();

	wxStaticText* pEvents = new wxStaticText( pElemParent, wxID_ANY, "Events" );

	m_pEvent = new wxComboBox( pElemParent, wxID_SEQUENCE_EVENT, "" );
	m_pEvent->SetEditable( false );

	m_pPlaySound = new wxCheckBox( pElemParent, wxID_ANY, "Play Sound" );

	m_pEventInfo = new wxPanel( pElemParent );

	m_pFrame	= new wxStaticText( m_pEventInfo, wxID_ANY, "Frame: Undefined" );
	m_pEventId	= new wxStaticText( m_pEventInfo, wxID_ANY, "Event: Undefined" );
	m_pOptions	= new wxStaticText( m_pEventInfo, wxID_ANY, "Options: Undefined" );
	m_pType		= new wxStaticText( m_pEventInfo, wxID_ANY, "Type: Undefined" );

	//Layout
	wxGridBagSizer* pSizer = new wxGridBagSizer( 5, 5 );

	CreateUI( pSizer );

	pSizer->Add( pEvents, wxGBPosition( 0, BASESEQUENCES_FIRST_FREE_COL ), wxDefaultSpan, wxEXPAND );
	pSizer->Add( m_pEvent, wxGBPosition( 1, BASESEQUENCES_FIRST_FREE_COL ), wxDefaultSpan, wxEXPAND );
	pSizer->Add( m_pPlaySound, wxGBPosition( 2, BASESEQUENCES_FIRST_FREE_COL ), wxDefaultSpan, wxEXPAND );

	wxBoxSizer* pEventSizer = new wxBoxSizer( wxVERTICAL );

	pEventSizer->Add( m_pFrame );
	pEventSizer->Add( m_pEventId );
	pEventSizer->Add( m_pOptions );
	pEventSizer->Add( m_pType );

	pSizer->Add( m_pEventInfo, wxGBPosition( 0, BASESEQUENCES_FIRST_FREE_COL + 1 ), wxDefaultSpan, wxEXPAND | wxRESERVE_SPACE_EVEN_IF_HIDDEN );

	m_pEventInfo->SetSizer( pEventSizer );

	GetBoxSizer()->Add( pSizer );

	m_pEventInfo->Show( false );
}

CSequencesPanel::~CSequencesPanel()
{
}

void CSequencesPanel::ModelChanged( const StudioModel& model )
{
	CBaseSequencesPanel::ModelChanged( model );
	//TODO: fill info

	UpdateEvents();
}

void CSequencesPanel::SequenceChanged( wxCommandEvent& event )
{
	CBaseSequencesPanel::SequenceChanged( event );

	UpdateEvents();
}

void CSequencesPanel::EventChanged( wxCommandEvent& event )
{
	UpdateEventInfo( m_pEvent->GetSelection() );
}

void CSequencesPanel::UpdateEvents()
{
	const studiohdr_t* const pHdr = Options.GetStudioModel()->getStudioHeader();

	if( !pHdr )
		return;

	m_pEvent->Clear();

	const mstudioseqdesc_t& sequence = ( ( mstudioseqdesc_t* ) ( ( byte* ) pHdr + pHdr->seqindex ) )[ m_pSequence->GetSelection() ];

	m_pEvent->Enable( sequence.numevents > 0 );

	if( sequence.numevents > 0 )
	{
		char szBuffer[ 64 ];

		for( int i = 0; i < sequence.numevents; ++i )
		{
			snprintf( szBuffer, sizeof( szBuffer ), "Event %d", i + 1 );

			m_pEvent->Append( szBuffer );
		}

		m_pEvent->Select( 0 );
		UpdateEventInfo( 0 );
	}
	else
		UpdateEventInfo( -1 );
}

void CSequencesPanel::UpdateEventInfo( int iIndex )
{
	if( iIndex == -1 )
	{
		m_pEventInfo->Show( false );
		return;
	}

	const studiohdr_t* const pHdr = Options.GetStudioModel()->getStudioHeader();

	if( !pHdr )
		return;

	const mstudioseqdesc_t& sequence = ( ( mstudioseqdesc_t* ) ( ( byte* ) pHdr + pHdr->seqindex ) )[ m_pSequence->GetSelection() ];

	if( iIndex < 0 || iIndex >= sequence.numevents )
		return;

	const mstudioevent_t& event = ( ( mstudioevent_t* ) ( ( byte* ) pHdr + sequence.eventindex ) )[ iIndex ];

	m_pFrame->SetLabelText( wxString::Format( "Frame: %d", event.frame ) );
	m_pEventId->SetLabelText( wxString::Format( "Event: %d", event.event ) );
	m_pOptions->SetLabelText( wxString::Format( "Options: %s", static_cast<const char*>( event.options ) ) );
	m_pType->SetLabelText( wxString::Format( "Type: %d", event.type ) );

	m_pEventInfo->Show( true );
}