#include <wx/sizer.h>
#include <wx/gbsizer.h>

#include "studiomodel/StudioModel.h"

#include "hlmv/ui/CHLMV.h"
#include "hlmv/CHLMVState.h"

#include "CSequencesPanel.h"

namespace hlmv
{
wxBEGIN_EVENT_TABLE( CSequencesPanel, CBaseSequencesPanel )
	EVT_COMBOBOX( wxID_SEQUENCE_EVENT, CSequencesPanel::EventChanged )
	EVT_CHECKBOX( wxID_SEQUENCE_PLAYSOUND, CSequencesPanel::PlaySoundChanged )
wxEND_EVENT_TABLE()

CSequencesPanel::CSequencesPanel( wxWindow* pParent, CHLMV* const pHLMV )
	: CBaseSequencesPanel( pParent, "Sequences", pHLMV )
{
	wxWindow* const pElemParent = GetBox();

	wxStaticText* pEvents = new wxStaticText( pElemParent, wxID_ANY, "Events" );

	m_pEvent = new wxComboBox( pElemParent, wxID_SEQUENCE_EVENT, "" );
	m_pEvent->SetEditable( false );

	m_pPlaySound = new wxCheckBox( pElemParent, wxID_SEQUENCE_PLAYSOUND, "Play Sound" );

	m_pEventInfo = new wxPanel( pElemParent );
	m_pEventInfo->SetSize( wxSize( 200, wxDefaultSize.GetY() ) );

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

	pEventSizer->Add( m_pFrame, wxSizerFlags().Expand() );
	pEventSizer->Add( m_pEventId, wxSizerFlags().Expand() );
	pEventSizer->Add( m_pOptions, wxSizerFlags().Expand() );
	pEventSizer->Add( m_pType, wxSizerFlags().Expand() );

	pSizer->Add( m_pEventInfo, wxGBPosition( 1, BASESEQUENCES_FIRST_FREE_COL + 1 ), wxGBSpan( 3, 1 ), wxEXPAND | wxRESERVE_SPACE_EVEN_IF_HIDDEN );

	m_pEventInfo->SetSizer( pEventSizer );

	GetBoxSizer()->Add( pSizer );

	m_pEventInfo->Show( false );
}

CSequencesPanel::~CSequencesPanel()
{
}

void CSequencesPanel::InitializeUI()
{
	CBaseSequencesPanel::InitializeUI();

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

void CSequencesPanel::PlaySoundChanged( wxCommandEvent& event )
{
	m_pHLMV->GetState()->playSound = m_pPlaySound->GetValue();
}

void CSequencesPanel::UpdateEvents()
{
	m_pEvent->Clear();

	auto pModel = m_pHLMV->GetState()->GetStudioModel();

	if( !pModel || !pModel->getStudioHeader() )
	{
		UpdateEventInfo( -1 );
		return;
	}

	const studiohdr_t* const pHdr = pModel->getStudioHeader();

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
		m_pEvent->Enable( false );
		m_pEventInfo->Show( false );
		return;
	}

	const studiohdr_t* const pHdr = m_pHLMV->GetState()->GetStudioModel()->getStudioHeader();

	if( !pHdr )
		return;

	const mstudioseqdesc_t& sequence = ( ( mstudioseqdesc_t* ) ( ( byte* ) pHdr + pHdr->seqindex ) )[ m_pSequence->GetSelection() ];

	if( iIndex < 0 || iIndex >= sequence.numevents )
		return;

	m_pEvent->Enable( true );

	const mstudioevent_t& event = ( ( mstudioevent_t* ) ( ( byte* ) pHdr + sequence.eventindex ) )[ iIndex ];

	m_pFrame->SetLabelText( wxString::Format( "Frame: %d", event.frame ) );
	m_pEventId->SetLabelText( wxString::Format( "Event: %d", event.event ) );
	m_pOptions->SetLabelText( wxString::Format( "Options: %s", static_cast<const char*>( event.options ) ) );
	m_pType->SetLabelText( wxString::Format( "Type: %d", event.type ) );

	//Resize it so it doesn't cut off the options text
	m_pEventInfo->Fit();

	m_pEventInfo->Show( true );
}
}