#ifndef CONTROLPANELS_CMODELDISPLAYPANEL_H
#define CONTROLPANELS_CMODELDISPLAYPANEL_H

#include "CBaseControlPanel.h"

#include "../../CHLMVState.h"

#include "cvar/CCVar.h"

class wxButton;
class wxSpinCtrlDouble;

namespace ui
{
class CFOVCtrl;
}

namespace hlmv
{
class CModelDisplayPanel final : public CBaseControlPanel, public cvar::ICVarHandler
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
			SHOW_BONES,
			SHOW_ATTACHMENTS,
			SHOW_EYE_POSITION,
			BACKFACE_CULLING,

			SHOW_GROUND,
			MIRROR_ON_GROUND,
			SHOW_BACKGROUND,
			WIREFRAME_OVERLAY,
			DRAW_SHADOWS,
			FIX_SHADOW_Z_FIGHTING,
			SHOW_AXES,
			SHOW_NORMALS,
			SHOW_CROSSHAIR,
			SHOW_GUIDELINES,
			SHOW_PLAYER_HITBOX,

			COUNT,
			LAST = COUNT - 1	//Must be last.
		};

	private:
		CheckBox() = delete;
		CheckBox( const CheckBox& ) = delete;
		CheckBox& operator=( const CheckBox& ) = delete;
	};

	static const size_t NUM_CHECKBOX_COLS = 4;

	static const size_t OPACITY_MIN = 0;
	static const size_t OPACITY_MAX = 100;
	static const size_t OPACITY_DEFAULT = OPACITY_MAX;

public:
	CModelDisplayPanel( wxWindow* pParent, CModelViewerApp* const pHLMV );
	~CModelDisplayPanel();

	void InitializeUI() override;

	void SetRenderMode( RenderMode renderMode );

	//0..100
	void SetOpacity( int iValue, const bool bUpdateSlider = true );

	void SetCheckBox( const CheckBox::Type checkBox, const bool bValue );

protected:
	wxDECLARE_EVENT_TABLE();

private:
	void InternalSetCheckBox( const CheckBox::Type checkBox, const bool bValue, const bool bCameFromChangeEvent );

	void RenderModeChanged( wxCommandEvent& event );

	void OpacityChanged( wxCommandEvent& event );

	void CheckBoxChanged( wxCommandEvent& event );

	void OnMirrorAxis( wxCommandEvent& event );

	void OnFOVChanged( wxCommandEvent& event );

	void OnFOVFPChanged( wxCommandEvent& event );

	void OnCenterOnOrigin(wxCommandEvent& event);

	void OnAlignOnGround(wxCommandEvent& event);

	void HandleCVar( cvar::CCVar& cvar, const char* pszOldValue, float flOldValue ) override final;

private:
	wxChoice* m_pRenderMode;

	wxStaticText* m_pOpacity;
	wxSlider* m_pOpacitySlider;

	wxCheckBox* m_pCheckBoxes[ CheckBox::COUNT ];

	wxCheckBox* m_pMirror[ 3 ];

	ui::CFOVCtrl* m_pFOV;
	ui::CFOVCtrl* m_pFPFOV;

	wxButton* m_pCenterOnOriginButton;
	wxButton* m_pAlignOnGroundButton;

private:
	CModelDisplayPanel( const CModelDisplayPanel& ) = delete;
	CModelDisplayPanel& operator=( const CModelDisplayPanel& ) = delete;
};
}

#endif //CONTROLPANELS_CMODELDISPLAYPANEL_H