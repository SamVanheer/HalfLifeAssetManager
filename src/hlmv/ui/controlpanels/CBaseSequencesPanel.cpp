#include <wx/sizer.h>
#include <wx/gbsizer.h>
#include <wx/tglbtn.h>

#include "hlmv/ui/CHLMV.h"
#include "hlmv/CHLMVState.h"
#include "studiomodel/StudioModel.h"

#include "CBaseSequencesPanel.h"

wxBEGIN_EVENT_TABLE( CBaseSequencesPanel, wxPanel )
	EVT_COMBOBOX( wxID_BASESEQUENCE_SEQCHANGED, CBaseSequencesPanel::SequenceChanged )
	EVT_TOGGLEBUTTON( wxID_BASESEQUENCE_TOGGLEPLAY, CBaseSequencesPanel::TogglePlay )
	EVT_BUTTON( wxID_BASESEQUENCE_PREVFRAME, CBaseSequencesPanel::PrevFrame )
	EVT_BUTTON( wxID_BASESEQUENCE_NEXTFRAME, CBaseSequencesPanel::NextFrame )
	EVT_TEXT( wxID_BASESEQUENCE_FRAME, CBaseSequencesPanel::FrameChanged )
	EVT_SLIDER( WXID_BASESEQUENCE_ANIMSPEED, CBaseSequencesPanel::AnimSpeedChanged )
wxEND_EVENT_TABLE()

CBaseSequencesPanel::CBaseSequencesPanel( wxWindow* pParent, const wxString& szName, hlmv::CHLMV* const pHLMV )
	: CBaseControlPanel( pParent, szName, pHLMV )
	, m_pSequence( nullptr )
	, m_pTogglePlayButton( nullptr )
	, m_pPrevFrameButton( nullptr )
	, m_pFrame( nullptr )
	, m_pNextFrameButton( nullptr )
	, m_pAnimSpeed( nullptr )
	, m_pSequenceIndex( nullptr )
	, m_pFrameCount( nullptr )
	, m_pFrameRate( nullptr )
	, m_pBlends( nullptr )
	, m_pEventCount( nullptr )
{
}

CBaseSequencesPanel::~CBaseSequencesPanel()
{
}

void CBaseSequencesPanel::ModelChanged( const StudioModel& model )
{
	m_pSequence->Clear();

	const studiohdr_t* const pHdr = m_pHLMV->GetState()->GetStudioModel()->getStudioHeader();

	if( pHdr )
	{
		const mstudioseqdesc_t* const pseqdescs = ( mstudioseqdesc_t* ) ( ( byte* ) pHdr + pHdr->seqindex );

		//Insert all labels into the array, then append the array to the combo box. This is much faster than appending each label to the combo box directly.
		wxArrayString labels;

		for( int iIndex = 0; iIndex < pHdr->numseq; ++iIndex )
		{
			labels.Add( pseqdescs[ iIndex ].label );
		}

		m_pSequence->Append( labels );
	}

	SetSequence( 0 );

	m_pAnimSpeed->SetValue( ANIMSPEED_SLIDER_DEFAULT );
}

void CBaseSequencesPanel::SequenceChanged( wxCommandEvent& event )
{
	SetSequence( m_pSequence->GetSelection() );
}

void CBaseSequencesPanel::TogglePlay( wxCommandEvent& event )
{
	m_pHLMV->GetState()->playSequence = !m_pTogglePlayButton->GetValue();

	SetFrameControlsEnabled( !m_pHLMV->GetState()->playSequence );
}

void CBaseSequencesPanel::PrevFrame( wxCommandEvent& event )
{
	SetFrame( m_pHLMV->GetState()->GetStudioModel()->GetFrame() - 1 );
}

void CBaseSequencesPanel::NextFrame( wxCommandEvent& event )
{
	SetFrame( m_pHLMV->GetState()->GetStudioModel()->GetFrame() + 1 );
}

