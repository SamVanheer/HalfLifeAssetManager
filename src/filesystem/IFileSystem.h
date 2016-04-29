#ifndef FILESYSTEM_IFILESYSTEM_H
#define FILESYSTEM_IFILESYSTEM_H

#include "lib/LibInterface.h"

/** @file */

/**
*	@defgroup FileSystem SteamPipe filesystem.
*	
*	@{
*/

namespace filesystem
{
/**
*	Represents the SteamPipe filesystem. This can find game resources.
*	The filesystem has a concept of a base path: this is the path to the game directory, like "common/Half-Life"
*	All search paths are relative to this base path.
*/
class IFileSystem : public IBaseInterface
{
public:
	virtual ~IFileSystem() = 0;

	/**
	*	Gets the list of SteamPipe directory extensions.
	*	@param ppszDirectoryExts Pointer to pointer that points to the first extension.
	*	@return Number of extensions.
	*/
	virtual size_t GetSteamPipeDirectoryExtensions( const char* const*& ppszDirectoryExts ) = 0;

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
	*/
	virtual void SetBasePath( const char* const pszPath ) = 0;

	/**
	*	Returns whether the filesystem has the given search path.
	*/
	virtual bool HasSearchPath( const char* const pszPath ) const = 0;

	/**
	*	Adds a search path. No duplicates.
	*/
	virtual void AddSearchPath( const char* const pszPath ) = 0;

	/**
	*	Removes a search path.
	*/
	virtual void RemoveSearchPath( const char* const pszPath ) = 0;

	/**
	*	Removes all search paths.
	*/
	virtual void RemoveAllSearchPaths() = 0;

	/**
	*	Gets a relative path to a file. This may actually be an absolute path, depending on the value of the base path.
	*/
	virtual bool GetRelativePath( const char* const pszFilename, char* pszOutPath, const size_t uiBufferSize ) = 0;

	/**
	*	Returns whether the given file exists.
	*/
	virtual bool FileExists( const char* const pszFilename ) const = 0;
};

inline IFileSystem::~IFileSystem()
{
}
}

/**
*	Filesystem interface name.
*/
#define IFILESYSTEM_NAME "IFileSystemV001"

/** @} */

#endif //FILESYSTEM_IFILESYSTEM_H