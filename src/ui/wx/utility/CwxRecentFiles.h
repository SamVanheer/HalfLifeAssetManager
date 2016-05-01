#ifndef UI_WX_UTILITY_CWXRECENTFILES_H
#define UI_WX_UTILITY_CWXRECENTFILES_H

#include <memory>
#include <vector>

#include "ui/wx/wxInclude.h"

namespace settings
{
class CRecentFiles;
}

namespace ui
{
/**
*	Manages a set of recent files menu items.
*	To define an event handler for these items, bind a function using:
*	@code
*	Bind( wxEVT_MENU, functor, handler, m_RecentFiles.GetBaseID(), m_RecentFiles.GetLastID() )
*	@endcode
*/
class CwxRecentFiles final
{
private:
	typedef std::vector<wxMenuItem*> MenuItems_t;

public:
	/**
	*	Constructor.
	*	@param recentFiles Recent files list to use. Must be valid.
	*/
	CwxRecentFiles( const std::shared_ptr<settings::CRecentFiles>& recentFiles );
	~CwxRecentFiles();

	const std::shared_ptr<const settings::CRecentFiles>& GetRecentFiles() const { return m_RecentFiles; }
	const std::shared_ptr<settings::CRecentFiles>& GetRecentFiles() { return m_RecentFiles; }

	/**
	*	Returns the base ID used for the list of items. This is the ID of the first item, each item having an ID that follows it.
	*	Returns wxID_NONE if no items exist.
	*/
	wxWindowID GetBaseID() const;

	/**
	*	Returns the last ID used by the list of items.
	*/
	wxWindowID GetLastID() const;

	/**
	*	Adds menu items to the given item. If this is called after items were already added, they are removed and added again.
	*	The list is automatically refreshed after this.
	*	@param pMenu Menu to add items to.
	*/
	void AddMenuItems( wxMenu* const pMenu );

	/**
	*	Refreshes the files list.
	*/
	void Refresh();

	/**
	*	Given an EVT_MENU event, returns the name of the file to open, or an empty string if the event was not meant for this list, or if no recent file exist at that position.
	*	@param event EVT_MENU event.
	*	@param szFilename Filename.
	*	@return true if the event was valid, false otherwise.
	*/
	bool OnOpenRecentFile( wxCommandEvent& event, wxString& szFilename ) const;

private:
	void ClearItems();

	size_t GetFileIndex( const wxWindowID ID ) const;

	wxMenuItem* FindItemById( const wxWindowID ID ) const;

private:
	std::shared_ptr<settings::CRecentFiles> m_RecentFiles;

	MenuItems_t m_MenuItems;

private:
	CwxRecentFiles( const CwxRecentFiles& ) = delete;
	CwxRecentFiles& operator=( const CwxRecentFiles& ) = delete;
};
}

#endif //UI_WX_UTILITY_CWXRECENTFILES_H