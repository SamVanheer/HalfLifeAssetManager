#ifndef UI_WX_SHARED_CFOVCTRL_H
#define UI_WX_SHARED_CFOVCTRL_H

#include "../wxInclude.h"

namespace ui
{
enum FOVCtrlStyle
{
	FOV_DEFAULT_STYLE = 0
};

extern const wxString FOVCtrlNameStr;

wxDECLARE_EVENT( wxEVT_FOV_CHANGED, wxCommandEvent );

#define EVT_FOV_CHANGED( winid, func ) wx__DECLARE_EVT1(ui::wxEVT_FOV_CHANGED, winid, wxCommandEventHandler(func))

/**
*	A control that represents a Field Of View value. Contains buttons to set and reset the FOV.
*/
class CFOVCtrl : public wxPanel
{
public:
	CFOVCtrl( wxWindow *parent,
			  wxWindowID winid,
			  const float flDefault,
			  const wxString& szLabelText,
			  const wxPoint& pos = wxDefaultPosition,
			  const wxSize& size = wxDefaultSize,
			  long style = FOV_DEFAULT_STYLE,
			  const wxString& name = FOVCtrlNameStr );

	virtual ~CFOVCtrl();

	/**
	*	Gets the default FOV value.
	*/
	float GetDefault() const { return m_flDefault; }

	/**
	*	Sets the default FOV value.
	*	@param flDefault Default value.
	*/
	void SetDefault( const float flDefault );

	/**
	*	Resets the FOV to the default value. This will not generate an EVT_FOV_CHANGED event.
	*/
	void ChangeToDefault();

	/**
	*	Resets the FOV to the default value. This will generate an EVT_FOV_CHANGED event.
	*/
	void ResetToDefault();

	/**
	*	Gets the current value.
	*/
	float GetValue() const;

	/**
	*	Sets the current value. This will not generate an EVT_FOV_CHANGED event.
	*	@param flValue Value to set.
	*/
	void ChangeValue( const float flValue );

	/**
	*	Sets the current value. This will generate an EVT_FOV_CHANGED event.
	*	@param flValue Value to set.
	*/
	void SetValue( const float flValue );

protected:
	wxDECLARE_EVENT_TABLE();

private:
	void ValueChanged();

	void OnSetFOV( wxCommandEvent& event );

	void OnSetDefault( wxCommandEvent& event );

private:
	wxStaticText*	m_pText;
	wxTextCtrl*		m_pValue;
	wxButton*		m_pSet;
	wxButton*		m_pDefault;

	float m_flDefault;

private:
	CFOVCtrl( const CFOVCtrl& ) = delete;
	CFOVCtrl& operator=( const CFOVCtrl& ) = delete;
};
}

#endif //UI_WX_SHARED_CFOVCTRL_H