void CBaseSequencesPanel::FrameChanged( wxCommandEvent& event )
{
	const wxString szValue = m_pFrame->GetValue();

	long iValue;

	if( szValue.ToLong( &iValue ) )
		SetFrame( iValue );
}

void CBaseSequencesPanel::AnimSpeedChanged( wxCommandEvent& event )
{
	m_pHLMV->GetState()->speedScale = m_pAnimSpeed->GetValue() / static_cast<float>( ANIMSPEED_SLIDER_DEFAULT );
}

void CBaseSequencesPanel::SetSequence( int iIndex )
{
	const studiohdr_t* const pHdr = m_pHLMV->GetState()->GetStudioModel()->getStudioHeader();

	if( !pHdr )
	{
		return;
	}

	const mstudioseqdesc_t* const pseqdescs = ( mstudioseqdesc_t* ) ( ( byte* ) pHdr + pHdr->seqindex );

	if( iIndex < 0 || iIndex >= pHdr->numseq )
		iIndex = 0;

	m_pSequence->Select( iIndex );

	m_pHLMV->GetState()->sequence = iIndex;

	m_pHLMV->GetState()->GetStudioModel()->SetSequence( m_pHLMV->GetState()->sequence );

	mstudioseqdesc_t nullSeq;

	memset( &nullSeq, 0, sizeof( nullSeq ) );

	const mstudioseqdesc_t& sequence = pHdr->numseq > 0 ? pseqdescs[ iIndex ] : nullSeq;

	m_pSequenceIndex->SetLabelText( wxString::Format( "Sequence #: %d", iIndex ) );
	m_pFrameCount->SetLabelText( wxString::Format( "Frames: %d", sequence.numframes ) );
	m_pFrameRate->SetLabelText( wxString::Format( "FPS: %.2f", sequence.fps ) );
	m_pBlends->SetLabelText( wxString::Format( "Blends: %d", sequence.numblends ) );
	m_pEventCount->SetLabelText( wxString::Format( "# of Events: %d", sequence.numevents ) );
}

void CBaseSequencesPanel::SetFrame( int iFrame )
{
	StudioModel* const pStudioModel = m_pHLMV->GetState()->GetStudioModel();

	if( iFrame < 0 )
		iFrame = pStudioModel->GetNumFrames();
	else if( iFrame >= pStudioModel->GetNumFrames() )
		iFrame = 0;

	pStudioModel->SetFrame( iFrame );

	long iPos = m_pFrame->GetInsertionPoint();

	m_pFrame->ChangeValue( wxString::Format( "%d", iFrame ) );

	const long iLast = m_pFrame->GetLastPosition();

	if( iPos > iLast )
		iPos = iLast > 0 ? iLast - 1 : 0;

	m_pFrame->SetInsertionPoint( iPos );
}

