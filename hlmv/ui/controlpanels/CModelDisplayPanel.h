#ifndef CONTROLPANELS_CMODELDISPLAYPANEL_H
#define CONTROLPANELS_CMODELDISPLAYPANEL_H

#include "CBaseControlPanel.h"

#include "hlmv/CHLMVOptions.h"

class CModelDisplayPanel final : public CBaseControlPanel
{
public:
	//Weakly typed, strongly scoped.
	class CheckBox
	{
	public:
		enum Type
		{
			FIRST = 0,
			SHOW_HITBOXES = FIRST,
			SHOW_GROUND,
			SHOW_BONES,
			MIRROR_ON_GROUND,
			SHOW_ATTACHMENTS,
			SHOW_BACKGROUND,
			SHOW_EYE_POSITION,
			WIREFRAME_OVERLAY,

			COUNT,
			LAST = COUNT - 1	//Must be last.
		};

	private:
		CheckBox() = delete;
		CheckBox( const CheckBox& ) = delete;
		CheckBox& operator=( const CheckBox& ) = delete;
	};

	static const size_t CONTROLS_ROWS = 4;
	static const size_t CONTROLS_COLS = 5;

	static const size_t CONTROLS_ROW_GAP = 5;
	static const size_t CONTROLS_COL_GAP = 5;

	static const size_t NUM_CHECKBOXES_PER_ROW = 2;

	static const size_t OPACITY_MIN = 0;
	static const size_t OPACITY_MAX = 100;
	static const size_t OPACITY_DEFAULT = OPACITY_MAX;

public:
	CModelDisplayPanel( wxWindow* pParent );
	~CModelDisplayPanel();

	void ModelChanged( const StudioModel& model ) override;

	void ViewUpdated() override;

	void RenderModeChanged( wxCommandEvent& event );

	void OpacityChanged( wxCommandEvent& event );

	void CheckBoxChanged( wxCommandEvent& event );

	void ScaleMesh( wxCommandEvent& event );

	void ScaleBones( wxCommandEvent& event );

	void SetRenderMode( RenderMode renderMode );

	//0..100
	void SetOpacity( int iValue, const bool bUpdateSlider = true );

protected:
	wxDECLARE_EVENT_TABLE();

private:
	wxComboBox* m_pRenderMode;

	wxStaticText* m_pOpacity;
	wxSlider* m_pOpacitySlider;

	wxCheckBox* m_pCheckBoxes[ CheckBox::COUNT ];

	wxTextCtrl* m_pMeshScale;
	wxButton* m_pMeshScaleButton;

	wxTextCtrl* m_pBonesScale;
	wxButton* m_pBonesScaleButton;

	wxStaticText* m_pDrawnPolys;

	unsigned int m_uiDrawnPolysLast = 0;

private:
	CModelDisplayPanel( const CModelDisplayPanel& ) = delete;
	CModelDisplayPanel& operator=( const CModelDisplayPanel& ) = delete;
};

#endif //CONTROLPANELS_CMODELDISPLAYPANEL_H