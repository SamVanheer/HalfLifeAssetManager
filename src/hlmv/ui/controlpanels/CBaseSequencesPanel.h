#ifndef CONTROLPANELS_CBASESEQUENCESPANEL_H
#define CONTROLPANELS_CBASESEQUENCESPANEL_H

#include "CBaseControlPanel.h"

class wxGridBagSizer;
class wxToggleButton;

namespace hlmv
{
class CBaseSequencesPanel : public CBaseControlPanel
{
public:
	//Divided by 10 to get a multiplier
	static const size_t ANIMSPEED_SLIDER_MIN		= 0;
	static const size_t ANIMSPEED_SLIDER_MAX		= 200;
	static const size_t ANIMSPEED_SLIDER_DEFAULT	= 40;

	static const size_t BASESEQUENCES_FIRST_FREE_ROW = 4;
	static const size_t BASESEQUENCES_FIRST_FREE_COL = 5;

public:
	virtual ~CBaseSequencesPanel();

	virtual void ModelChanged( const StudioModel& model ) override;

	virtual void SequenceChanged( wxCommandEvent& event );

	void TogglePlay( wxCommandEvent& event );

	void PrevFrame( wxCommandEvent& event );

	void NextFrame( wxCommandEvent& event );

	void FrameChanged( wxCommandEvent& event );

	void AnimSpeedChanged( wxCommandEvent& event );

	void SetSequence( int iIndex );

	void SetFrame( int iFrame );

protected:
	wxDECLARE_EVENT_TABLE();

	CBaseSequencesPanel( wxWindow* pParent, const wxString& szName, CHLMV* const pHLMV );

	void CreateUI( wxGridBagSizer* pSizer );

private:

	void SetFrameControlsEnabled( const bool bState );

protected:
	wxComboBox* m_pSequence;

	wxToggleButton* m_pTogglePlayButton;
	wxButton* m_pPrevFrameButton;
	wxTextCtrl* m_pFrame;
	wxButton* m_pNextFrameButton;

	wxSlider* m_pAnimSpeed;

	wxStaticText* m_pSequenceIndex;
	wxStaticText* m_pFrameCount;
	wxStaticText* m_pFrameRate;
	wxStaticText* m_pBlends;
	wxStaticText* m_pEventCount;

private:
	CBaseSequencesPanel( const CBaseSequencesPanel& ) = delete;
	CBaseSequencesPanel& operator=( const CBaseSequencesPanel& ) = delete;
};
}

#endif //CONTROLPANELS_CBASESEQUENCESPANEL_H