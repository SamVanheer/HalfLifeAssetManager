#ifndef FILESYSTEM_CFILESYSTEM_H
#define FILESYSTEM_CFILESYSTEM_H

#include <vector>

#include "shared/Platform.h"

#include "IFileSystem.h"

/**
*	@ingroup FileSystem
*
*	@{
*/

namespace filesystem
{
class CFileSystem final : public IFileSystem
{
private:
	struct SearchPath_t
	{
		char szPath[ MAX_PATH_LENGTH ];
	};

	typedef std::vector<SearchPath_t> SearchPaths_t;

public:
	CFileSystem();
	~CFileSystem();

	bool Initialize() override final;
	void Shutdown() override final;

public:
	//Filesystem API

	size_t GetSteamPipeDirectoryExtensions( const char* const*& ppszDirectoryExts ) override final;

	const char* GetBasePath() const override final;

	void SetBasePath( const char* const pszPath ) override final;

	bool HasSearchPath( const char* const pszPath ) const override final;

	void AddSearchPath( const char* const pszPath ) override final;

	void RemoveSearchPath( const char* const pszPath ) override final;

	void RemoveAllSearchPaths() override final;

	bool GetRelativePath( const char* const pszFilename, char* pszOutPath, const size_t uiBufferSize ) override final;

	bool FileExists( const char* const pszFilename ) const override final;

private:
	char m_szBasePath[ MAX_PATH_LENGTH ];

	SearchPaths_t m_SearchPaths;

private:
	CFileSystem( const CFileSystem& ) = delete;
	CFileSystem& operator=( const CFileSystem& ) = delete;
};
}

/** @} */

#endif //FILESYSTEM_CFILESYSTEM_H