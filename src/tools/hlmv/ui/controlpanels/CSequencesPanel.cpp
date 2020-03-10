#include <cfloat>

#include <wx/sizer.h>
#include <wx/gbsizer.h>
#include <wx/tglbtn.h>
#include <wx/spinctrl.h>

#include "utility/Color.h"

#include "cvar/CVar.h"

#include "ui/common/CEditEventsDialog.h"

#include "../CModelViewerApp.h"
#include "../../settings/CHLMVSettings.h"
#include "../../CHLMVState.h"

#include "CSequencesPanel.h"

namespace hlmv
{
wxBEGIN_EVENT_TABLE( CSequencesPanel, CBaseControlPanel )
	EVT_CHOICE( wxID_SEQUENCE_SEQCHANGED, CSequencesPanel::SequenceChanged )
	EVT_TOGGLEBUTTON( wxID_SEQUENCE_TOGGLEPLAY, CSequencesPanel::TogglePlay )
	EVT_BUTTON( wxID_SEQUENCE_PREVFRAME, CSequencesPanel::PrevFrame )
	EVT_BUTTON( wxID_SEQUENCE_NEXTFRAME, CSequencesPanel::NextFrame )
	EVT_TEXT( wxID_SEQUENCE_FRAME, CSequencesPanel::FrameChanged )
	EVT_SLIDER( WXID_SEQUENCE_ANIMSPEED, CSequencesPanel::AnimSpeedChanged )
	EVT_CHOICE( wxID_SEQUENCE_EVENT, CSequencesPanel::EventChanged )
	EVT_BUTTON( wxID_SEQUENCE_EDITEVENTS, CSequencesPanel::OnEditEvents )
	EVT_CHECKBOX( wxID_SEQUENCE_PLAYSOUND, CSequencesPanel::PlaySoundChanged )
	EVT_CHECKBOX( wxID_SEQUENCE_PITCHFRAMERATE, CSequencesPanel::PitchFramerateChanged )
	EVT_SPINCTRLDOUBLE( wxID_SEQUENCE_ORIGIN, CSequencesPanel::OnOriginChanged )
	EVT_BUTTON( wxID_SEQUENCE_ORIGIN, CSequencesPanel::TestOrigin )
wxEND_EVENT_TABLE()

CSequencesPanel::CSequencesPanel( wxWindow* pParent, CModelViewerApp* const pHLMV )
	: CBaseControlPanel( pParent, "Sequences", pHLMV )
{
	wxWindow* const pElemParent = GetElementParent();

	wxStaticText* pSequence = new wxStaticText( pElemParent, wxID_ANY, "Animation Sequence" );

	m_pSequence = new wxChoice( pElemParent, wxID_SEQUENCE_SEQCHANGED, wxDefaultPosition, wxSize( 400, wxDefaultSize.GetY() ) );

	m_pTogglePlayButton = new wxToggleButton( pElemParent, wxID_SEQUENCE_TOGGLEPLAY, "Stop" );
	m_pPrevFrameButton = new wxButton( pElemParent, wxID_SEQUENCE_PREVFRAME, "<<" );

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

	m_pSequenceFrame = new wxTextCtrl( pElemParent, wxID_SEQUENCE_FRAME, "", wxDefaultPosition, wxDefaultSize, 0, validator );
	m_pNextFrameButton = new wxButton( pElemParent, wxID_SEQUENCE_NEXTFRAME, ">>" );

	m_pAnimSpeed = new wxSlider( pElemParent, WXID_SEQUENCE_ANIMSPEED, ANIMSPEED_SLIDER_DEFAULT, ANIMSPEED_SLIDER_MIN, ANIMSPEED_SLIDER_MAX,
								 wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_MIN_MAX_LABELS );

	wxStaticText* pSpeed = new wxStaticText( pElemParent, wxID_ANY, "Speed" );

	m_pResetSpeedButton = new wxButton(pElemParent, wxID_ANY, "Reset Speed");

	m_pResetSpeedButton->Bind(wxEVT_BUTTON, &CSequencesPanel::ResetAnimSpeed, this);

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

	m_pEvent = new wxChoice( pElemParent, wxID_SEQUENCE_EVENT );

	m_pPlaySound = new wxCheckBox( pElemParent, wxID_SEQUENCE_PLAYSOUND, "Play Sound" );
	m_pPitchFramerate = new wxCheckBox( pElemParent, wxID_SEQUENCE_PITCHFRAMERATE, "Pitch *= Framerate" );

	m_pEditEvents = new wxButton( pElemParent, wxID_SEQUENCE_EDITEVENTS, "Edit Events" );

	m_pEventInfo = new wxPanel( pElemParent );
	m_pEventInfo->SetSize( wxSize( 200, wxDefaultSize.GetY() ) );

	m_pFrame	= new wxStaticText( m_pEventInfo, wxID_ANY, "Frame: Undefined" );
	m_pEventId	= new wxStaticText( m_pEventInfo, wxID_ANY, "Event: Undefined" );
	m_pOptions	= new wxStaticText( m_pEventInfo, wxID_ANY, "Options: Undefined", wxDefaultPosition, wxSize( 200, wxDefaultSize.GetHeight() ) );
	m_pType		= new wxStaticText( m_pEventInfo, wxID_ANY, "Type: Undefined" );

	m_pOrigin[ 0 ] = new wxSpinCtrlDouble( pElemParent, wxID_SEQUENCE_ORIGIN, "0" );
	m_pOrigin[ 1 ] = new wxSpinCtrlDouble( pElemParent, wxID_SEQUENCE_ORIGIN, "0" );
	m_pOrigin[ 2 ] = new wxSpinCtrlDouble( pElemParent, wxID_SEQUENCE_ORIGIN, "0" );

	for( size_t uiIndex = 0; uiIndex < 3; ++uiIndex )
	{
		m_pOrigin[ uiIndex ]->SetRange( -DBL_MAX, DBL_MAX );
		m_pOrigin[ uiIndex ]->SetDigits( 6 );
	}

	wxStaticText* pXOrigin = new wxStaticText( pElemParent, wxID_ANY, "Origin X" );
	wxStaticText* pYOrigin = new wxStaticText( pElemParent, wxID_ANY, "Origin Y" );
	wxStaticText* pZOrigin = new wxStaticText( pElemParent, wxID_ANY, "Origin Z" );

	m_pTestOrigins = new wxButton( pElemParent, wxID_SEQUENCE_ORIGIN, "Test Origin" );

	m_pShowCrosshair = new wxCheckBox( pElemParent, wxID_ANY, "Show Crosshair" );
	m_pShowGuidelines = new wxCheckBox( pElemParent, wxID_ANY, "Show Guidelines" );

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
	pSizer->Add(m_pResetSpeedButton, wxGBPosition(3, iCol + 4), wxDefaultSpan, wxEXPAND);

	iCol += 5;

	pSizer->Add( m_pSequenceInfo, wxGBPosition( 1, iCol++ ), wxGBSpan( 3, 1 ), wxEXPAND | wxRESERVE_SPACE_EVEN_IF_HIDDEN );

	pSizer->Add( pEvents, wxGBPosition( 0, iCol ), wxDefaultSpan, wxEXPAND );
	pSizer->Add( m_pEvent, wxGBPosition( 1, iCol ), wxDefaultSpan, wxEXPAND );
	pSizer->Add( m_pPlaySound, wxGBPosition( 2, iCol ), wxDefaultSpan, wxEXPAND );
	pSizer->Add( m_pPitchFramerate, wxGBPosition( 3, iCol++ ), wxDefaultSpan, wxEXPAND );

	pSizer->Add( m_pEditEvents, wxGBPosition( 0, iCol ), wxGBSpan( 1, 2 ), wxEXPAND );

	wxBoxSizer* pEventSizer = new wxBoxSizer( wxVERTICAL );

	pEventSizer->Add( m_pFrame, wxSizerFlags().Expand() );
	pEventSizer->Add( m_pEventId, wxSizerFlags().Expand() );
	pEventSizer->Add( m_pOptions, wxSizerFlags().Expand() );
	pEventSizer->Add( m_pType, wxSizerFlags().Expand() );

	pSizer->Add( m_pEventInfo, wxGBPosition( 1, iCol ), wxGBSpan( 3, 2 ), wxEXPAND | wxRESERVE_SPACE_EVEN_IF_HIDDEN );

	iCol += 2;

	pSizer->Add( m_pOrigin[ 0 ], wxGBPosition( 0, iCol ), wxDefaultSpan, wxEXPAND );
	pSizer->Add( m_pOrigin[ 1 ], wxGBPosition( 1, iCol ), wxDefaultSpan, wxEXPAND );
	pSizer->Add( m_pOrigin[ 2 ], wxGBPosition( 2, iCol ), wxDefaultSpan, wxEXPAND );

	pSizer->Add( m_pTestOrigins, wxGBPosition( 3, iCol++ ), wxDefaultSpan, wxEXPAND );

	pSizer->Add( pXOrigin, wxGBPosition( 0, iCol ), wxDefaultSpan, wxEXPAND );
	pSizer->Add( pYOrigin, wxGBPosition( 1, iCol ), wxDefaultSpan, wxEXPAND );
	pSizer->Add( pZOrigin, wxGBPosition( 2, iCol++ ), wxDefaultSpan, wxEXPAND );

	pSizer->Add( m_pShowCrosshair, wxGBPosition( 0, iCol ), wxDefaultSpan, wxEXPAND );
	pSizer->Add( m_pShowGuidelines, wxGBPosition( 1, iCol ), wxDefaultSpan, wxEXPAND );

	m_pEventInfo->SetSizer( pEventSizer );

	GetMainSizer()->Add( pSizer );

	m_pEventInfo->Show( false );

	g_pCVar->InstallGlobalCVarHandler( this );
}

CSequencesPanel::~CSequencesPanel()
{
	g_pCVar->RemoveGlobalCVarHandler( this );
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

	auto pEntity = m_pHLMV->GetState()->GetEntity();

	bool bSuccess = false;

	m_RootBonePositions.clear();

	if( pEntity )
	{
		auto pModel = pEntity->GetModel();

		const studiohdr_t* const pHdr = pModel->GetStudioHeader();

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

			SetSequence( 0 );

			m_pSequenceInfo->Show( true );

			for (auto value : m_pOrigin)
			{
				value->SetValue(0);
			}

			auto rootBones = pModel->GetRootBones();

			for (auto rootBone : rootBones)
			{
				m_RootBonePositions.emplace_back(
					RootBoneData
					{
						rootBone,
						glm::vec3
						{
							rootBone->value[0],
							rootBone->value[1],
							rootBone->value[2]
						}
					});
			}

			bSuccess = true;
		}
	}

