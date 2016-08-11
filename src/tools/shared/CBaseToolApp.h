#ifndef TOOLS_SHARED_CBASETOOLAPP_H
#define TOOLS_SHARED_CBASETOOLAPP_H

#include "app/CAppSystem.h"

namespace filesystem
{
class IFileSystem;
}

namespace soundsystem
{
class ISoundSystem;
}

class ILibSystem;

namespace tools
{
/**
*	Base class for tools.
*/
class CBaseToolApp : public app::CAppSystem
{
public:
	/**
	*	Gets the filesystem.
	*	@return Filesystem instance.
	*/
	filesystem::IFileSystem* GetFileSystem() { return m_pFileSystem; }

	/**
	*	Gets the soundsystem.
	*	@return Soundsystem instance.
	*/
	soundsystem::ISoundSystem* GetSoundSystem() { return m_pSoundSystem; }

protected:
	bool LoadAppLibraries() override;

	bool Connect( const CreateInterfaceFn* pFactories, const size_t uiNumFactories ) override;

	bool RunApp( int iArgc, wchar_t* pszArgV[] ) = 0;

	void ShutdownApp() override;

private:
	filesystem::IFileSystem* m_pFileSystem = nullptr;
	soundsystem::ISoundSystem* m_pSoundSystem = nullptr;

	ILibSystem* m_pRendererLib = nullptr;
};
}

#endif //TOOLS_SHARED_CBASETOOLAPP_H