#pragma once

#include <string>
#include <string_view>

#include "utility/IOUtils.hpp"

/** @file */

/**
*	@defgroup FileSystem SteamPipe filesystem
*
*	@{
*/

/**
*	@brief Represents the SteamPipe filesystem. This can find game resources.
*/
class IFileSystem
{
public:
	virtual ~IFileSystem() {}

	/**
	*	@brief Returns whether the filesystem has the given search path.
	*	@param path Path to check.
	*	@return true if the path is in the list, false otherwise.
	*/
	virtual bool HasSearchPath(std::string_view path) const = 0;

	/**
	*	@brief Adds a search path. No duplicates.
	*	@param path Path to add.
	*/
	virtual void AddSearchPath(std::string&& path) = 0;

	/**
	*	@brief Removes a search path.
	*	@param path Path to remove.
	*/
	virtual void RemoveSearchPath(std::string_view path) = 0;

	/**
	*	@brief Removes all search paths.
	*/
	virtual void RemoveAllSearchPaths() = 0;

	/**
	*	@brief Gets an absolute path to a file.
	*	The file must exist.
	*	@param fileName File to get a path to.
	*	@return The path to the file if a path could be formed, an empty string otherwise.
	*/
	virtual std::string GetAbsolutePath(std::string_view fileName) = 0;

	/**
	*	@brief Returns whether the given file exists.
	*	@param fileName Name of the file to check for.
	*	@return true if the file exists, false otherwise.
	*/
	virtual bool FileExists(const std::string& fileName) const = 0;

	/**
	*	@brief Tries to open the file with the given filename.
	*	@param fileName Absolute name of the file to open.
	*	@param binary Whether to open as binary or text.
	*	@param exclusive Whether to open the file using exclusive mode (no other programs have an open write handle to it).
	*/
	virtual FilePtr TryOpenAbsolute(const std::string& fileName, bool binary, bool exclusive = false) const = 0;

	/**
	*	@brief Tries to open the file with the given filename.
	*	@param fileName Relative name of the file to open.
	*	@param binary Whether to open as binary or text.
	*	@param exclusive Whether to open the file using exclusive mode (no other programs have an open write handle to it).
	*/
	virtual FilePtr TryOpen(std::string_view fileName, bool binary, bool exclusive = false) const = 0;
};

/** @} */