	m_pAnimSpeed->SetValue( ANIMSPEED_SLIDER_DEFAULT );

	if( !bSuccess )
	{
		m_pSequenceInfo->Show( false );
	}

	UpdateEvents();

	this->Enable(bSuccess);
}

void CSequencesPanel::SetSequence( int iIndex )
{
	if( auto pEntity = m_pHLMV->GetState()->GetEntity() )
	{
		auto pModel = pEntity->GetModel();

		const studiohdr_t* const pHdr = pModel->GetStudioHeader();

		if( !pHdr )
		{
			return;
		}

		const mstudioseqdesc_t* const pseqdescs = ( mstudioseqdesc_t* ) ( ( byte* ) pHdr + pHdr->seqindex );

		if( iIndex < 0 || iIndex >= pHdr->numseq )
			iIndex = 0;

		m_pSequence->Select( iIndex );

		pEntity->SetSequence( iIndex );

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
	auto pEntity = m_pHLMV->GetState()->GetEntity();

	if( pEntity )
	{
		if( iFrame < 0 )
			iFrame = pEntity->GetNumFrames();
		else if( iFrame >= pEntity->GetNumFrames() )
			iFrame = 0;

		pEntity->SetFrame( iFrame );
	}

	long iPos = m_pSequenceFrame->GetInsertionPoint();

	m_pSequenceFrame->ChangeValue( wxString::Format( "%d", iFrame ) );

	const long iLast = m_pSequenceFrame->GetLastPosition();

	if( iPos > iLast )
		iPos = iLast > 0 ? iLast - 1 : 0;

	m_pSequenceFrame->SetInsertionPoint( iPos );
}

void CSequencesPanel::HandleCVar( cvar::CCVar& cvar, const char* pszOldValue, float flOldValue )
{
	if( strcmp( cvar.GetName(), "s_ent_playsounds" ) == 0 )
	{
		m_pPlaySound->SetValue( cvar.GetBool() );
	}
	else if( strcmp( cvar.GetName(), "s_ent_pitchframerate" ) == 0 )
	{
		m_pPitchFramerate->SetValue( cvar.GetBool() );
	}
}

void CSequencesPanel::SetFrameControlsEnabled( const bool bState )
{
	m_pPrevFrameButton->Enable( bState );
	m_pSequenceFrame->Enable( bState );
	m_pNextFrameButton->Enable( bState );

	if( bState )
	{
		auto pEntity = m_pHLMV->GetState()->GetEntity();

		SetFrame( static_cast<int>( pEntity ? pEntity->GetFrame() : 0 ) );
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

	auto pEntity = m_pHLMV->GetState()->GetEntity();

	if( !pEntity )
	{
		UpdateEventInfo( -1 );
		return;
	}

	auto pModel = pEntity->GetModel();

	const studiohdr_t* const pHdr = pModel->GetStudioHeader();

	const mstudioseqdesc_t& sequence = ( ( mstudioseqdesc_t* ) ( ( byte* ) pHdr + pHdr->seqindex ) )[ m_pSequence->GetSelection() ];

	m_pEvent->Enable( sequence.numevents > 0 );

	if( sequence.numevents > 0 )
	{
		char szBuffer[ 64 ];

		for( int i = 0; i < sequence.numevents; ++i )
		{
			if( !PrintfSuccess( snprintf( szBuffer, sizeof( szBuffer ), "Event %d", i + 1 ), sizeof( szBuffer ) ) )
				szBuffer[ 0 ] = '\0';

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
	auto pEntity = m_pHLMV->GetState()->GetEntity();

	if( iIndex == -1 || !pEntity )
	{
		m_pEvent->Enable( false );
		m_pEventInfo->Show( false );
		return;
	}

	auto pModel = pEntity->GetModel();

	const studiohdr_t* const pHdr = pModel->GetStudioHeader();

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

void CSequencesPanel::UpdateOrigin()
{
	if (auto pEntity = m_pHLMV->GetState()->GetEntity())
	{
		if (!m_RootBonePositions.empty())
		{
			const glm::vec3 offset{m_pOrigin[0]->GetValue(), m_pOrigin[1]->GetValue(), m_pOrigin[2]->GetValue()};

			for (auto& data : m_RootBonePositions)
			{
				const auto newPosition = data.OriginalRootBonePosition + offset;

				data.Bone->value[0] = newPosition.x;
				data.Bone->value[1] = newPosition.y;
				data.Bone->value[2] = newPosition.z;
			}

			m_pHLMV->GetState()->modelChanged = true;
		}
	}
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
	auto pEntity = m_pHLMV->GetState()->GetEntity();

	if( !pEntity )
		return;

	SetFrame( pEntity->GetFrame() - 1 );
}

void CSequencesPanel::NextFrame( wxCommandEvent& event )
{
	auto pEntity = m_pHLMV->GetState()->GetEntity();

	if( !pEntity )
		return;

	SetFrame( pEntity->GetFrame() + 1 );
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
	if( auto pEntity = m_pHLMV->GetState()->GetEntity() )
	{
		pEntity->SetFrameRate( m_pAnimSpeed->GetValue() / static_cast<float>( ANIMSPEED_SLIDER_DEFAULT ) );
	}
}

void CSequencesPanel::ResetAnimSpeed(wxCommandEvent& event)
{
	if (auto pEntity = m_pHLMV->GetState()->GetEntity())
	{
		m_pAnimSpeed->SetValue(ANIMSPEED_SLIDER_DEFAULT);
		pEntity->SetFrameRate(1.f);
	}
}

void CSequencesPanel::EventChanged( wxCommandEvent& event )
{
	UpdateEventInfo( m_pEvent->GetSelection() );
}

void CSequencesPanel::OnEditEvents( wxCommandEvent& event )
{
	auto pEntity = m_pHLMV->GetState()->GetEntity();

	if( !pEntity )
	{
		return;
	}

	auto pModel = pEntity->GetModel();

	const studiohdr_t* const pHdr = pModel->GetStudioHeader();

	if( !pHdr )
		return;

	const mstudioseqdesc_t& sequence = *pHdr->GetSequence( m_pSequence->GetSelection() );

	ui::CEditEventsDialog dlg( this, wxID_ANY, "Edit Events", ( mstudioevent_t* ) ( pHdr->GetData() + sequence.eventindex ), static_cast<size_t>( sequence.numevents ) );

	//Start on the current event for convenience.
	dlg.SetEvent( m_pEvent->GetSelection() );

	//Note: if the user clicks on apply and then cancel, we should still update our state.
	dlg.ShowModal();

	//Current event may have been updated.
	UpdateEventInfo( m_pEvent->GetSelection() );

	if( dlg.ChangesSaved() )
		m_pHLMV->GetState()->modelChanged = true;
}

void CSequencesPanel::PlaySoundChanged( wxCommandEvent& event )
{
	g_pCVar->Command( wxString::Format( "s_ent_playsounds %d", m_pPlaySound->GetValue() ? 1 : 0 ).c_str() );
}

void CSequencesPanel::PitchFramerateChanged( wxCommandEvent& event )
{
	g_pCVar->SetCVarFloat( "s_ent_pitchframerate", m_pPitchFramerate->GetValue() ? 1 : 0 );
}

void CSequencesPanel::OnOriginChanged( wxSpinDoubleEvent& event )
{
	UpdateOrigin();
}

void CSequencesPanel::TestOrigin( wxCommandEvent& event )
{
	UpdateOrigin();
}
}