#include <algorithm>

#include "CRecentFiles.h"

namespace settings
{
CRecentFiles::CRecentFiles( const size_t uiMaxFiles )
	: m_uiMaxFiles( uiMaxFiles )
{
}

void CRecentFiles::SetMaxFiles( const size_t uiMaxFiles )
{
	if( m_uiMaxFiles > uiMaxFiles )
	{
		TruncateToCount( uiMaxFiles );
	}

	m_uiMaxFiles = uiMaxFiles;
}

std::string CRecentFiles::Get( size_t uiIndex ) const
{
	if( uiIndex >= m_RecentFiles.size() )
		return "";

	auto it = m_RecentFiles.begin();

	//Kinda ugly.
	while( uiIndex-- )
	{
		++it;
	}

	return *it;
}

bool CRecentFiles::Contains( const std::string& szFilename ) const
{
	return std::find( m_RecentFiles.begin(), m_RecentFiles.end(), szFilename ) != m_RecentFiles.end();
}

bool CRecentFiles::Add( const std::string& szFilename )
{
	//if it's already in the list, remove it.
	Remove( szFilename );

	m_RecentFiles.push_front( szFilename );

	Truncate();

	return true;
}

void CRecentFiles::Remove( const std::string& szFilename )
{
	auto it = std::find( m_RecentFiles.begin(), m_RecentFiles.end(), szFilename );

	if( it != m_RecentFiles.end() )
		m_RecentFiles.erase( it );
}

void CRecentFiles::RemoveAll()
{
	m_RecentFiles.clear();
}

void CRecentFiles::TruncateToCount( const size_t uiCount )
{
	if( m_RecentFiles.size() <= uiCount )
		return;

	size_t uiRemove = m_RecentFiles.size() - uiCount;

	while( uiRemove-- )
	{
		m_RecentFiles.pop_back();
	}
}

void CRecentFiles::Truncate()
{
	if( m_uiMaxFiles == NO_MAXIMUM )
		return;

	TruncateToCount( m_uiMaxFiles );
}
}