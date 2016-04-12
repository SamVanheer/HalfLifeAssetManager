#include "filesystem/CFileSystem.h"
#include "soundsystem/CSoundSystem.h"

#include "CwxOpenGL.h"

#include "CwxBaseApp.h"

CwxBaseApp::CwxBaseApp()
{
}

CwxBaseApp::~CwxBaseApp()
{
}

void CwxBaseApp::OnTimer( CTimer& timer )
{
	//TODO: move this
	if( soundsystem::CSoundSystem::InstanceExists() )
		soundSystem().RunFrame();
}

bool CwxBaseApp::InitApp( InitFlags_t initFlags, const wxString& szDisplayName )
{
	//The sound system requires the use of the file system.
	if( initFlags & INIT_SOUNDSYSTEM )
		initFlags |= INIT_FILESYSTEM;

	if( initFlags & INIT_FILESYSTEM )
	{
		filesystem::CFileSystem::CreateInstance();

		if( !fileSystem().Initialize() )
		{
			wxMessageBox( "Failed to initialize file system", wxMessageBoxCaptionStr, wxOK | wxCENTRE | wxICON_ERROR );
			return false;
		}
	}

	if( !szDisplayName.IsEmpty() )
		SetAppDisplayName( szDisplayName );

	//TODO: make customizable based on app
	if( initFlags & INIT_OPENGL )
	{
		//Set up OpenGL parameters.
		CwxOpenGL::CreateInstance();

		wxGLAttributes canvasAttributes;

		canvasAttributes
			.PlatformDefaults()
			.Stencil( 8 )
			.EndList();

		wxGLContextAttrs contextAttributes;

		contextAttributes.PlatformDefaults()
			.MajorVersion( 3 )
			.MinorVersion( 0 )
			.EndList();

		if( !wxOpenGL().Initialize( canvasAttributes, &contextAttributes ) )
			return false;
	}

	if( initFlags & INIT_IMAGEHANDLERS )
	{
		wxInitAllImageHandlers();
	}

	if( initFlags & INIT_SOUNDSYSTEM )
	{
		soundsystem::CSoundSystem::CreateInstance();

		if( !soundSystem().Initialize() )
		{
			wxMessageBox( "Failed to initialize sound system", wxMessageBoxCaptionStr, wxOK | wxCENTRE | wxICON_ERROR );
			return false;
		}
	}

	m_pTimer = new CTimer( this );

	return true;
}

void CwxBaseApp::ShutdownApp()
{
	if( m_pTimer )
	{
		delete m_pTimer;
		m_pTimer = nullptr;
	}

	if( soundsystem::CSoundSystem::InstanceExists() )
	{
		soundSystem().Shutdown();
		soundsystem::CSoundSystem::DestroyInstance();
	}

	if( filesystem::CFileSystem::InstanceExists() )
	{
		fileSystem().Shutdown();
		filesystem::CFileSystem::DestroyInstance();
	}

	if( CwxOpenGL::InstanceExists() )
	{
		wxOpenGL().Shutdown();

		CwxOpenGL::DestroyInstance();
	}
}

void CwxBaseApp::StartTimer()
{
	//60 FPS
	m_pTimer->Start( ( 1 / 60.0 ) * 1000 );
}

void CwxBaseApp::ExitApp( const bool bMainWndClosed )
{
	m_pTimer->SetListener( nullptr );
}