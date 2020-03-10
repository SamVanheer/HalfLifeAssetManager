#include <cassert>

#include "settings/CRecentFiles.h"

#include "CwxRecentFiles.h"

namespace ui
{
CwxRecentFiles::CwxRecentFiles( const std::shared_ptr<settings::CRecentFiles>& recentFiles )
	: m_RecentFiles( recentFiles )
{
	assert( recentFiles );
}

CwxRecentFiles::~CwxRecentFiles()
{
	ClearItems();
}

wxWindowID CwxRecentFiles::GetBaseID() const
{
	if( m_MenuItems.empty() )
	{
		return wxID_NONE;
	}

	return m_MenuItems.front()->GetId();
}

wxWindowID CwxRecentFiles::GetLastID() const
{
	if( m_MenuItems.empty() )
	{
		return wxID_NONE;
	}

	return m_MenuItems.back()->GetId();
}

void CwxRecentFiles::AddMenuItems( wxMenu* const pMenu )
{
	assert( pMenu );

	//Called multiple times, so clear the list.
	ClearItems();

	m_MenuItems.reserve( m_RecentFiles->GetMaxFiles() );

	const wxWindowID baseID = wxWindow::NewControlId( static_cast<int>( m_RecentFiles->GetMaxFiles() ) );

	for( size_t uiIndex = 0; uiIndex < m_RecentFiles->GetMaxFiles(); ++uiIndex )
	{
		m_MenuItems.push_back( pMenu->Append( baseID + static_cast<wxWindowID>( uiIndex ), "(empty)" ) );
	}

	Refresh();
}

void CwxRecentFiles::Refresh()
{
	const auto& recentFiles = m_RecentFiles->GetFiles();

	auto it = recentFiles.begin();
	auto end = recentFiles.end();

	//If the recent files list increased in size, don't write past the last existing menu item.
	for( size_t uiIndex = 0; uiIndex < m_MenuItems.size(); ++uiIndex )
	{
		wxMenuItem* const pItem = m_MenuItems[ uiIndex ];

		if( it != end )
		{
			pItem->Enable( true );
			pItem->SetItemLabel( it->c_str() );

			++it;
		}
		else
		{
			pItem->Enable( false );
			pItem->SetItemLabel( "(empty)" );
		}
	}
}

bool CwxRecentFiles::OnOpenRecentFile( wxCommandEvent& event, wxString& szFilename ) const
{
	szFilename = "";

	//No items? Not initialized yet.
	if( m_MenuItems.empty() )
	{
		return false;
	}

	const size_t uiIndex = GetFileIndex( event.GetId() );

	if( uiIndex == -1 )
		return false;

	szFilename = m_RecentFiles->Get( uiIndex );

	return true;
}

void CwxRecentFiles::ClearItems()
{
	if( !m_MenuItems.empty() )
	{
		//We don't have to unreserve the item IDs here since the IDs are owned by the items themselves.
		m_MenuItems.clear();
	}
}

size_t CwxRecentFiles::GetFileIndex( const wxWindowID ID ) const
{
	if( m_MenuItems.empty() )
		return -1;

	if( ID < m_MenuItems.front()->GetId() || ID > m_MenuItems.back()->GetId() )
		return -1;

	return static_cast<size_t>( ID - m_MenuItems.front()->GetId() );
}

wxMenuItem* CwxRecentFiles::FindItemById( const wxWindowID ID ) const
{
	const size_t uiIndex = GetFileIndex( ID );

	if( uiIndex == -1 )
		return nullptr;

	return m_MenuItems[ uiIndex ];
}
}