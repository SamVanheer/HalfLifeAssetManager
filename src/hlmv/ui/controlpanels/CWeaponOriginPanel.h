#ifndef CONTROLPANELS_CWEAPONORIGINPANEL_H
#define CONTROLPANELS_CWEAPONORIGINPANEL_H

#include "CBaseSequencesPanel.h"

class CWeaponOriginPanel final : public CBaseSequencesPanel
{
public:
	static const int CROSSHAIR_LINE_WIDTH			= 3;
	static const int CROSSHAIR_LINE_START			= 5;
	static const int CROSSHAIR_LINE_LENGTH			= 10;
	static const int CROSSHAIR_LINE_END				= CROSSHAIR_LINE_START + CROSSHAIR_LINE_LENGTH;

	static const int GUIDELINES_LINE_WIDTH			= 1;
	static const int GUIDELINES_LINE_LENGTH			= 5;
	static const int GUIDELINES_POINT_LINE_OFFSET	= 2;
	static const int GUIDELINES_OFFSET				= GUIDELINES_LINE_LENGTH + ( GUIDELINES_POINT_LINE_OFFSET * 2 ) + GUIDELINES_LINE_WIDTH;

	static const int GUIDELINES_EDGE_WIDTH			= 4;

public:
	CWeaponOriginPanel( wxWindow* pParent, hlmv::CHLMV* const pHLMV );
	~CWeaponOriginPanel();

	void Draw3D( const wxSize& size ) override;

	void ModelChanged( const StudioModel& model ) override;

	void PanelActivated() override;

	void PanelDeactivated() override;

	void TestOrigins( wxCommandEvent& event );

protected:
	wxDECLARE_EVENT_TABLE();

private:
	wxTextCtrl* m_pXOrigin;
	wxTextCtrl* m_pYOrigin;
	wxTextCtrl* m_pZOrigin;

	wxButton* m_pTestOrigins;

	wxCheckBox* m_pShowCrosshair;
	wxCheckBox* m_pShowGuidelines;

private:
	CWeaponOriginPanel( const CWeaponOriginPanel& ) = delete;
	CWeaponOriginPanel& operator=( const CWeaponOriginPanel& ) = delete;
};

#endif //CONTROLPANELS_CWEAPONORIGINPANEL_H