#ifndef SETTINGS_CRECENTFILES_H
#define SETTINGS_CRECENTFILES_H

#include <list>
#include <string>

namespace settings
{
/**
*	Stores a list of recent files.
*	TODO: use absolute paths internally.
*/
class CRecentFiles final
{
public:
	typedef std::list<std::string> RecentFiles_t;

	static const size_t NO_MAXIMUM = 0;

public:
	/**
	*	Constructor. Creates an empty list with the given maximum number of files.
	*	@param uiMaxFiles Maximum number of files that can be added. If set to NO_MAXIMUM, does not apply a limit.
	*/
	CRecentFiles( const size_t uiMaxFiles = NO_MAXIMUM );

	/**
	*	Constructor. Creates a list using files taken from the given range to fill the list.
	*	@param begin Iterator pointing to the beginning of a range to use to fill the list.
	*	@param end Iterator pointing to the end of a range to use to fill the list.
	*	@param uiMaxFiles Maximum number of files that can be added. If set to NO_MAXIMUM, does not apply a limit.
	*/
	template<typename INPUTITERATOR>
	CRecentFiles( INPUTITERATOR begin, INPUTITERATOR end, const size_t uiMaxFiles = NO_MAXIMUM );

	/**
	*	Constructor. Creates a list using files taken from the given container to fill the list.
	*	@param container Container to use to fill the list.
	*	@param uiMaxFiles Maximum number of files that can be added. If set to NO_MAXIMUM, does not apply a limit.
	*/
	template<typename CONTAINER>
	CRecentFiles( const CONTAINER& container, const size_t uiMaxFiles = NO_MAXIMUM );

	/**
	*	Copy constructor.
	*/
	CRecentFiles( const CRecentFiles& ) = default;

	/**
	*	Assignment operator.
	*/
	CRecentFiles& operator=( const CRecentFiles& ) = default;

	/**
	*	@return True if the list has a file limit, false otherwise.
	*/
	bool HasFileLimit() const { return m_uiMaxFiles != NO_MAXIMUM; }

	/**
	*	Gets the maximum number of files that can be tracked by this list. If this returns NO_MAXIMUM, there is no maximum.
	*/
	size_t GetMaxFiles() const { return m_uiMaxFiles; }

	/**
	*	Sets the maximum number of files that can be tracked by this list. If set to NO_MAXIMUM, there is no maximum.
	*	@param uiMaxFiles Maximum number of files to track.
	*/
	void SetMaxFiles( const size_t uiMaxFiles );

	/**
	*	Gets the list of files. The returned list may not be modified.
	*/
	const RecentFiles_t& GetFiles() const { return m_RecentFiles; }

	/**
	*	Gets the list of files. The returned list is a copy, and may be modified.
	*/
	RecentFiles_t GetFiles() { return m_RecentFiles; }

	/**
	*	@return The number of files in the list.
	*/
	size_t GetFileCount() const { return m_RecentFiles.size(); }

	/**
	*	Gets the file at the given index, or an empty string, if the index is invalid.
	*	@param uiIndex Index of the file to get.
	*	@return Filename, or an empty string.
	*/
	std::string Get( size_t uiIndex ) const;

	/**
	*	Returns whether the given file is in the list.
	*	@param szFilename Filename to search.
	*	@return true if the file is in the list, false otherwise.
	*/
	bool Contains( const std::string& szFilename ) const;

	/**
	*	Adds a file to the list, removing the oldest if the maximum number of files was reached.
	*	@param szFilename File to add to the list.
	*/
	bool Add( const std::string& szFilename );

	/**
	*	Removes a file from the list.
	*	szFilename File to remove from the list.
	*/
	void Remove( const std::string& szFilename );

	/**
	*	Removes all files from the list.
	*/
	void RemoveAll();

	/**
	*	Truncates the list to uiCount files. Note: if you wish to remove all files, use RemoveAll instead.
	*	@param uiCount Maximum number of files to keep in the list.
	*/
	void TruncateToCount( const size_t uiCount );

	/**
	*	Truncates the list to the maximum number of files allowed. Typically unnecessary, since the list is managed automatically.
	*/
	void Truncate();

private:
	size_t m_uiMaxFiles;
	RecentFiles_t m_RecentFiles;
};

template<typename INPUTITERATOR>
CRecentFiles::CRecentFiles( INPUTITERATOR begin, INPUTITERATOR end, const size_t uiMaxFiles )
	: m_uiMaxFiles( uiMaxFiles )
{
	if( HasFileLimit() )
	{
		size_t uiCount = 0;

		for( auto it = begin; it != end && uiCount < uiMaxFiles; ++it, ++uiCount )
		{
			m_RecentFiles.push_back( *it );
		}
	}
	else
	{
		m_RecentFiles = std::move( RecentFiles_t( begin, end ) );
	}
}

template<typename CONTAINER>
CRecentFiles::CRecentFiles( const CONTAINER& container, const size_t uiMaxFiles )
	: CRecentFiles( std::begin( container ), std::end( container ), uiMaxFiles )
{
}
}

#endif //SETTINGS_CRECENTFILES_H