#include <cfloat>

#include <wx/sizer.h>
#include <wx/gbsizer.h>
#include <wx/tglbtn.h>
#include <wx/spinctrl.h>

#include "utility/Color.h"

#include "cvar/CVar.h"

#include "engine/shared/activity.h"

#include "ui/common/CEditEventsDialog.h"

#include "../CModelViewerApp.h"
#include "../../settings/CHLMVSettings.h"
#include "../../CHLMVState.h"

#include "CSequencesPanel.h"

namespace hlmv
{
wxBEGIN_EVENT_TABLE(CSequencesPanel, CBaseControlPanel)
	EVT_CHOICE(wxID_SEQUENCE_SEQCHANGED, CSequencesPanel::SequenceChanged)
	EVT_TOGGLEBUTTON(wxID_SEQUENCE_TOGGLEPLAY, CSequencesPanel::TogglePlay)
	EVT_BUTTON(wxID_SEQUENCE_PREVFRAME, CSequencesPanel::PrevFrame)
	EVT_BUTTON(wxID_SEQUENCE_NEXTFRAME, CSequencesPanel::NextFrame)
	EVT_TEXT(wxID_SEQUENCE_FRAME, CSequencesPanel::FrameChanged)
	EVT_SLIDER(WXID_SEQUENCE_ANIMSPEED, CSequencesPanel::AnimSpeedChanged)
	EVT_CHOICE(wxID_SEQUENCE_EVENT, CSequencesPanel::EventChanged)
	EVT_BUTTON(wxID_SEQUENCE_EDITEVENTS, CSequencesPanel::OnEditEvents)
	EVT_CHECKBOX(wxID_SEQUENCE_PLAYSOUND, CSequencesPanel::PlaySoundChanged)
	EVT_CHECKBOX(wxID_SEQUENCE_PITCHFRAMERATE, CSequencesPanel::PitchFramerateChanged)
wxEND_EVENT_TABLE()

//Must match StudioLoopingMode
static const wxString LoopingModes[] =
{
	"Always Loop",
	"Never Loop",
	"Use Sequence Setting"
};

CSequencesPanel::CSequencesPanel( wxWindow* pParent, CModelViewerApp* const pHLMV )
	: CBaseControlPanel( pParent, "Sequences", pHLMV )
{
	wxWindow* const pElemParent = GetElementParent();

	m_pSequence = new wxChoice(pElemParent, wxID_SEQUENCE_SEQCHANGED, wxDefaultPosition, wxSize(400, wxDefaultSize.GetY()));

	//Validator so it only considers numbers
	wxTextValidator validator(wxFILTER_INCLUDE_CHAR_LIST);

	wxArrayString list;

	for (auto character : "0123456789")
	{
		list.Add(character);
	}

	validator.SetIncludes(list);

	m_pTogglePlayButton = new wxToggleButton(pElemParent, wxID_SEQUENCE_TOGGLEPLAY, "Stop");
	m_pPrevFrameButton = new wxButton(pElemParent, wxID_SEQUENCE_PREVFRAME, "<<");
	m_pSequenceFrame = new wxTextCtrl(pElemParent, wxID_SEQUENCE_FRAME, "", wxDefaultPosition, wxDefaultSize, 0, validator);
	m_pNextFrameButton = new wxButton(pElemParent, wxID_SEQUENCE_NEXTFRAME, ">>");

	m_pAnimSpeed = new wxSlider(pElemParent, WXID_SEQUENCE_ANIMSPEED,
		ANIMSPEED_DEFAULT * ANIMSPEED_SLIDER_MULTIPLIER,
		ANIMSPEED_MIN * ANIMSPEED_SLIDER_MULTIPLIER,
		ANIMSPEED_MAX * ANIMSPEED_SLIDER_MULTIPLIER,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);

	m_pAnimSpeedSpinner = new wxSpinCtrlDouble(pElemParent, wxID_ANY, "0", wxDefaultPosition, wxSize(75, wxDefaultSize.GetHeight()));

	m_pAnimSpeedSpinner->SetRange(ANIMSPEED_MIN, ANIMSPEED_MAX);
	m_pAnimSpeedSpinner->SetIncrement(0.1);

	m_pAnimSpeedSpinner->Bind(wxEVT_SPINCTRLDOUBLE, &CSequencesPanel::AnimSpeedSpinnerChanged, this);

	m_pResetSpeedButton = new wxButton(pElemParent, wxID_ANY, "Reset Speed");

	m_pResetSpeedButton->Bind(wxEVT_BUTTON, &CSequencesPanel::ResetAnimSpeed, this);

	m_pSequenceInfo = new wxPanel(pElemParent);

	m_pSequenceIndex = new wxStaticText(m_pSequenceInfo, wxID_ANY, "Sequence #: 000");
	m_pFrameCount = new wxStaticText(m_pSequenceInfo, wxID_ANY, "Frames: 000");
	m_pFrameRate = new wxStaticText(m_pSequenceInfo, wxID_ANY, "FPS: 000");
	m_pBlends = new wxStaticText(m_pSequenceInfo, wxID_ANY, "Blends: 0");
	m_pEventCount = new wxStaticText(m_pSequenceInfo, wxID_ANY, "# of Events: 000");
	m_pIsLooping = new wxStaticText(m_pSequenceInfo, wxID_ANY, "Is Looping: Yes");
	m_pActivity = new wxStaticText(m_pSequenceInfo, wxID_ANY, "Activity: Undefined");
	m_pActivity->SetMinSize(wxSize(190, wxDefaultSize.GetHeight()));
	m_pActWeight = new wxStaticText(m_pSequenceInfo, wxID_ANY, "Activity Weight: 000");

	//Info
	{
		auto infoSizer = new wxGridBagSizer(1, 1);

		infoSizer->Add(m_pSequenceIndex, wxGBPosition(0, 0), wxDefaultSpan);
		infoSizer->Add(m_pFrameCount, wxGBPosition(1, 0), wxDefaultSpan);
		infoSizer->Add(m_pFrameRate, wxGBPosition(2, 0), wxDefaultSpan);
		infoSizer->Add(m_pBlends, wxGBPosition(3, 0), wxDefaultSpan);
		infoSizer->Add(m_pEventCount, wxGBPosition(4, 0), wxDefaultSpan);
		infoSizer->Add(m_pIsLooping, wxGBPosition(5, 0), wxDefaultSpan);
		infoSizer->Add(m_pActivity, wxGBPosition(0, 1), wxDefaultSpan);
		infoSizer->Add(m_pActWeight, wxGBPosition(1, 1), wxDefaultSpan);

		m_pSequenceInfo->SetSizer(infoSizer);
	}

	SetFrameControlsEnabled(false);

	m_pLoopingMode = new wxChoice(pElemParent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 3, LoopingModes);
	m_pLoopingMode->SetSelection(0);
	m_pLoopingMode->Bind(wxEVT_CHOICE, &CSequencesPanel::OnLoopingModeChanged, this);

	m_pRestartSequence = new wxButton(pElemParent, wxID_ANY, "Restart Sequence");
	m_pRestartSequence->Bind(wxEVT_BUTTON, &CSequencesPanel::OnRestartSequence, this);

	m_pEvent = new wxChoice(pElemParent, wxID_SEQUENCE_EVENT);

	m_pPlaySound = new wxCheckBox(pElemParent, wxID_SEQUENCE_PLAYSOUND, "Play Sound");
	m_pPitchFramerate = new wxCheckBox(pElemParent, wxID_SEQUENCE_PITCHFRAMERATE, "Pitch *= Framerate");

	m_pEditEvents = new wxButton(pElemParent, wxID_SEQUENCE_EDITEVENTS, "Edit Events");

	m_pEventInfo = new wxPanel(pElemParent);
	m_pEventInfo->SetSize(wxSize(200, wxDefaultSize.GetY()));

	m_pFrame = new wxStaticText(m_pEventInfo, wxID_ANY, "Frame: Undefined");
	m_pEventId = new wxStaticText(m_pEventInfo, wxID_ANY, "Event: Undefined");
	m_pOptions = new wxStaticText(m_pEventInfo, wxID_ANY, "Options: Undefined", wxDefaultPosition, wxSize(200, wxDefaultSize.GetHeight()));
	m_pType = new wxStaticText(m_pEventInfo, wxID_ANY, "Type: Undefined");

	//Layout
	auto sizer = new wxGridBagSizer(1, 1);

	{
		auto sequenceSizer = new wxGridBagSizer(1, 1);

		sequenceSizer->Add(new wxStaticText(pElemParent, wxID_ANY, "Animation Sequence"), wxGBPosition(0, 0), wxGBSpan(1, 1), wxALIGN_CENTER_VERTICAL);
		sequenceSizer->Add(m_pSequence, wxGBPosition(1, 0), wxGBSpan(1, 1));

		{
			auto frameSizer = new wxBoxSizer(wxHORIZONTAL);

			frameSizer->Add(m_pTogglePlayButton, wxSizerFlags().Expand());
			frameSizer->Add(m_pPrevFrameButton, wxSizerFlags().Expand());
			frameSizer->Add(m_pSequenceFrame, wxSizerFlags(1).Expand());
			frameSizer->Add(m_pNextFrameButton, wxSizerFlags().Expand());

			sequenceSizer->Add(frameSizer, wxGBPosition(2, 0), wxGBSpan(1, 1), wxEXPAND);
		}

		{
			auto speedSizer = new wxBoxSizer(wxHORIZONTAL);

			speedSizer->Add(new wxStaticText(pElemParent, wxID_ANY, "Speed"), wxSizerFlags().Expand());
			speedSizer->AddSpacer(5);
			speedSizer->Add(m_pAnimSpeed, wxSizerFlags(1).Expand());
			speedSizer->Add(m_pAnimSpeedSpinner, wxSizerFlags().Expand());
			speedSizer->Add(m_pResetSpeedButton, wxSizerFlags().Expand());

			sequenceSizer->Add(speedSizer, wxGBPosition(3, 0), wxGBSpan(1, 1), wxEXPAND);
		}

		sizer->Add(sequenceSizer, wxGBPosition(0, 0), wxGBSpan(1, 1), wxEXPAND);
	}

	{
		auto controlsSizer = new wxGridBagSizer(1, 1);

		controlsSizer->Add(new wxStaticText(pElemParent, wxID_ANY, "Looping Mode"), wxGBPosition(0, 0), wxGBSpan(1, 1));
		controlsSizer->Add(m_pLoopingMode, wxGBPosition(0, 1), wxGBSpan(1, 1));
		controlsSizer->Add(m_pRestartSequence, wxGBPosition(1, 0), wxGBSpan(1, 2), wxEXPAND);

		sizer->Add(controlsSizer, wxGBPosition(0, 1), wxGBSpan(1, 1), wxEXPAND);
	}

	sizer->Add(m_pSequenceInfo, wxGBPosition(0, 2), wxGBSpan(1, 1), wxEXPAND | wxRESERVE_SPACE_EVEN_IF_HIDDEN);

	{
		auto eventsSizer = new wxGridBagSizer(1, 1);

		eventsSizer->Add(new wxStaticText(pElemParent, wxID_ANY, "Events"), wxGBPosition(0, 0), wxGBSpan(1, 1), wxEXPAND);
		eventsSizer->Add(m_pEditEvents, wxGBPosition(0, 1), wxGBSpan(1, 1), wxEXPAND);
		eventsSizer->Add(m_pEvent, wxGBPosition(1, 0), wxGBSpan(1, 2), wxEXPAND);
		eventsSizer->Add(m_pPlaySound, wxGBPosition(2, 0), wxGBSpan(1, 2), wxEXPAND);
		eventsSizer->Add(m_pPitchFramerate, wxGBPosition(3, 0), wxGBSpan(1, 2), wxEXPAND);

		sizer->Add(eventsSizer, wxGBPosition(0, 3), wxDefaultSpan, wxEXPAND);
	}

	{
		auto eventInfoSizer = new wxBoxSizer(wxVERTICAL);

		eventInfoSizer->Add(m_pFrame, wxSizerFlags().Expand());
		eventInfoSizer->Add(m_pEventId, wxSizerFlags().Expand());
		eventInfoSizer->Add(m_pOptions, wxSizerFlags().Expand());
		eventInfoSizer->Add(m_pType, wxSizerFlags().Expand());

		m_pEventInfo->SetSizer(eventInfoSizer);

		sizer->Add(m_pEventInfo, wxGBPosition(0, 5), wxGBSpan(1, 1), wxEXPAND | wxRESERVE_SPACE_EVEN_IF_HIDDEN);
	}

	GetMainSizer()->Add(sizer);

	m_pEventInfo->Show(false);

	g_pCVar->InstallGlobalCVarHandler(this);
}

CSequencesPanel::~CSequencesPanel()
{
	g_pCVar->RemoveGlobalCVarHandler( this );
}

void CSequencesPanel::InitializeUI()
{
	m_pSequence->Clear();

	auto pEntity = m_pHLMV->GetState()->GetEntity();

	bool bSuccess = false;

	if( pEntity )
	{
		pEntity->SetLoopingMode(static_cast<StudioLoopingMode>(m_pLoopingMode->GetSelection()));

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

			bSuccess = true;
		}
	}

