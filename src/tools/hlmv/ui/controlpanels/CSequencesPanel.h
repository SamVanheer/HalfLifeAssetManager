#ifndef CONTROLPANELS_CSEQUENCESPANEL_H
#define CONTROLPANELS_CSEQUENCESPANEL_H

#include <vector>

#include "cvar/CCVar.h"

#include "CBaseControlPanel.h"

class wxToggleButton;
class wxSpinCtrlDouble;
class wxSpinDoubleEvent;
struct mstudiobone_t;

namespace hlmv
{
class CSequencesPanel final : public CBaseControlPanel, public cvar::ICVarHandler
{
private:
	static const int ANIMSPEED_MIN = 0;
	static const int ANIMSPEED_MAX = 10;
	static const int ANIMSPEED_DEFAULT = 1;
	static const int ANIMSPEED_SLIDER_MULTIPLIER = 40;

public:
	CSequencesPanel( wxWindow* pParent, CModelViewerApp* const pHLMV );
	~CSequencesPanel();

	void InitializeUI() override;

	void SetSequence( int iIndex );

	void SetFrame( int iFrame );

protected:
	wxDECLARE_EVENT_TABLE();

	virtual void HandleCVar( cvar::CCVar& cvar, const char* pszOldValue, float flOldValue ) override;

private:
	void SetFrameControlsEnabled( const bool bState );

	void UpdateEvents();

	void UpdateEventInfo( int iIndex );

	void SequenceChanged( wxCommandEvent& event );

	void TogglePlay( wxCommandEvent& event );

	void PrevFrame( wxCommandEvent& event );

	void NextFrame( wxCommandEvent& event );

	void FrameChanged( wxCommandEvent& event );

	void AnimSpeedChanged( wxCommandEvent& event );

	void AnimSpeedSpinnerChanged(wxSpinDoubleEvent& event);

	void ResetAnimSpeed(wxCommandEvent& event);

	void OnLoopingModeChanged(wxCommandEvent& event);

	void OnRestartSequence(wxCommandEvent& event);

	void OnSliderChanged(wxCommandEvent& event);

	void OnSpinnerChanged(wxSpinDoubleEvent& event);

	void EventChanged( wxCommandEvent& event );

	void OnEditEvents( wxCommandEvent& event );

	void PlaySoundChanged( wxCommandEvent& event );

	void PitchFramerateChanged( wxCommandEvent& event );

private:
	wxChoice* m_pSequence;

	wxToggleButton* m_pTogglePlayButton;
	wxButton* m_pPrevFrameButton;
	wxTextCtrl* m_pSequenceFrame;
	wxButton* m_pNextFrameButton;

	wxSlider* m_pAnimSpeed;
	wxSpinCtrlDouble* m_pAnimSpeedSpinner;
	wxButton* m_pResetSpeedButton;

	wxPanel* m_pSequenceInfo;

	wxStaticText* m_pSequenceIndex;
	wxStaticText* m_pFrameCount;
	wxStaticText* m_pFrameRate;
	wxStaticText* m_pDuration;
	wxStaticText* m_pBlends;
	wxStaticText* m_pEventCount;
	wxStaticText* m_pIsLooping;
	wxStaticText* m_pActivity;
	wxStaticText* m_pActWeight;

	wxChoice* m_pLoopingMode;
	wxButton* m_pRestartSequence;

	wxSlider* m_pBlendsSliders[2];
	wxSpinCtrlDouble* m_pBlendsSpinners[2];

	float m_BlendsScales[2] = {1.f, 1.f};

	wxChoice* m_pEvent;

	wxCheckBox* m_pPlaySound;
	wxCheckBox* m_pPitchFramerate;

	wxButton* m_pEditEvents;

	wxPanel* m_pEventInfo;

	wxStaticText* m_pFrame;
	wxStaticText* m_pEventId;
	wxStaticText* m_pOptions;
	wxStaticText* m_pType;

private:
	CSequencesPanel( const CSequencesPanel& ) = delete;
	CSequencesPanel& operator=( const CSequencesPanel& ) = delete;
};
}

#endif //CONTROLPANELS_CSEQUENCESPANEL_H