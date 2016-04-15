#ifndef CONTROLPANELS_CSEQUENCESPANEL_H
#define CONTROLPANELS_CSEQUENCESPANEL_H

#include "CBaseSequencesPanel.h"

namespace hlmv
{
class CSequencesPanel final : public CBaseSequencesPanel
{
public:
	CSequencesPanel( wxWindow* pParent, CHLMV* const pHLMV );
	~CSequencesPanel();

	void InitializeUI() override;

	void SequenceChanged( wxCommandEvent& event ) override;

	void EventChanged( wxCommandEvent& event );

protected:
	wxDECLARE_EVENT_TABLE();

private:
	void PlaySoundChanged( wxCommandEvent& event );

	void UpdateEvents();

	void UpdateEventInfo( int iIndex );

private:
	wxComboBox* m_pEvent;

	wxCheckBox* m_pPlaySound;

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