	m_pAnimSpeed->SetValue(ANIMSPEED_DEFAULT * ANIMSPEED_SLIDER_MULTIPLIER);
	m_pAnimSpeedSpinner->SetValue(ANIMSPEED_DEFAULT);

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

		wxString activityName{"Unknown"};

		if (sequence.activity >= ACT_IDLE && sequence.activity <= ACT_FLINCH_RIGHTLEG)
		{
			activityName = activity_map[sequence.activity - 1].name;
		}

		m_pSequenceIndex->SetLabelText( wxString::Format( "Sequence #: %d", iIndex ) );
		m_pFrameCount->SetLabelText( wxString::Format( "Frames: %d", sequence.numframes ) );
		m_pFrameRate->SetLabelText( wxString::Format( "FPS: %.2f", sequence.fps ) );
		m_pBlends->SetLabelText( wxString::Format( "Blends: %d", sequence.numblends ) );
		m_pEventCount->SetLabelText( wxString::Format( "# of Events: %d", sequence.numevents ) );
		m_pIsLooping->SetLabelText(wxString::Format("Is Looping: %s", (sequence.flags & STUDIO_LOOPING) ? "Yes" : "No"));
		m_pActivity->SetLabelText(wxString::Format("Activity: %s", activityName));
		m_pActWeight->SetLabelText(wxString::Format("Activity Weight: %d", sequence.actweight));
	}

	UpdateEvents();
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

