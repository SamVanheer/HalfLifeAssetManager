#ifndef TOOLS_HLMV_UI_CONTROLPANELS_CGLOBALFLAGSPANEL_H
#define TOOLS_HLMV_UI_CONTROLPANELS_CGLOBALFLAGSPANEL_H

#include "CBaseControlPanel.h"

#include "../../CHLMVState.h"

namespace hlmv
{
class CGlobalFlagsPanel final : public CBaseControlPanel
{
public:
	//Weakly typed, strongly scoped.
	class CheckBox
	{
	public:
		enum Type
		{
			FIRST = 0,
			ROCKET = FIRST,
			GRENADE,
			GIB,
			ROTATE,
			TRACER,
			ZOMGIB,
			TRACER2,
			TRACER3,
			NOSHADELIGHT,
			HITBOXCOLLISIONS,
			FORCESKYLIGHT,

			COUNT,
			LAST = COUNT - 1	//Must be last.
		};

	private:
		CheckBox() = delete;
		CheckBox( const CheckBox& ) = delete;
		CheckBox& operator=( const CheckBox& ) = delete;
	};

	static const size_t CONTROLS_ROW_GAP = 5;
	static const size_t CONTROLS_COL_GAP = 5;

	static const size_t NUM_CHECKBOX_COLS = 3;

public:
	CGlobalFlagsPanel( wxWindow* pParent, CModelViewerApp* const pHLMV );
	~CGlobalFlagsPanel();

	void InitializeUI() override;

	void SetCheckBox( const CheckBox::Type checkBox, const bool bValue );

protected:
	wxDECLARE_EVENT_TABLE();

private:
	void InternalSetCheckBox( const CheckBox::Type checkBox, const bool bValue, const bool bCameFromChangeEvent );

	void CheckBoxChanged( wxCommandEvent& event );

private:
	wxCheckBox* m_pCheckBoxes[ CheckBox::COUNT ];

private:
	CGlobalFlagsPanel( const CGlobalFlagsPanel& ) = delete;
	CGlobalFlagsPanel& operator=( const CGlobalFlagsPanel& ) = delete;
};
}

#endif //TOOLS_HLMV_UI_CONTROLPANELS_CGLOBALFLAGSPANEL_H
