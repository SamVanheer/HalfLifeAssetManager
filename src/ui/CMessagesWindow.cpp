#include <wx/sizer.h>

#include "utility/IWindowCloseListener.h"

#include "CMessagesWindow.h"

wxBEGIN_EVENT_TABLE( CMessagesWindow, wxFrame )
	EVT_SIZE( CMessagesWindow::OnSize )
	EVT_BUTTON( wxID_COMMON_MESSAGES_CLEAR, CMessagesWindow::OnClear )
	EVT_LIST_COL_BEGIN_DRAG( wxID_ANY, CMessagesWindow::OnListColumnBeginDrag )
	EVT_CLOSE( CMessagesWindow::OnClose )
wxEND_EVENT_TABLE()

CMessagesWindow::CMessagesWindow( const size_t uiMaxMessagesCount, IWindowCloseListener* pWindowCloseListener )
	: wxFrame( nullptr, wxID_ANY, "Messages Window", wxDefaultPosition, wxDefaultSize, ( wxDEFAULT_FRAME_STYLE ) )
	, m_uiMaxMessagesCount( 0 )
	, m_pWindowCloseListener( pWindowCloseListener )
{

	wxButton* pClear = new wxButton( this, wxID_COMMON_MESSAGES_CLEAR, "Clear" );

	m_pList = new wxListView( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_HRULES );

	m_pList->InsertColumn( 0, "", wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER );

	UpdateHeader();

	//Layout
	wxBoxSizer* pSizer = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* pBtnsSizer = new wxBoxSizer( wxHORIZONTAL );

	pBtnsSizer->Add( pClear );

	pSizer->Add( pBtnsSizer );

	pSizer->Add( m_pList, wxSizerFlags().Expand().Proportion( 1 ) );

	this->SetSizer( pSizer );

	SetMaxMessagesCount( uiMaxMessagesCount );
}

CMessagesWindow::~CMessagesWindow()
{
}

void CMessagesWindow::SetMaxMessagesCount( const size_t uiMaxMessagesCount )
{
	if( uiMaxMessagesCount < m_uiMaxMessagesCount )
	{
		TruncateToCount( uiMaxMessagesCount );
	}

	m_uiMaxMessagesCount = uiMaxMessagesCount;
}

void CMessagesWindow::AddMessage( const LogType type, const wxString& szMessage )
{
	wxListItem item;

	wxString szPrefix;

	item.SetId( m_pList->GetItemCount() );

	switch( type )
	{
	default:
	case LogType::MESSAGE:
		{
			item.SetTextColour( wxColor( 0, 0, 0 ) );
			break;
		}

	case LogType::WARNING:
		{
			item.SetTextColour( wxColor( 128, 0, 0 ) );
			szPrefix = "WARNING: ";
			break;
		}

	case LogType::ERROR:
		{
			item.SetTextColour( wxColor( 255, 0, 0 ) );
			szPrefix = "ERROR: ";
			break;
		}
	}

	item.SetText( szPrefix + szMessage );

	m_pList->InsertItem( item );

	Truncate();

	UpdateHeader();
}

void CMessagesWindow::TruncateToCount( size_t uiCount )
{
	const size_t uiTotal = static_cast<size_t>( m_pList->GetItemCount() );

	if( uiCount >= uiTotal )
		return;

	const size_t uiRemove = uiTotal - uiCount;

	for( size_t uiIndex = 0; uiIndex < uiRemove; ++uiIndex )
	{
		m_pList->DeleteItem( 0 );
	}

	UpdateHeader();
}

void CMessagesWindow::Truncate()
{
	TruncateToCount( m_uiMaxMessagesCount );
}

void CMessagesWindow::Clear()
{
	m_pList->DeleteAllItems();

	UpdateHeader();
}

void CMessagesWindow::OnSize( wxSizeEvent& event )
{
	//Force the column to be the same width as the window.
	m_pList->SetColumnWidth( 0, GetClientSize().GetWidth() );

	event.Skip();
}

void CMessagesWindow::OnClear( wxCommandEvent& event )
{
	Clear();
}

void CMessagesWindow::OnListColumnBeginDrag( wxListEvent& event )
{
	//Disallow user dragging of the header
	event.Veto();
}

void CMessagesWindow::OnClose( wxCloseEvent& event )
{
	//Veto any non-forced close. This window must stick around until it's closed by force.
	if( event.CanVeto() )
	{
		event.Veto();
		//Hide the window.
		Show( false );
	}

	if( m_pWindowCloseListener )
		m_pWindowCloseListener->OnWindowClose( this, event );

	if( !event.CanVeto() )
	{
		event.Skip();
	}
}

void CMessagesWindow::UpdateHeader()
{
	wxListItem column;

	column.SetText( wxString::Format( "Messages (%d)", m_pList->GetItemCount() ) );

	m_pList->SetColumn( 0, column );
}