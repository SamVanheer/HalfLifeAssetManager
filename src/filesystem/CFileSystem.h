#ifndef FILESYSTEM_CFILESYSTEM_H
#define FILESYSTEM_CFILESYSTEM_H

#include <vector>

#include "common/Platform.h"

namespace filesystem
{
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
	const char* GetBasePath() const;
	void SetBasePath( const char* const pszPath );

	bool HasSearchPath( const char* const pszPath ) const;
	void AddSearchPath( const char* const pszPath );
	void RemoveSearchPath( const char* const pszPath );
	void RemoveAllSearchPaths();

	bool GetRelativePath( const char* const pszFilename, char* pszOutPath, const size_t uiBufferSize );

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