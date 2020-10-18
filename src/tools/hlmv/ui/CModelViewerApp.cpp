#include <filesystem>

#include <wx/cmdline.h>
#include <wx/private/timer.h>

#include "shared/Logging.h"
#include "utility/PlatUtils.h"

#include "core/shared/CWorldTime.h"

#include "cvar/CVar.h"
#include "cvar/CCVarSystem.h"

#include "filesystem/CFileSystem.h"
#include "filesystem/IFileSystem.h"

#include "game/entity/CEntityManager.h"
#include "game/entity/CBaseEntityList.h"

#include "soundsystem/CSoundSystem.h"
#include "soundsystem/DummySoundSystem.h"
#include "soundsystem/ISoundSystem.h"

#include "engine/renderer/gl/imode/CRenderContextIMode.h"
#include "engine/renderer/studiomodel/CStudioModelRenderer.h"
#include "engine/shared/renderer/IRenderContext.h"
#include "engine/shared/renderer/studiomodel/IStudioModelRenderer.h"

#include "CFullscreenWindow.h"
#include "CMainWindow.h"

#include "ui/common/CMessagesWindow.h"
#include "wx/CwxOpenGL.h"
#include "wx/wxLogging.h"

#include "CModelViewerApp.h"

//TODO: this is temporary until the renderers can be moved into contexts - Solokiller
namespace sprite
{
class ISpriteRenderer;
}

//TODO: remove
soundsystem::ISoundSystem* g_pSoundSystem = nullptr;

renderer::IRenderContext* g_pRenderContext = nullptr;
studiomdl::IStudioModelRenderer* g_pStudioMdlRenderer = nullptr;
sprite::ISpriteRenderer* g_pSpriteRenderer = nullptr;

wxIMPLEMENT_APP( hlmv::CModelViewerApp );