void CBaseSequencesPanel::CreateUI( wxGridBagSizer* pSizer )
{
	wxWindow* const pElemParent = GetBox();

	wxStaticText* pSequence = new wxStaticText( pElemParent, wxID_ANY, "Animation Sequence" );

	m_pSequence = new wxComboBox( pElemParent, wxID_BASESEQUENCE_SEQCHANGED, "", wxDefaultPosition, wxSize( 400, wxDefaultSize.GetY() ) );
	
	m_pSequence->SetEditable( false );

	m_pTogglePlayButton = new wxToggleButton( pElemParent, wxID_BASESEQUENCE_TOGGLEPLAY, "Stop" );
	m_pPrevFrameButton = new wxButton( pElemParent, wxID_BASESEQUENCE_PREVFRAME, "<<" );

	//Validator so it only considers numbers
	wxTextValidator validator(wxFILTER_INCLUDE_CHAR_LIST );

	const wxString szCharacters( "0123456789" );

	const size_t uiLength = szCharacters.Length();

	wxArrayString list;

	for( size_t uiIndex = 0; uiIndex < uiLength; ++uiIndex )
	{
		list.Add( szCharacters.GetChar( uiIndex ) );
	}

	validator.SetIncludes( list );

	m_pFrame = new wxTextCtrl( pElemParent, wxID_BASESEQUENCE_FRAME, "", wxDefaultPosition, wxDefaultSize, 0, validator );
	m_pNextFrameButton = new wxButton( pElemParent, wxID_BASESEQUENCE_NEXTFRAME, ">>" );

	m_pAnimSpeed = new wxSlider( pElemParent, WXID_BASESEQUENCE_ANIMSPEED, ANIMSPEED_SLIDER_DEFAULT, ANIMSPEED_SLIDER_MIN, ANIMSPEED_SLIDER_MAX,
								 wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_MIN_MAX_LABELS );

	wxStaticText* pSpeed = new wxStaticText( pElemParent, wxID_ANY, "Speed" );

	wxPanel* pInfoPanel = new wxPanel( pElemParent );

	m_pSequenceIndex	= new wxStaticText( pInfoPanel, wxID_ANY, "Sequence #: Undefined" );
	m_pFrameCount		= new wxStaticText( pInfoPanel, wxID_ANY, "Frames: Undefined" );
	m_pFrameRate		= new wxStaticText( pInfoPanel, wxID_ANY, "FPS: Undefined" );
	m_pBlends			= new wxStaticText( pInfoPanel, wxID_ANY, "Blends: Undefined" );
	m_pEventCount		= new wxStaticText( pInfoPanel, wxID_ANY, "# of Events: Undefined" );

	pSizer->Add( pSequence, wxGBPosition( 0, 0 ), wxGBSpan( 1, 4 ), wxALIGN_CENTER_VERTICAL );
	pSizer->Add( m_pSequence, wxGBPosition( 1, 0 ), wxGBSpan( 1, 4 ) );

	pSizer->Add( m_pTogglePlayButton, wxGBPosition( 2, 0 ), wxDefaultSpan );
	pSizer->Add( m_pPrevFrameButton, wxGBPosition( 2, 1 ), wxDefaultSpan );
	pSizer->Add( m_pFrame, wxGBPosition( 2, 2 ), wxDefaultSpan );
	pSizer->Add( m_pNextFrameButton, wxGBPosition( 2, 3 ), wxDefaultSpan );

	pSizer->Add( m_pAnimSpeed, wxGBPosition( 3, 0 ), wxGBSpan( 1, 3 ), wxEXPAND );
	pSizer->Add( pSpeed, wxGBPosition( 3, 3 ), wxDefaultSpan, wxEXPAND );

	pSizer->Add( pInfoPanel, wxGBPosition( 1, 4 ), wxGBSpan( 3, 1 ), wxEXPAND );

	//Info
	wxBoxSizer* pInfoSizer = new wxBoxSizer( wxVERTICAL );

	pInfoSizer->Add( m_pSequenceIndex, 0, wxEXPAND );
	pInfoSizer->Add( m_pFrameCount, 0, wxEXPAND );
	pInfoSizer->Add( m_pFrameRate, 0, wxEXPAND );
	pInfoSizer->Add( m_pBlends, 0, wxEXPAND );
	pInfoSizer->Add( m_pEventCount, 0, wxEXPAND );

	pInfoPanel->SetSizer( pInfoSizer );

	SetFrameControlsEnabled( false );
}

void CBaseSequencesPanel::SetFrameControlsEnabled( const bool bState )
{
	m_pPrevFrameButton->Enable( bState );
	m_pFrame->Enable( bState );
	m_pNextFrameButton->Enable( bState );

	if( bState )
	{
		SetFrame( static_cast<int>( m_pHLMV->GetState()->GetStudioModel()->GetFrame() ) );
	}
	else
	{
		m_pFrame->SetLabelText( "" );
	}

	m_pTogglePlayButton->SetLabelText( bState ? "Play" : "Stop" );
}