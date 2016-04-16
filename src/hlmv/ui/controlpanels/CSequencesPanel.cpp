#include <wx/sizer.h>
#include <wx/gbsizer.h>
#include <wx/tglbtn.h>

#include "utility/Color.h"

#include "studiomodel/StudioModel.h"

#include "hlmv/ui/CHLMV.h"
#include "hlmv/settings/CHLMVSettings.h"
#include "hlmv/CHLMVState.h"

#include "CSequencesPanel.h"

namespace hlmv
{
wxBEGIN_EVENT_TABLE( CSequencesPanel, CBaseControlPanel )
	EVT_COMBOBOX( wxID_BASESEQUENCE_SEQCHANGED, CSequencesPanel::SequenceChanged )
	EVT_TOGGLEBUTTON( wxID_BASESEQUENCE_TOGGLEPLAY, CSequencesPanel::TogglePlay )
	EVT_BUTTON( wxID_BASESEQUENCE_PREVFRAME, CSequencesPanel::PrevFrame )
	EVT_BUTTON( wxID_BASESEQUENCE_NEXTFRAME, CSequencesPanel::NextFrame )
	EVT_TEXT( wxID_BASESEQUENCE_FRAME, CSequencesPanel::FrameChanged )
	EVT_SLIDER( WXID_BASESEQUENCE_ANIMSPEED, CSequencesPanel::AnimSpeedChanged )
	EVT_COMBOBOX( wxID_SEQUENCE_EVENT, CSequencesPanel::EventChanged )
	EVT_CHECKBOX( wxID_SEQUENCE_PLAYSOUND, CSequencesPanel::PlaySoundChanged )
	EVT_BUTTON( wxID_WPN_TESTORIGINS, CSequencesPanel::TestOrigins )
wxEND_EVENT_TABLE()

CSequencesPanel::CSequencesPanel( wxWindow* pParent, CHLMV* const pHLMV )
	: CBaseControlPanel( pParent, "Sequences", pHLMV )
{
	wxWindow* const pElemParent = GetBox();

	wxStaticText* pSequence = new wxStaticText( pElemParent, wxID_ANY, "Animation Sequence" );

	m_pSequence = new wxComboBox( pElemParent, wxID_BASESEQUENCE_SEQCHANGED, "", wxDefaultPosition, wxSize( 400, wxDefaultSize.GetY() ) );

	m_pSequence->SetEditable( false );

	m_pTogglePlayButton = new wxToggleButton( pElemParent, wxID_BASESEQUENCE_TOGGLEPLAY, "Stop" );
	m_pPrevFrameButton = new wxButton( pElemParent, wxID_BASESEQUENCE_PREVFRAME, "<<" );

	//Validator so it only considers numbers
	wxTextValidator validator( wxFILTER_INCLUDE_CHAR_LIST );

	const wxString szCharacters( "0123456789" );

	const size_t uiLength = szCharacters.Length();

	wxArrayString list;

	for( size_t uiIndex = 0; uiIndex < uiLength; ++uiIndex )
	{
		list.Add( szCharacters.GetChar( uiIndex ) );
	}

	validator.SetIncludes( list );

	m_pSequenceFrame = new wxTextCtrl( pElemParent, wxID_BASESEQUENCE_FRAME, "", wxDefaultPosition, wxDefaultSize, 0, validator );
	m_pNextFrameButton = new wxButton( pElemParent, wxID_BASESEQUENCE_NEXTFRAME, ">>" );

	m_pAnimSpeed = new wxSlider( pElemParent, WXID_BASESEQUENCE_ANIMSPEED, ANIMSPEED_SLIDER_DEFAULT, ANIMSPEED_SLIDER_MIN, ANIMSPEED_SLIDER_MAX,
								 wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_MIN_MAX_LABELS );

	wxStaticText* pSpeed = new wxStaticText( pElemParent, wxID_ANY, "Speed" );

	m_pSequenceInfo = new wxPanel( pElemParent );

	m_pSequenceIndex = new wxStaticText( m_pSequenceInfo, wxID_ANY, "Sequence #: Undefined" );
	m_pFrameCount = new wxStaticText( m_pSequenceInfo, wxID_ANY, "Frames: Undefined" );
	m_pFrameRate = new wxStaticText( m_pSequenceInfo, wxID_ANY, "FPS: Undefined" );
	m_pBlends = new wxStaticText( m_pSequenceInfo, wxID_ANY, "Blends: Undefined" );
	m_pEventCount = new wxStaticText( m_pSequenceInfo, wxID_ANY, "# of Events: Undefined" );

	//Info
	wxBoxSizer* pInfoSizer = new wxBoxSizer( wxVERTICAL );

	pInfoSizer->Add( m_pSequenceIndex, 0, wxEXPAND );
	pInfoSizer->Add( m_pFrameCount, 0, wxEXPAND );
	pInfoSizer->Add( m_pFrameRate, 0, wxEXPAND );
	pInfoSizer->Add( m_pBlends, 0, wxEXPAND );
	pInfoSizer->Add( m_pEventCount, 0, wxEXPAND );

	m_pSequenceInfo->SetSizer( pInfoSizer );

	SetFrameControlsEnabled( false );

	wxStaticText* pEvents = new wxStaticText( pElemParent, wxID_ANY, "Events" );

	m_pEvent = new wxComboBox( pElemParent, wxID_SEQUENCE_EVENT, "" );
	m_pEvent->SetEditable( false );

	m_pPlaySound = new wxCheckBox( pElemParent, wxID_SEQUENCE_PLAYSOUND, "Play Sound" );

	m_pEventInfo = new wxPanel( pElemParent );
	m_pEventInfo->SetSize( wxSize( 200, wxDefaultSize.GetY() ) );

	m_pFrame	= new wxStaticText( m_pEventInfo, wxID_ANY, "Frame: Undefined" );
	m_pEventId	= new wxStaticText( m_pEventInfo, wxID_ANY, "Event: Undefined" );
	m_pOptions	= new wxStaticText( m_pEventInfo, wxID_ANY, "Options: Undefined", wxDefaultPosition, wxSize( 200, wxDefaultSize.GetHeight() ) );
	m_pType		= new wxStaticText( m_pEventInfo, wxID_ANY, "Type: Undefined" );

	//TODO: use float only controls here
	m_pXOrigin = new wxTextCtrl( pElemParent, wxID_ANY, "Undefined" );
	m_pYOrigin = new wxTextCtrl( pElemParent, wxID_ANY, "Undefined" );
	m_pZOrigin = new wxTextCtrl( pElemParent, wxID_ANY, "Undefined" );

	wxStaticText* pXOrigin = new wxStaticText( pElemParent, wxID_ANY, "Origin X" );
	wxStaticText* pYOrigin = new wxStaticText( pElemParent, wxID_ANY, "Origin Y" );
	wxStaticText* pZOrigin = new wxStaticText( pElemParent, wxID_ANY, "Origin Z" );

	m_pTestOrigins = new wxButton( pElemParent, wxID_WPN_TESTORIGINS, "Test Origins" );

	m_pShowCrosshair = new wxCheckBox( pElemParent, wxID_ANY, "Show Crosshair" );
	m_pShowGuidelines = new wxCheckBox( pElemParent, wxID_ANY, "Show Guidelines" );

	wxStaticText* pOriginInfo = new wxStaticText( pElemParent, wxID_ANY, "Changes to the origin must be made by altering the $origin line in your model's QC file.",
												  wxDefaultPosition, wxSize( 180, 40 ) );

	//Layout
	wxGridBagSizer* pSizer = new wxGridBagSizer( 5, 5 );

	int iCol = 0;

	pSizer->Add( pSequence, wxGBPosition( 0, iCol ), wxGBSpan( 1, 4 ), wxALIGN_CENTER_VERTICAL );
	pSizer->Add( m_pSequence, wxGBPosition( 1, iCol ), wxGBSpan( 1, 4 ) );

	pSizer->Add( m_pTogglePlayButton, wxGBPosition( 2, iCol ), wxDefaultSpan );
	pSizer->Add( m_pPrevFrameButton, wxGBPosition( 2, iCol + 1 ), wxDefaultSpan );
	pSizer->Add( m_pSequenceFrame, wxGBPosition( 2, iCol + 2 ), wxDefaultSpan );
	pSizer->Add( m_pNextFrameButton, wxGBPosition( 2, iCol + 3 ), wxDefaultSpan );

	pSizer->Add( m_pAnimSpeed, wxGBPosition( 3, iCol ), wxGBSpan( 1, 3 ), wxEXPAND );
	pSizer->Add( pSpeed, wxGBPosition( 3, iCol + 3 ), wxDefaultSpan, wxEXPAND );

	iCol += 4;

	pSizer->Add( m_pSequenceInfo, wxGBPosition( 1, iCol++ ), wxGBSpan( 3, 1 ), wxEXPAND | wxRESERVE_SPACE_EVEN_IF_HIDDEN );

	pSizer->Add( pEvents, wxGBPosition( 0, iCol ), wxDefaultSpan, wxEXPAND );
	pSizer->Add( m_pEvent, wxGBPosition( 1, iCol ), wxDefaultSpan, wxEXPAND );
	pSizer->Add( m_pPlaySound, wxGBPosition( 2, iCol++ ), wxDefaultSpan, wxEXPAND );

	wxBoxSizer* pEventSizer = new wxBoxSizer( wxVERTICAL );

	pEventSizer->Add( m_pFrame, wxSizerFlags().Expand() );
	pEventSizer->Add( m_pEventId, wxSizerFlags().Expand() );
	pEventSizer->Add( m_pOptions, wxSizerFlags().Expand() );
	pEventSizer->Add( m_pType, wxSizerFlags().Expand() );

	pSizer->Add( m_pEventInfo, wxGBPosition( 1, iCol ), wxGBSpan( 3, 2 ), wxEXPAND | wxRESERVE_SPACE_EVEN_IF_HIDDEN );

	iCol += 2;

	pSizer->Add( m_pXOrigin, wxGBPosition( 0, iCol ), wxDefaultSpan, wxEXPAND );
	pSizer->Add( m_pYOrigin, wxGBPosition( 1, iCol ), wxDefaultSpan, wxEXPAND );
	pSizer->Add( m_pZOrigin, wxGBPosition( 2, iCol ), wxDefaultSpan, wxEXPAND );

	pSizer->Add( m_pTestOrigins, wxGBPosition( 3, iCol++ ), wxDefaultSpan, wxEXPAND );

	pSizer->Add( pXOrigin, wxGBPosition( 0, iCol ), wxDefaultSpan, wxEXPAND );
	pSizer->Add( pYOrigin, wxGBPosition( 1, iCol ), wxDefaultSpan, wxEXPAND );
	pSizer->Add( pZOrigin, wxGBPosition( 2, iCol++ ), wxDefaultSpan, wxEXPAND );

	pSizer->Add( m_pShowCrosshair, wxGBPosition( 0, iCol ), wxDefaultSpan, wxEXPAND );
	pSizer->Add( m_pShowGuidelines, wxGBPosition( 1, iCol ), wxDefaultSpan, wxEXPAND );

	pSizer->Add( pOriginInfo, wxGBPosition( 2, iCol ), wxGBSpan( 2, 1 ), wxEXPAND );

	m_pEventInfo->SetSizer( pEventSizer );

	GetBoxSizer()->Add( pSizer );

	m_pEventInfo->Show( false );
}

CSequencesPanel::~CSequencesPanel()
{
}

void CSequencesPanel::Draw3D( const wxSize& size )
{
	const int x = size.GetX() / 2;
	const int y = size.GetY() / 2;

	if( m_pShowCrosshair->GetValue() )
	{
		glMatrixMode( GL_PROJECTION );
		glLoadIdentity();

		glOrtho( 0.0f, ( float ) size.GetX(), ( float ) size.GetY(), 0.0f, 1.0f, -1.0f );

		glMatrixMode( GL_MODELVIEW );
		glPushMatrix();
		glLoadIdentity();

		glDisable( GL_CULL_FACE );

		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

		glDisable( GL_TEXTURE_2D );

		const Color& crosshairColor = m_pHLMV->GetSettings()->GetCrosshairColor();

		glColor4f( crosshairColor.GetRed() / 255.0f, crosshairColor.GetGreen() / 255.0f, crosshairColor.GetBlue() / 255.0f, 1.0 );

		glPointSize( CROSSHAIR_LINE_WIDTH );
		glLineWidth( CROSSHAIR_LINE_WIDTH );

		glBegin( GL_POINTS );

		glVertex2f( x - CROSSHAIR_LINE_WIDTH / 2, y + 1 );

		glEnd();

		glBegin( GL_LINES );

		glVertex2f( x - CROSSHAIR_LINE_START, y );
		glVertex2f( x - CROSSHAIR_LINE_END, y );

		glVertex2f( x + CROSSHAIR_LINE_START, y );
		glVertex2f( x + CROSSHAIR_LINE_END, y );

		glVertex2f( x, y - CROSSHAIR_LINE_START );
		glVertex2f( x, y - CROSSHAIR_LINE_END );

		glVertex2f( x, y + CROSSHAIR_LINE_START );
		glVertex2f( x, y + CROSSHAIR_LINE_END );

		glEnd();

		glPointSize( 1 );
		glLineWidth( 1 );

		glPopMatrix();
	}

	if( m_pShowGuidelines->GetValue() )
	{
		glMatrixMode( GL_PROJECTION );
		glLoadIdentity();

		glOrtho( 0.0f, ( float ) size.GetX(), ( float ) size.GetY(), 0.0f, 1.0f, -1.0f );

		glMatrixMode( GL_MODELVIEW );
		glPushMatrix();
		glLoadIdentity();

		glDisable( GL_CULL_FACE );

		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

		glDisable( GL_TEXTURE_2D );

		const Color& crosshairColor = m_pHLMV->GetSettings()->GetCrosshairColor();

		glColor4f( crosshairColor.GetRed() / 255.0f, crosshairColor.GetGreen() / 255.0f, crosshairColor.GetBlue() / 255.0f, 1.0 );

		glPointSize( GUIDELINES_LINE_WIDTH );
		glLineWidth( GUIDELINES_LINE_WIDTH );

		glBegin( GL_POINTS );

		for( int yPos = size.GetY() - GUIDELINES_LINE_LENGTH; yPos >= y + CROSSHAIR_LINE_END; yPos -= GUIDELINES_OFFSET )
		{
			glVertex2f( x - GUIDELINES_LINE_WIDTH, yPos );
		}

		glEnd();

		glBegin( GL_LINES );

		for( int yPos = size.GetY() - GUIDELINES_LINE_LENGTH - GUIDELINES_POINT_LINE_OFFSET - GUIDELINES_LINE_WIDTH;
			 yPos >= y + CROSSHAIR_LINE_END + GUIDELINES_LINE_LENGTH;
			 yPos -= GUIDELINES_OFFSET )
		{
			glVertex2f( x, yPos );
			glVertex2f( x, yPos - GUIDELINES_LINE_LENGTH );
		}

		glEnd();

		const float flWidth = size.GetY() * ( 16 / 9.0 );

		glLineWidth( GUIDELINES_EDGE_WIDTH );

		glBegin( GL_LINES );

		glVertex2f( ( size.GetX() / 2 ) - ( flWidth / 2 ), 0 );
		glVertex2f( ( size.GetX() / 2 ) - ( flWidth / 2 ), size.GetY() );

		glVertex2f( ( size.GetX() / 2 ) + ( flWidth / 2 ), 0 );
		glVertex2f( ( size.GetX() / 2 ) + ( flWidth / 2 ), size.GetY() );

		glEnd();

		glPointSize( 1 );
		glLineWidth( 1 );

		glPopMatrix();
	}
}

void CSequencesPanel::InitializeUI()
{
	m_pSequence->Clear();

	bool bSuccess = false;

	if( auto pModel = m_pHLMV->GetState()->GetStudioModel() )
	{
		const studiohdr_t* const pHdr = pModel->getStudioHeader();

		if( pHdr )
		{
			m_pSequence->Enable( true );

			const mstudioseqdesc_t* const pseqdescs = ( mstudioseqdesc_t* ) ( ( byte* ) pHdr + pHdr->seqindex );

			//Insert all labels into the array, then append the array to the combo box. This is much faster than appending each label to the combo box directly.
			wxArrayString labels;

			for( int iIndex = 0; iIndex < pHdr->numseq; ++iIndex )
			{
				labels.Add( pseqdescs[ iIndex ].label );
			}

			m_pSequence->Append( labels );

			SetSequence( 0 );

			m_pSequenceInfo->Show( true );

			bSuccess = true;
		}
	}

	m_pAnimSpeed->SetValue( ANIMSPEED_SLIDER_DEFAULT );

	if( !bSuccess )
	{
		m_pSequence->Enable( false );
		m_pSequenceInfo->Show( false );
	}

	UpdateEvents();

	m_pXOrigin->SetLabelText( wxString::Format( "%.6f", m_pHLMV->GetState()->weaponOrigin[ 0 ] ) );
	m_pYOrigin->SetLabelText( wxString::Format( "%.6f", m_pHLMV->GetState()->weaponOrigin[ 1 ] ) );
	m_pZOrigin->SetLabelText( wxString::Format( "%.6f", m_pHLMV->GetState()->weaponOrigin[ 2 ] ) );
}

void CSequencesPanel::SetSequence( int iIndex )
{
	if( auto pModel = m_pHLMV->GetState()->GetStudioModel() )
	{
		const studiohdr_t* const pHdr = pModel->getStudioHeader();

		if( !pHdr )
		{
			return;
		}

		const mstudioseqdesc_t* const pseqdescs = ( mstudioseqdesc_t* ) ( ( byte* ) pHdr + pHdr->seqindex );

		if( iIndex < 0 || iIndex >= pHdr->numseq )
			iIndex = 0;

		m_pSequence->Select( iIndex );

		m_pHLMV->GetState()->sequence = iIndex;

		pModel->SetSequence( m_pHLMV->GetState()->sequence );

		mstudioseqdesc_t nullSeq;

		memset( &nullSeq, 0, sizeof( nullSeq ) );

		const mstudioseqdesc_t& sequence = pHdr->numseq > 0 ? pseqdescs[ iIndex ] : nullSeq;

		m_pSequenceIndex->SetLabelText( wxString::Format( "Sequence #: %d", iIndex ) );
		m_pFrameCount->SetLabelText( wxString::Format( "Frames: %d", sequence.numframes ) );
		m_pFrameRate->SetLabelText( wxString::Format( "FPS: %.2f", sequence.fps ) );
		m_pBlends->SetLabelText( wxString::Format( "Blends: %d", sequence.numblends ) );
		m_pEventCount->SetLabelText( wxString::Format( "# of Events: %d", sequence.numevents ) );
	}
}

void CSequencesPanel::SetFrame( int iFrame )
{
	StudioModel* const pStudioModel = m_pHLMV->GetState()->GetStudioModel();

	if( pStudioModel )
	{
		if( iFrame < 0 )
			iFrame = pStudioModel->GetNumFrames();
		else if( iFrame >= pStudioModel->GetNumFrames() )
			iFrame = 0;

		pStudioModel->SetFrame( iFrame );
	}

	long iPos = m_pSequenceFrame->GetInsertionPoint();

	m_pSequenceFrame->ChangeValue( wxString::Format( "%d", iFrame ) );

	const long iLast = m_pSequenceFrame->GetLastPosition();

	if( iPos > iLast )
		iPos = iLast > 0 ? iLast - 1 : 0;

	m_pSequenceFrame->SetInsertionPoint( iPos );
}

void CSequencesPanel::SetFrameControlsEnabled( const bool bState )
{
	m_pPrevFrameButton->Enable( bState );
	m_pSequenceFrame->Enable( bState );
	m_pNextFrameButton->Enable( bState );

	if( bState )
	{
		auto pModel = m_pHLMV->GetState()->GetStudioModel();

		SetFrame( static_cast<int>( pModel ? pModel->GetFrame() : 0 ) );
	}
	else
	{
		m_pSequenceFrame->SetLabelText( "" );
	}

	m_pTogglePlayButton->SetLabelText( bState ? "Play" : "Stop" );
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

void CSequencesPanel::SequenceChanged( wxCommandEvent& event )
{
	SetSequence( m_pSequence->GetSelection() );

	UpdateEvents();
}

void CSequencesPanel::TogglePlay( wxCommandEvent& event )
{
	m_pHLMV->GetState()->playSequence = !m_pTogglePlayButton->GetValue();

	SetFrameControlsEnabled( !m_pHLMV->GetState()->playSequence );
}

void CSequencesPanel::PrevFrame( wxCommandEvent& event )
{
	auto pModel = m_pHLMV->GetState()->GetStudioModel();

	if( !pModel )
		return;

	SetFrame( pModel->GetFrame() - 1 );
}

void CSequencesPanel::NextFrame( wxCommandEvent& event )
{
	auto pModel = m_pHLMV->GetState()->GetStudioModel();

	if( !pModel )
		return;

	SetFrame( pModel->GetFrame() + 1 );
}

void CSequencesPanel::FrameChanged( wxCommandEvent& event )
{
	const wxString szValue = m_pSequenceFrame->GetValue();

	long iValue;

	if( szValue.ToLong( &iValue ) )
		SetFrame( iValue );
}

void CSequencesPanel::AnimSpeedChanged( wxCommandEvent& event )
{
	if( auto pModel = m_pHLMV->GetState()->GetStudioModel() )
	{
		pModel->SetFrameRate( m_pAnimSpeed->GetValue() / static_cast<float>( ANIMSPEED_SLIDER_DEFAULT ) );
	}
}

void CSequencesPanel::EventChanged( wxCommandEvent& event )
{
	UpdateEventInfo( m_pEvent->GetSelection() );
}

void CSequencesPanel::PlaySoundChanged( wxCommandEvent& event )
{
	m_pHLMV->GetState()->playSound = m_pPlaySound->GetValue();
}

static void GetDoubleFromTextCtrl( wxTextCtrl* const pCtrl, vec_t& flInOutValue )
{
	double flValue = 0;

	if( pCtrl->GetValue().ToDouble( &flValue ) )
		flInOutValue = flValue;
	else
	{
		//Invalid content, reset.
		pCtrl->ChangeValue( wxString::Format( "%.6f", flInOutValue ) );
	}
}

void CSequencesPanel::TestOrigins( wxCommandEvent& event )
{
	GetDoubleFromTextCtrl( m_pXOrigin, m_pHLMV->GetState()->weaponOrigin[ 0 ] );
	GetDoubleFromTextCtrl( m_pYOrigin, m_pHLMV->GetState()->weaponOrigin[ 1 ] );
	GetDoubleFromTextCtrl( m_pZOrigin, m_pHLMV->GetState()->weaponOrigin[ 2 ] );
}
}