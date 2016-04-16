#ifndef UI_CMESSAGESWINDOW_H
#define UI_CMESSAGESWINDOW_H

#include "ui/wxInclude.h"

#include <wx/listctrl.h>

#include "common/Logging.h"

class CMessagesWindow;
class wxListView;
class IWindowCloseListener;

/**
*	A window that lists a number of log messages.
*	TODO move to ui namespace
*/
class CMessagesWindow final : public wxFrame, public ILogListener
{
public:
	CMessagesWindow( const size_t uiMaxMessagesCount, IWindowCloseListener* pWindowCloseListener = nullptr );
	~CMessagesWindow();

	void LogMessage( const LogType type, const char* const pszMessage ) override final
	{
		AddMessage( type, pszMessage );
	}

	size_t GetMaxMessagesCount() const { return m_uiMaxMessagesCount; }

	void SetMaxMessagesCount( const size_t uiMaxMessagesCount );

	IWindowCloseListener* GetWindowCloseListener() { return m_pWindowCloseListener; }

	void SetWindowCloseListener( IWindowCloseListener* pListener )
	{
		m_pWindowCloseListener = pListener;
	}

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

	void OnClose( wxCloseEvent& event );

	void UpdateHeader();

private:
	size_t m_uiMaxMessagesCount;

	IWindowCloseListener* m_pWindowCloseListener;

	wxListView* m_pList;

private:
	CMessagesWindow( const CMessagesWindow& ) = delete;
	CMessagesWindow& operator=( const CMessagesWindow& ) = delete;
};

#endif //UI_CMESSAGESWINDOW_H