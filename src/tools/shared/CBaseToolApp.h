#ifndef TOOLS_SHARED_CBASETOOLAPP_H
#define TOOLS_SHARED_CBASETOOLAPP_H

#include <string>

#include "app/CAppSystem.h"

namespace filesystem
{
class IFileSystem;
}

namespace soundsystem
{
class ISoundSystem;
}

namespace tools
{
/**
*	Base class for tools.
*/
class CBaseToolApp : public app::CAppSystem
{
public:
	/**
	*	@return The log filename.
	*/
	const std::string& GetLogFilename() const { return m_szLogFilename; }

	/**
	*	Sets the log filename.
	*/
	void SetLogFilename( std::string&& szFilename )
	{
		m_szLogFilename = std::move( szFilename );
	}

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
	bool StartupApp() override;

	bool LoadAppLibraries() override;

	bool Connect( const CreateInterfaceFn* pFactories, const size_t uiNumFactories ) override;

	void ShutdownApp() override;

	/**
	*	Initialize OpenGL. This is GUI specific at the moment.
	*	@return true on success, false otherwise.
	*/
	virtual bool InitOpenGL() = 0;

	/**
	*	Shuts down OpenGL. This is GUI specific at the moment.
	*/
	virtual void ShutdownOpenGL() = 0;

private:
	std::string m_szLogFilename;

	filesystem::IFileSystem* m_pFileSystem = nullptr;
	soundsystem::ISoundSystem* m_pSoundSystem = nullptr;
};
}

#endif //TOOLS_SHARED_CBASETOOLAPP_H