namespace hlmv
{
namespace
{
//Use the default list class for now.
static CBaseEntityList g_EntityList;
}

static cvar::CCVar max_fps(
	"max_fps",
	cvar::CCVarArgsBuilder()
	.HelpInfo("The maximum FPS that this program will redraw at")
	.FloatValue(60.0f)
	.Flags(cvar::Flag::ARCHIVE)
);

bool CModelViewerApp::OnInit()
{
	if (!wxApp::OnInit())
		return false;

	max_fps.SetHandler(this);

	SetAppDisplayName( HLMV_TITLE );

	//Install the wxWidgets specific default log listener.
	SetDefaultLogListener(GetwxDefaultLogListener());

	if (!Startup())
	{
		Shutdown();

		return false;
	}

	//Reduce the idle event strain on the system a bit.
	wxIdleEvent::SetMode(wxIDLE_PROCESS_SPECIFIED);

	ResetTickImplementation();

	return true;
}

int CModelViewerApp::OnExit()
{
	ClearTickImplementation();

	Shutdown();

	UseMessagesWindow(false);

	max_fps.SetHandler(nullptr);

	return wxApp::OnExit();
}

void CModelViewerApp::OnInitCmdLine( wxCmdLineParser& parser )
{
	wxApp::OnInitCmdLine( parser );

	//Note: this works by setting all available parameters in the order that they appear on the command line.
	//The model filename must be last for this to work with drag&drop.
	parser.AddParam( "Filename of the model to load on startup", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL );
}

bool CModelViewerApp::OnCmdLineParsed( wxCmdLineParser& parser )
{
	//Last parameter is the model to load.
	if( parser.GetParamCount() > 0 )
		m_szModel = parser.GetParam( parser.GetParamCount() - 1 );

	return wxApp::OnCmdLineParsed( parser );
}

void CModelViewerApp::HandleCVar(cvar::CCVar& cvar, const char* pszOldValue, float flOldValue)
{
	if (!strcmp(cvar.GetName(), "max_fps"))
	{
		if (m_pTimer)
		{
			m_pTimer->Start(static_cast<int>((1 / max_fps.GetFloat()) * 1000.));
		}
	}
}

bool CModelViewerApp::Startup()
{
	//Configure the working directory to be the exe directory.
	{
		bool bSuccess;
		auto szExePath = plat::GetExeFileName(&bSuccess);

		if (!bSuccess)
		{
			Error("CAppSystem::Startup: Failed to get executable filename!\n");
			return false;
		}

		std::filesystem::path exePath(szExePath);

		std::error_code canonicalError;

		auto exeDir = exePath.parent_path();

		exeDir = std::filesystem::canonical(exeDir, canonicalError);

		if (canonicalError)
		{
			Error(
				"CAppSystem::Startup: Failed to canonicalize  \"%s\" with error \"%s\"!\n",
				exeDir.string().c_str(), canonicalError.message().c_str());
			return false;
		}

		std::error_code cwdError;

		std::filesystem::current_path(exeDir, cwdError);

		if (cwdError)
		{
			Error(
				"CAppSystem::Startup: Failed to set current working directory \"%s\" with error \"%s\"!\n",
				exeDir.string().c_str(), cwdError.message().c_str());
			return false;
		}
	}

	const std::string szLogFilename = HLMV_TITLE + std::string{".log"};

	//Overwrite previous session log.
	logging().OpenLogFile(szLogFilename.c_str(), false);

	UTIL_InitRandom();

	m_pFileSystem = new filesystem::CFileSystem();
	g_pCVar = new cvar::CCVarSystem();
	g_pSoundSystem = m_pSoundSystem = new soundsystem::CSoundSystem();
	g_pRenderContext = new renderer::CRenderContextIMode();
	g_pStudioMdlRenderer = new studiomdl::CStudioModelRenderer();

	if (!g_pCVar->Initialize())
	{
		FatalError("Failed to initialize CVar system!\n");
		return false;
	}

	if (!m_pFileSystem->Initialize())
	{
		FatalError("Failed to initialize file system!\n");
		return false;
	}

	{
		//Set up OpenGL parameters.
		CwxOpenGL::CreateInstance();

		wxGLAttributes canvasAttributes;

		canvasAttributes
			.PlatformDefaults()
			.RGBA()
			.Depth(16)
			.Stencil(8)
			.DoubleBuffer()
			.EndList();

		wxGLContextAttrs contextAttributes;

		//The default settings for OpenGL allow for Windows XP support. OpenGL 2.1 is typically supported by XP era hardware, making it the best choice.
		//2.1 supports GLSL 120, which has some features that are nice to have in shaders.
		contextAttributes
			.PlatformDefaults()
			.MajorVersion(2)
			.MinorVersion(1)
			.EndList();

		if (!wxOpenGL().Initialize(canvasAttributes, &contextAttributes))
		{
			wxMessageBox("Failed to initialize OpenGL", wxMessageBoxCaptionStr, wxOK | wxCENTRE | wxICON_ERROR);
			return false;
		}
	}

	if (!g_pStudioMdlRenderer->Initialize())
	{
		FatalError("Failed to initialize StudioModel renderer!\n");
		return false;
	}

	if (!m_pSoundSystem->Initialize(m_pFileSystem))
	{
		//No audio device available
		//TODO: may need to check for errors specifically to distinguish between no device present and failure to start
		m_pSoundSystem->Shutdown();
		delete m_pSoundSystem;
		
		g_pSoundSystem = m_pSoundSystem = new soundsystem::DummySoundSystem();
	}

	wxInitAllImageHandlers();

	m_pState = new CHLMVState();
	m_pSettings = new CHLMVSettings(m_pFileSystem);

	//TODO: fix on Linux - Solokiller
	m_ToolIcon = wxICON(HLMV_ICON);

	SetEntityList(&g_EntityList);

	if (!EntityManager().OnMapBegin())
	{
		FatalError("Failed to start map\n");
		return false;
	}

	//Must be called before we create the main window, since it accesses the window.
	UseMessagesWindow(true);

	if (!GetSettings()->Initialize(HLMV_SETTINGS_FILE))
	{
		return false;
	}

	m_pMainWindow = new hlmv::CMainWindow(this);

	m_pMainWindow->Show(true);

	if (!m_szModel.IsEmpty())
		LoadModel(m_szModel);

	return true;
}

void CModelViewerApp::Shutdown()
{
	if(auto settings = GetSettings(); settings)
	{
		if (m_pMainWindow)
		{
			m_pMainWindow->SaveWindowSettings();
		}

		settings->Shutdown( HLMV_SETTINGS_FILE );
	}

	//If either window is still open at this time, force close them
	if( m_pFullscreenWindow )
	{
		m_pFullscreenWindow->Close(true);
		m_pFullscreenWindow = nullptr;
	}

	if( m_pMainWindow )
	{
		m_pMainWindow->Close(true);
		m_pMainWindow = nullptr;
	}

	if( EntityManager().IsMapRunning() )
	{
		EntityManager().OnMapEnd();
	}

	EntityManager().Shutdown();

	if( m_pSettings )
	{
		delete m_pSettings;
		m_pSettings = nullptr;
	}

	if( m_pState )
	{
		delete m_pState;
		m_pState = nullptr;
	}

	if (m_pSoundSystem)
	{
		m_pSoundSystem->Shutdown();
		delete m_pSoundSystem;
		m_pSoundSystem = nullptr;
	}

	if (g_pStudioMdlRenderer)
	{
		g_pStudioMdlRenderer->Shutdown();
		delete g_pStudioMdlRenderer;
		g_pStudioMdlRenderer = nullptr;
	}

	if (g_pRenderContext)
	{
		delete g_pRenderContext;
		g_pRenderContext = nullptr;
	}

	if (CwxOpenGL::InstanceExists())
	{
		wxOpenGL().Shutdown();

		CwxOpenGL::DestroyInstance();
	}

	if (m_pFileSystem)
	{
		m_pFileSystem->Shutdown();
		delete m_pFileSystem;
		m_pFileSystem = nullptr;
	}

	if (g_pCVar)
	{
		g_pCVar->Shutdown();
		delete g_pCVar;
		g_pCVar = nullptr;
	}

	//Don't close the log file just yet. It'll be closed when the program is unloaded, so anything that happens between now and then should be logged.
}

void CModelViewerApp::RunFrame()
{
	EntityManager().RunFrame();

	if( m_pFullscreenWindow )
		m_pFullscreenWindow->RunFrame();
	else if( m_pMainWindow )
		m_pMainWindow->RunFrame();
}

void CModelViewerApp::OnTick()
{
	const double flCurTime = GetCurrentTime();

	double flFrameTime = flCurTime - WorldTime.GetPreviousRealTime();

	WorldTime.SetRealTime(flCurTime);

	if (flFrameTime > 1.0)
		flFrameTime = 0.1;

	//Don't use this when using wxTimer, since it lowers the FPS by a fair amount.
	if (!m_pTimer)
	{
		if (flFrameTime < (1.0 / max_fps.GetFloat()))
		{
			return;
		}
	}

	WorldTime.TimeChanged(flCurTime);

	g_pCVar->RunFrame();

	g_pStudioMdlRenderer->RunFrame();

	m_pSoundSystem->RunFrame();

	RunFrame();
}

void CModelViewerApp::OnIdle(wxIdleEvent& event)
{
	event.RequestMore();

	OnTick();
}

void CModelViewerApp::OnTimerTick(wxTimerEvent& event)
{
	OnTick();
}

void CModelViewerApp::ResetTickImplementation()
{
	ClearTickImplementation();

	if (m_pSettings->UseTimerForFrame())
	{
		m_pTimer = new wxTimer();

		m_pTimer->Bind(wxEVT_TIMER, &CModelViewerApp::OnTimerTick, this);

		m_pTimer->Start(static_cast<int>((1 / max_fps.GetFloat()) * 1000.));
	}
	else
	{
		wxApp::Connect(wxEVT_IDLE, wxIdleEventHandler(CModelViewerApp::OnIdle));
	}
}

void CModelViewerApp::ClearTickImplementation()
{
	if (m_pTimer)
	{
		m_pTimer->Stop();
		delete m_pTimer;
		m_pTimer = nullptr;
	}

	wxApp::Disconnect(wxEVT_IDLE, wxIdleEventHandler(CModelViewerApp::OnIdle));
}

void CModelViewerApp::Exit(const bool bMainWndClosed)
{
	//Don't call multiple times.
	if (m_bExiting)
		return;

	m_bExiting = true;

	//Close messages window if needed.
	UseMessagesWindow(false);

	//Don't let any log message boxes pop up during shutdown.
	logging().SetLogListener(GetNullLogListener());

	if (bMainWndClosed)
		m_pMainWindow = nullptr;

	if (m_pFullscreenWindow)
	{
		m_pFullscreenWindow->Close(true);
		m_pFullscreenWindow = nullptr;
	}

	if (m_pMainWindow)
	{
		m_pMainWindow->Close(true);
		m_pMainWindow = nullptr;
	}

	wxOpenGL().Shutdown();
}

void CModelViewerApp::OnWindowClose(wxCloseEvent& event)
{
	if (event.GetEventObject() == m_pMessagesWindow)
	{
		if (!event.CanVeto())
		{
			MessagesWindowClosed();
		}
	}

	//Need to skip in all cases so other event handlers still run
	event.Skip();
}

void CModelViewerApp::UseMessagesWindow(const bool bUse)
{
	//Don't allow creation during exit.
	if (bUse && m_bExiting)
		return;

	//No change
	if (bUse == (m_pMessagesWindow != nullptr))
		return;

	if (bUse)
	{
		m_pMessagesWindow = new ui::CMessagesWindow(m_uiMaxMessagesCount);

		m_pMessagesWindow->Bind(wxEVT_CLOSE_WINDOW, &CModelViewerApp::OnWindowClose, this);

		m_pMessagesWindow->SetIcon(m_ToolIcon);

		logging().SetLogListener(m_pMessagesWindow);
	}
	else
	{
		m_pMessagesWindow->Close(true);

		//Don't call MessagesWindowClosed; Close calls this.
	}
}

void CModelViewerApp::ShowMessagesWindow(const bool bShow)
{
	if (!m_pMessagesWindow)
		return;

	m_pMessagesWindow->Show(bShow);
}

size_t CModelViewerApp::GetMaxMessagesCount() const
{
	if (!m_pMessagesWindow)
		return m_uiMaxMessagesCount;

	return m_pMessagesWindow->GetMaxMessagesCount();
}

void CModelViewerApp::SetMaxMessagesCount(const size_t uiMaxMessagesCount)
{
	m_uiMaxMessagesCount = uiMaxMessagesCount;

	if (m_pMessagesWindow)
	{
		m_pMessagesWindow->SetMaxMessagesCount(uiMaxMessagesCount);
	}
}

void CModelViewerApp::MessagesWindowClosed()
{
	m_pMessagesWindow = nullptr;

	logging().SetLogListener(nullptr);
}

bool CModelViewerApp::LoadModel( const char* const pszFilename )
{
	return m_pMainWindow->LoadModel( pszFilename );
}

bool CModelViewerApp::PromptLoadModel()
{
	return m_pMainWindow->PromptLoadModel();
}

bool CModelViewerApp::PromptSaveModel()
{
	return m_pMainWindow->PromptSaveModel();
}

bool CModelViewerApp::LoadBackgroundTexture( const char* const pszFilename )
{
	return m_pMainWindow->LoadBackgroundTexture( pszFilename );
}

bool CModelViewerApp::PromptLoadBackgroundTexture()
{
	return m_pMainWindow->PromptLoadBackgroundTexture();
}

void CModelViewerApp::UnloadBackgroundTexture()
{
	m_pMainWindow->UnloadBackgroundTexture();
}

bool CModelViewerApp::LoadGroundTexture( const char* const pszFilename )
{
	return m_pMainWindow->LoadGroundTexture( pszFilename );
}

bool CModelViewerApp::PromptGroundTexture()
{
	return m_pMainWindow->PromptLoadGroundTexture();
}

void CModelViewerApp::UnloadGroundTexture()
{
	m_pMainWindow->UnloadGroundTexture();
}

void CModelViewerApp::SaveUVMap( const wxString& szFilename, const int iTexture )
{
	m_pMainWindow->SaveUVMap( szFilename, iTexture );
}
}
