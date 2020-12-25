#pragma once

#include <vector>

#include "shared/Platform.hpp"

#include "IFileSystem.hpp"

/**
*	@ingroup FileSystem
*
*	@{
*/

namespace filesystem
{
class FileSystem final : public IFileSystem
{
private:
	struct SearchPath_t
	{
		char szPath[ MAX_PATH_LENGTH ];
	};

	typedef std::vector<SearchPath_t> SearchPaths_t;

public:
	FileSystem();
	~FileSystem();

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
	bool CheckFileExists( const char* const pszCompletePath, const size_t uiLength, char* pszOutPath, size_t uiBufferSize ) const;

private:
	char m_szBasePath[ MAX_PATH_LENGTH ];

	SearchPaths_t m_SearchPaths;

private:
	FileSystem( const FileSystem& ) = delete;
	FileSystem& operator=( const FileSystem& ) = delete;
};
}

/** @} */
