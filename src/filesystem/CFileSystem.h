#ifndef FILESYSTEM_CFILESYSTEM_H
#define FILESYSTEM_CFILESYSTEM_H

#include <vector>

#include "common/Platform.h"

namespace filesystem
{
/**
*	Represents the SteamPipe filesystem. This can find game resources.
*	The filesystem has a concept of a base path: this is the path to the game directory, like "common/Half-Life"
*	All search paths are relative to this base path.
*/
class CFileSystem final
{
private:
	struct SearchPath_t
	{
		char szPath[ MAX_PATH_LENGTH ];
	};

	typedef std::vector<SearchPath_t> SearchPaths_t;

public:
	static CFileSystem& CreateInstance();
	static void DestroyInstance();
	static bool InstanceExists();
	static CFileSystem& GetInstance();

	CFileSystem();
	~CFileSystem();

	bool Initialize();
	void Shutdown();

public:
	//Filesystem API

	/**
	*	Gets the base path.
	*/
	const char* GetBasePath() const;

	/**
	*	Sets the base path.
	*/
	void SetBasePath( const char* const pszPath );

	/**
	*	Returns whether the filesystem has the given search path.
	*/
	bool HasSearchPath( const char* const pszPath ) const;

	/**
	*	Adds a search path. No duplicates.
	*/
	void AddSearchPath( const char* const pszPath );

	/**
	*	Removes a search path.
	*/
	void RemoveSearchPath( const char* const pszPath );

	/**
	*	Removes all search paths.
	*/
	void RemoveAllSearchPaths();

	/**
	*	Gets a relative path to a file. This may actually be an absolute path, depending on the value of the base path.
	*/
	bool GetRelativePath( const char* const pszFilename, char* pszOutPath, const size_t uiBufferSize );

	/**
	*	Returns whether the given file exists.
	*/
	bool FileExists( const char* const pszFilename ) const;

private:
	static CFileSystem* m_pInstance;

	char m_szBasePath[ MAX_PATH_LENGTH ];

	SearchPaths_t m_SearchPaths;

private:
	CFileSystem( const CFileSystem& ) = delete;
	CFileSystem& operator=( const CFileSystem& ) = delete;
};
}

inline filesystem::CFileSystem& fileSystem()
{
	return filesystem::CFileSystem::GetInstance();
}

#endif //FILESYSTEM_CFILESYSTEM_H