#ifndef CONTROLPANELS_CBODYPARTSPANEL_H
#define CONTROLPANELS_CBODYPARTSPANEL_H

#include "CBaseControlPanel.h"

namespace hlmv
{
class CBodyPartsPanel final : public CBaseControlPanel
{
public:
	static const size_t CONTROLS_ROWS = 3;
	static const size_t CONTROLS_COLS = 5;

	static const size_t CONTROLLER_SLIDER_MIN = 0;
	static const size_t CONTROLLER_SLIDER_MAX = 255;
	static const size_t CONTROLLER_SLIDER_DEFAULT = 128;

	static const size_t COMBOBOX_WIDTH = 200;

public:
	CBodyPartsPanel( wxWindow* pParent, CHLMV* const pHLMV );
	~CBodyPartsPanel();

	void ModelChanged( const StudioModel& model ) override;

	void ViewUpdated() override;

	void BodypartChanged( wxCommandEvent& event );

	void SubmodelChanged( wxCommandEvent& event );

	void SkinChanged( wxCommandEvent& event );

	void ControllerChanged( wxCommandEvent& event );

	void ControllerSliderChanged( wxCommandEvent& event );

	void SetBodypart( int iIndex );

	void SetSubmodel( int iIndex );

	void SetSkin( int iIndex );

	void SetController( int iIndex );

	void SetControllerValue( int iIndex, int iValue );

protected:
	wxDECLARE_EVENT_TABLE();

private:
	void UpdateSubmodels( const int iIndex );

private:
	wxComboBox* m_pBodypart;
	wxComboBox* m_pSubmodel;
	wxComboBox* m_pSkin;

	wxComboBox* m_pController;

	wxSlider* m_pControllerSlider;

	wxStaticText* m_pDrawnPolys;

	//Info text
	wxStaticText* m_pBones;
	wxStaticText* m_pBoneControllers;
	wxStaticText* m_pHitBoxes;
	wxStaticText* m_pSequences;
	wxStaticText* m_pSequenceGroups;

	wxStaticText* m_pTextures;
	wxStaticText* m_pSkinFamilies;
	wxStaticText* m_pBodyparts;
	wxStaticText* m_pAttachments;
	wxStaticText* m_pTransitions;

private:
	CBodyPartsPanel( const CBodyPartsPanel& ) = delete;
	CBodyPartsPanel& operator=( const CBodyPartsPanel& ) = delete;
};
}

#endif //CONTROLPANELS_CBODYPARTSPANEL_H