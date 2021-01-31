#pragma once

#include <string>
#include <string_view>

/** @file */

/**
*	@defgroup FileSystem SteamPipe filesystem
*
*	@{
*/

namespace filesystem
{
/**
*	@brief Represents the SteamPipe filesystem. This can find game resources.
*
*	<pre>
*	The filesystem has a concept of a base path: this is the path to the game directory, like "common/Half-Life"
*	All search paths are relative to this base path.
*	</pre>
*/
class IFileSystem
{
public:
	virtual ~IFileSystem() {}

	/**
	*	@brief Gets the base path.
	*/
	virtual std::string GetBasePath() const = 0;

	/**
	*	@brief Sets the base path.
	*	@param path Base path.
	*/
	virtual void SetBasePath(std::string&& path) = 0;

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
	*	@brief Gets a relative path to a file. This may actually be an absolute path, depending on the value of the base path. The file must exist.
	*	@param fileName File to get a path to.
	*	@return The path to the file if a path could be formed, an empty string otherwise.
	*/
	virtual std::string GetRelativePath(std::string_view fileName) = 0;

	/**
	*	@brief Returns whether the given file exists.
	*	@param fileName Name of the file to check for.
	*	@return true if the file exists, false otherwise.
	*/
	virtual bool FileExists(const std::string& fileName) const = 0;
};
}

/** @} */
