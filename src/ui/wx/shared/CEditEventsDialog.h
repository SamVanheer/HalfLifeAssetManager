#ifndef UI_WX_SHARED_CEDITEVENTDIALOG_H
#define UI_WX_SHARED_CEDITEVENTDIALOG_H

#include <vector>

#include "../wxInclude.h"

#include "shared/Const.h"

#include <glm/vec3.hpp>

#include "shared/studiomodel/studio.h"

namespace ui
{
/**
*	A dialog that can be used to edit events.
*/
class CEditEventsDialog final : public wxDialog
{
public:
	/**
	*	@copydoc wxDialog::wxDialog
	*	@param pEvents Pointer to the events that will be edited.
	*	@param uiNumEvents Number of events.
	*/
	CEditEventsDialog( wxWindow *parent, wxWindowID id,
					  const wxString& title,
					  mstudioevent_t* const pEvents, const size_t uiNumEvents,
					  const wxPoint& pos = wxDefaultPosition,
					  const wxSize& size = wxDefaultSize,
					  long style = wxDEFAULT_DIALOG_STYLE,
					  const wxString& name = wxDialogNameStr );
	~CEditEventsDialog();

	/**
	*	@return Whether any changes have been saved so far.
	*/
	bool ChangesSaved() const { return m_bChangedSaved; }

	/**
	*	Sets the event that is currently being edited.
	*/
	void SetEvent( const int iIndex );

protected:
	wxDECLARE_EVENT_TABLE();

private:
	void InitFromEvents( const mstudioevent_t* const pEvents, const size_t uiNumEvents );

	void CopyToEvents( mstudioevent_t* const pEvents, const size_t uiNumEvents );

	void InitFromEvent( const mstudioevent_t& event );

	void CopyToEvent( mstudioevent_t& event );

	void OnEventChanged( wxCommandEvent& event );

	void OnButton( wxCommandEvent& event );

private:
	wxListBox* m_pEvents;

	wxSpinCtrl* m_pFrame;
	wxSpinCtrl* m_pEvent;
	wxTextCtrl* m_pOptions;
	wxSpinCtrl* m_pType;

	mstudioevent_t* const m_pDestEvents;

	std::vector<mstudioevent_t> m_Events;

	/*
	*	first: Is index valid?
	*	second: Current event index
	*/
	std::pair<bool, size_t> m_CurrentEvent = { false, 0 };

	bool m_bChangedSaved = false;

private:
	CEditEventsDialog( const CEditEventsDialog& ) = delete;
	CEditEventsDialog& operator=( const CEditEventsDialog& ) = delete;
};
}

#endif //UI_WX_SHARED_CEDITEVENTDIALOG_H