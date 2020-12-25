#pragma once

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
	virtual ~IFileSystem() = 0;

	/**
	*	Initializes the filesystem. This should be called once on startup.
	*/
	virtual bool Initialize() = 0;

	/**
	*	Shuts down the filesystem. This should be called on shutdown.
	*/
	virtual void Shutdown() = 0;

	/**
	*	Gets the base path.
	*/
	virtual const char* GetBasePath() const = 0;

	/**
	*	Sets the base path.
	*	@param pszPath Base path.
	*/
	virtual void SetBasePath( const char* const pszPath ) = 0;

	/**
	*	Returns whether the filesystem has the given search path.
	*	@param pszPath Path to check.
	*	@return true if the path is in the list, false otherwise.
	*/
	virtual bool HasSearchPath( const char* const pszPath ) const = 0;

	/**
	*	Adds a search path. No duplicates.
	*	@param pszPath Path to add.
	*/
	virtual void AddSearchPath( const char* const pszPath ) = 0;

	/**
	*	Removes a search path.
	*	@param pszPath Path to remove.
	*/
	virtual void RemoveSearchPath( const char* const pszPath ) = 0;

	/**
	*	Removes all search paths.
	*/
	virtual void RemoveAllSearchPaths() = 0;

	/**
	*	Gets a relative path to a file. This may actually be an absolute path, depending on the value of the base path. The file must exist.
	*	@param pszFilename File to get a path to.
	*	@param pszOutPath Destination buffer for the path.
	*	@param uiBufferSize Size of the destination buffer, in characters.
	*	@return true if a path could be formed, false otherwise.
	*/
	virtual bool GetRelativePath( const char* const pszFilename, char* pszOutPath, const size_t uiBufferSize ) = 0;

	/**
	*	Returns whether the given file exists.
	*	@param pszFilename Name of the file to check for.
	*	@return true if the file exists, false otherwise.
	*/
	virtual bool FileExists( const char* const pszFilename ) const = 0;
};

inline IFileSystem::~IFileSystem()
{
}
}

/** @} */
