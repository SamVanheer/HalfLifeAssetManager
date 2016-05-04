#ifndef HLMV_UI_CONTROLPANELS_CBONESPANEL_H
#define HLMV_UI_CONTROLPANELS_CBONESPANEL_H

#include "engine/shared/studiomodel/studio.h"

#include "CBaseControlPanel.h"

namespace hlmv
{
/**
*	@brief Bones control panel for studio models.
*/
class CBonesPanel final : public CBaseControlPanel
{
public:
	/**
	*	Constructor.
	*	@param pParent Parent window.
	*	@param pHLMV HLMV instance.
	*/
	CBonesPanel( wxWindow* pParent, CHLMV* const pHLMV );

	/**
	*	Destructor.
	*/
	~CBonesPanel();

	void InitializeUI() override final;

	void SetBone( int iIndex );

	void SetBoneController( int iIndex );

protected:
	wxDECLARE_EVENT_TABLE();

private:
	void OnBoneChanged( wxCommandEvent& event );

	void OnShowBoneController( wxCommandEvent& event );

	void OnBoneControllerChanged( wxCommandEvent& event );

	void OnShowBone( wxCommandEvent& event );

	void OnShowTypesCheatSheet( wxCommandEvent& event );

private:
	wxComboBox* m_pBones;

	wxPanel* m_pBoneInfo;

	wxStaticText* m_pBoneName;
	wxStaticText* m_pParentBone;
	wxStaticText* m_pBoneFlags;
	wxStaticText* m_Position;
	wxStaticText* m_pPositionScale;
	wxStaticText* m_pRotation;
	wxStaticText* m_pRotationScale;

	wxComboBox* m_pBoneBoneControllers;
	wxButton* m_pShowBoneControllerInfo;

	wxComboBox* m_pBoneControllers;

	wxButton* m_pShowBoneInfo;

	wxPanel* m_pBoneControllerInfo;

	wxStaticText* m_pControllerBone;
	wxStaticText* m_pType;
	wxStaticText* m_pStart;
	wxStaticText* m_pEnd;
	wxStaticText* m_pRest;
	wxStaticText* m_pIndex;

	wxListBox* m_pTypes;

	wxButton* m_pTypesCheatSheet;

private:
	CBonesPanel( const CBonesPanel& ) = delete;
	CBonesPanel& operator=( const CBonesPanel& ) = delete;
};
}

#endif //HLMV_UI_CONTROLPANELS_CBONESPANEL_H