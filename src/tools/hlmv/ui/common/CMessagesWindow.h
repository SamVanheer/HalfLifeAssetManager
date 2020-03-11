#ifndef UI_CMESSAGESWINDOW_H
#define UI_CMESSAGESWINDOW_H

#include "ui/wxHLMV.h"

#include <wx/listctrl.h>

#include "shared/Logging.h"

class wxListView;

namespace ui
{
/**
*	A window that lists a number of log messages.
*/
class CMessagesWindow final : public wxFrame, public ILogListener
{
public:
	CMessagesWindow(const size_t uiMaxMessagesCount);
	~CMessagesWindow();

	void LogMessage( const LogType type, const char* const pszMessage ) override final
	{
		AddMessage( type, pszMessage );
	}

	size_t GetMaxMessagesCount() const { return m_uiMaxMessagesCount; }

	void SetMaxMessagesCount( const size_t uiMaxMessagesCount );

	void AddMessage( const LogType type, const wxString& szMessage );

	void TruncateToCount( size_t uiCount );

	void Truncate();

	void Clear();

protected:
	wxDECLARE_EVENT_TABLE();

private:
	void OnSize( wxSizeEvent& event );

	void OnClear( wxCommandEvent& event );

	void OnListColumnBeginDrag( wxListEvent& event );

	void CommandEntered( wxCommandEvent& event );

	void OnShown( wxShowEvent& event );

	void OnClose( wxCloseEvent& event );

	void UpdateHeader();

private:
	size_t m_uiMaxMessagesCount;

	wxListView* m_pList;

	wxTextCtrl* m_pCommand;

private:
	CMessagesWindow( const CMessagesWindow& ) = delete;
	CMessagesWindow& operator=( const CMessagesWindow& ) = delete;
};
}

#endif //UI_CMESSAGESWINDOW_H