void CSequencesPanel::SequenceChanged( wxCommandEvent& event )
{
	SetSequence( m_pSequence->GetSelection() );
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
	m_pAnimSpeedSpinner->SetValue(m_pAnimSpeed->GetValue() / static_cast<float>(ANIMSPEED_SLIDER_MULTIPLIER));

	if(auto pEntity = m_pHLMV->GetState()->GetEntity())
	{
		pEntity->SetFrameRate(m_pAnimSpeedSpinner->GetValue());
	}
}

void CSequencesPanel::AnimSpeedSpinnerChanged(wxSpinDoubleEvent& event)
{
	m_pAnimSpeed->SetValue(static_cast<int>(m_pAnimSpeedSpinner->GetValue() * ANIMSPEED_SLIDER_MULTIPLIER));

	if (auto pEntity = m_pHLMV->GetState()->GetEntity())
	{
		pEntity->SetFrameRate(m_pAnimSpeedSpinner->GetValue());
	}
}

void CSequencesPanel::ResetAnimSpeed(wxCommandEvent& event)
{
	if (auto pEntity = m_pHLMV->GetState()->GetEntity())
	{
		m_pAnimSpeed->SetValue(ANIMSPEED_DEFAULT * ANIMSPEED_SLIDER_MULTIPLIER);
		m_pAnimSpeedSpinner->SetValue(ANIMSPEED_DEFAULT);
		pEntity->SetFrameRate(1.f);
	}
}

void CSequencesPanel::OnLoopingModeChanged(wxCommandEvent& event)
{
	if (auto entity = m_pHLMV->GetState()->GetEntity(); entity)
	{
		entity->SetLoopingMode(static_cast<StudioLoopingMode>(m_pLoopingMode->GetSelection()));
	}
}

void CSequencesPanel::OnRestartSequence(wxCommandEvent& event)
{
	if (auto entity = m_pHLMV->GetState()->GetEntity(); entity)
	{
		entity->SetFrame(0);
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
}