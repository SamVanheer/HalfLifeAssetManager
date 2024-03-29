#include <algorithm>
#include <cassert>
#include <chrono>
#include <iterator>
#include <stdexcept>

#include <QFileInfo>
#include <QLayout>
#include <QMessageBox>
#include <QOpenGLFunctions_1_1>
#include <QOpenGLDebugLogger>
#include <QProcess>
#include <QSettings>

#include <spdlog/logger.h>

#include "application/AssetList.hpp"
#include "application/AssetManager.hpp"
#include "application/Assets.hpp"

#include "filesystem/FileSystem.hpp"
#include "filesystem/IFileSystem.hpp"

#include "graphics/IGraphicsContext.hpp"
#include "graphics/TextureLoader.hpp"

#include "plugins/IAssetManagerPlugin.hpp"

#include "qt/QtLogging.hpp"
#include "qt/QtLogSink.hpp"

#include "settings/ApplicationSettings.hpp"
#include "settings/ExternalProgramSettings.hpp"
#include "settings/GameConfigurationsSettings.hpp"

#include "soundsystem/DummySoundSystem.hpp"
#include "soundsystem/ISoundSystem.hpp"
#include "soundsystem/SoundSystem.hpp"

#include "ui/DragNDropEventFilter.hpp"
#include "ui/MainWindow.hpp"
#include "ui/SceneWidget.hpp"

#include "ui/options/OptionsPageColors.hpp"
#include "ui/options/OptionsPageExternalPrograms.hpp"
#include "ui/options/OptionsPageGeneral.hpp"
#include "ui/options/OptionsPageRegistry.hpp"
#include "ui/options/OptionsPageStyle.hpp"
#include "ui/options/gameconfigurations/OptionsPageFileSystem.hpp"
#include "ui/options/gameconfigurations/OptionsPageGameConfigurations.hpp"

#include "utility/WorldTime.hpp"

Q_LOGGING_CATEGORY(HLAM, "hlam")
Q_LOGGING_CATEGORY(HLAMFileSystem, "hlam.filesystem")
Q_LOGGING_CATEGORY(HLAMSoundSystem, "hlam.soundsystem")

AssetManager::AssetManager(
	QApplication* guiApplication,
	const std::shared_ptr<ApplicationSettings>& applicationSettings,
	std::unique_ptr<graphics::IGraphicsContext>&& graphicsContext,
	QObject* parent)
	: QObject(parent)
	, _guiApplication(guiApplication)

	, _logger(CreateQtLoggerSt(HLAM()))

	, _applicationSettings(applicationSettings)
	, _dragNDropEventFilter(new DragNDropEventFilter(this, this))

	, _graphicsContext(std::move(graphicsContext))
	, _openglFunctions(std::make_unique<QOpenGLFunctions_1_1>())
	, _textureLoader(std::make_unique<graphics::TextureLoader>(_openglFunctions.get()))

	, _assetProviderRegistry(std::make_unique<AssetProviderRegistry>())
	, _optionsPageRegistry(std::make_unique<OptionsPageRegistry>())

	, _timer(new QTimer(this))

	, _soundSystem(_applicationSettings->ShouldEnableAudioPlayback()
		? std::unique_ptr<ISoundSystem>(std::make_unique<SoundSystem>(CreateQtLoggerSt(HLAMSoundSystem())))
		: std::make_unique<DummySoundSystem>())
	, _worldTime(std::make_unique<WorldTime>())
	, _assets(std::make_unique<AssetList>(this, _logger))
{
	_logger->debug("Initializing OpenGL");

	_graphicsContext->Begin();

	const bool initializedOpenGLFunctions = _openglFunctions->initializeOpenGLFunctions();
	assert(initializedOpenGLFunctions);

	_openGLLogger = new QOpenGLDebugLogger(this);

	if (_openGLLogger->initialize())
	{
		connect(_openGLLogger, &QOpenGLDebugLogger::messageLogged, this, &AssetManager::OnOpenGLDebugMessage);

		_openGLLogger->startLogging();

		for (const auto& msg : _openGLLogger->loggedMessages())
		{
			OnOpenGLDebugMessage(msg);
		}
	}

	_graphicsContext->End();

	_textureLoader->SetResizeToPowerOf2(_applicationSettings->ShouldResizeTexturesToPowerOf2());
	_textureLoader->SetTextureFilters(_applicationSettings->GetMinFilter(), _applicationSettings->GetMagFilter(),
		_applicationSettings->GetMipmapFilter());

	_logger->debug("Initialized OpenGL");

	_timer->setTimerType(Qt::TimerType::PreciseTimer);

	if (!_soundSystem->Initialize())
	{
		throw std::runtime_error("Failed to initialize sound system");
	}

	connect(_guiApplication, &QGuiApplication::applicationStateChanged, this, &AssetManager::OnApplicationStateChanged);

	OnApplicationStateChanged(_guiApplication->applicationState());

	connect(this, &AssetManager::SettingsChanged, this, &AssetManager::OnSettingsChanged);

	connect(_timer, &QTimer::timeout, this, &AssetManager::OnTimerTick);
	connect(_applicationSettings.get(), &ApplicationSettings::TickRateChanged, this, &AssetManager::OnTickRateChanged);
	connect(_applicationSettings.get(), &ApplicationSettings::StylePathChanged, this, &AssetManager::OnStylePathChanged);

	connect(_applicationSettings.get(), &ApplicationSettings::ResizeTexturesToPowerOf2Changed,
		this, [this](bool value) { _textureLoader->SetResizeToPowerOf2(value); });
	connect(_applicationSettings.get(), &ApplicationSettings::TextureFiltersChanged,
		this, [this](graphics::TextureFilter minFilter, graphics::TextureFilter magFilter, graphics::MipmapFilter mipmapFilter)
		{
			_textureLoader->SetTextureFilters(minFilter, magFilter, mipmapFilter);
		});

	connect(_applicationSettings.get(), &ApplicationSettings::SceneWidgetSettingsChanged,
		this, &AssetManager::RecreateSceneWidget);

	_optionsPageRegistry->AddPage(std::make_unique<OptionsPageGeneral>(applicationSettings));
	_optionsPageRegistry->AddPage(std::make_unique<OptionsPageColors>(applicationSettings));
	_optionsPageRegistry->AddPage(std::make_unique<OptionsPageExternalPrograms>(applicationSettings));
	_optionsPageRegistry->AddPage(std::make_unique<OptionsPageFileSystem>());
	_optionsPageRegistry->AddPage(std::make_unique<OptionsPageGameConfigurations>());
	_optionsPageRegistry->AddPage(std::make_unique<OptionsPageStyle>(applicationSettings));
}

AssetManager::~AssetManager()
{
	_soundSystem->Shutdown();

	if (_sceneWidget)
	{
		delete _sceneWidget->GetContainer();
	}

	_graphicsContext->Begin();
	delete _openGLLogger;
	_graphicsContext->End();
}

QSettings* AssetManager::GetSettings() const
{
	return _applicationSettings->GetSettings();
}

ColorSettings* AssetManager::GetColorSettings() const
{
	return _applicationSettings->GetColorSettings();
}

GameConfigurationsSettings* AssetManager::GetGameConfigurations() const
{
	return _applicationSettings->GetGameConfigurations();
}

bool AssetManager::AddPlugin(std::unique_ptr<IAssetManagerPlugin> plugin)
{
	if (!plugin->Initialize(this))
	{
		QMessageBox::critical(nullptr, "Fatal Error", QString{"Error initializing plugin \"%1\""}.arg(plugin->GetName()));
		return false;
	}

	_logger->trace("Adding plugin {}", plugin->GetName());

	_plugins.push_back(std::move(plugin));

	return true;
}

SceneWidget* AssetManager::GetSceneWidget()
{
	if (!_sceneWidget)
	{
		RecreateSceneWidget();
	}

	return _sceneWidget;
}

void AssetManager::RecreateSceneWidget()
{
	auto oldWidget = _sceneWidget.data();

	// The filter needs to be installed on the main window (handles dropping on any child widget),
	// as well as the scene widget (has special behavior due to being OpenGL)
	_sceneWidget = new SceneWidget(this, GetOpenGLFunctions(), GetTextureLoader());
	_sceneWidget->installEventFilter(GetDragNDropEventFilter());

	emit SceneWidgetRecreated();

	// Delete the widget after notifying everybody so any remaining references to this can be removed.
	if (oldWidget)
	{
		delete oldWidget->GetContainer();
	}
}

bool AssetManager::IsInFullscreenMode()
{
	return _mainWindow->isFullScreen();
}

void AssetManager::StartTimer()
{
	_timer->start(1000 / _applicationSettings->GetTickRate());
}

void AssetManager::PauseTimer()
{
	++_timerPauseCount;

	if (_timerPauseCount == 1)
	{
		_timer->stop();
	}
}

void AssetManager::ResumeTimer()
{
	--_timerPauseCount;

	assert(_timerPauseCount >= 0);

	if (_timerPauseCount == 0)
	{
		StartTimer();
	}
}

QString AssetManager::GetPath(const QString& pathName) const
{
	return _applicationSettings->GetSavedPath(pathName);
}

void AssetManager::SetPath(const QString& pathName, const QString& path)
{
	_applicationSettings->SetSavedPath(pathName, path);
}

LaunchExternalProgramResult AssetManager::TryLaunchExternalProgram(
	const QString& programKey, const QStringList& arguments, const std::optional<QString> confirmMessage)
{
	const auto externalPrograms = GetApplicationSettings()->GetExternalPrograms();

	const QString exeFileName = externalPrograms->GetProgram(programKey);

	if (exeFileName.isEmpty())
	{
		return LaunchExternalProgramResult::Failed;
	}

	const QFileInfo info{exeFileName};

	if (!info.exists())
	{
		_logger->error(
			"The external program \"{}\" does not exist. Check the program settings to ensure the path is correct",
			exeFileName);
		return LaunchExternalProgramResult::Failed;
	}

	QMessageBox::StandardButton action = QMessageBox::StandardButton::Yes;

	if (confirmMessage && externalPrograms->PromptExternalProgramLaunch)
	{
		action = QMessageBox::question(GetMainWindow(), "Launch External Program",
			QString{"%1\nLaunch \"%2\"?"}.arg(*confirmMessage).arg(info.fileName()),
			QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::Cancel);
	}

	if (action == QMessageBox::StandardButton::Cancel)
	{
		return LaunchExternalProgramResult::Cancelled;
	}

	QStringList allArguments = QProcess::splitCommand(externalPrograms->GetAdditionalArguments(programKey));

	allArguments.append(arguments);

	_logger->info("Launching external program \"{}\" {}", exeFileName, allArguments.join(" "));

	// Make sure the working directory is the exe location to avoid bugs.
	if (!QProcess::startDetached(exeFileName, allArguments, info.absolutePath()))
	{
		_logger->critical("The external program could not be launched");
	}

	// If process start failed we've already logged an error, caller doesn't need to handle it.
	return LaunchExternalProgramResult::Success;
}

void AssetManager::Start()
{
	GetAssetProviderRegistry()->Initialize();

	_mainWindow = new MainWindow(this);

	_mainWindow->showMaximized();

	// Now load settings to restore window geometry.
	_mainWindow->LoadSettings();

	StartTimer();
}

void AssetManager::OnMainWindowClosing()
{
	_mainWindow = nullptr;

	_timer->stop();

	GetApplicationSettings()->SaveSettings();

	GetAssetProviderRegistry()->Shutdown();

	GetSettings()->sync();

	CallPlugins(&IAssetManagerPlugin::Shutdown);
}

void AssetManager::OnExit()
{
}

void AssetManager::LoadFile(const QString& fileName)
{
	_mainWindow->LoadFile(fileName);
}

void AssetManager::InitializeFileSystem(IFileSystem& fileSystem, const QString& fileName)
{
	GetApplicationSettings()->GetGameConfigurations()->InitializeFileSystem(fileSystem, fileName);
}

void AssetManager::OnApplicationStateChanged(Qt::ApplicationState state)
{
	if (_applicationSettings->MuteAudioWhenNotActive)
	{
		_soundSystem->SetMuted(state != Qt::ApplicationState::ApplicationActive);
	}
}

void AssetManager::OnSettingsChanged()
{
	// Just in case settings are changed while not active.
	if (!_applicationSettings->MuteAudioWhenNotActive)
	{
		_soundSystem->SetMuted(false);
	}
}

void AssetManager::OnTimerTick()
{
	const auto timeMillis{std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count()};

	const double currentTime = timeMillis / 1000.0;

	double flFrameTime = currentTime - _worldTime->GetPreviousRealTime();

	_worldTime->SetRealTime(currentTime);

	if (flFrameTime > 1.0)
	{
		flFrameTime = 0.1;
	}

#if false
	if (flFrameTime < (1.0 / /*max_fps.GetFloat()*/60.0f))
	{
		return;
	}
#endif

	_worldTime->TimeChanged(currentTime);

	if (auto asset = _assets->GetCurrent(); asset)
	{
		asset->GetProvider()->Tick();
	}
}

void AssetManager::OnTickRateChanged(int value)
{
	if (_timer->isActive())
	{
		StartTimer();
	}
}

void AssetManager::OnStylePathChanged(const QString& stylePath)
{
	auto file = std::make_unique<QFile>(stylePath);
	file->open(QFile::ReadOnly | QFile::Text);

	if (file->isOpen())
	{
		auto stream = std::make_unique<QTextStream>(file.get());

		_guiApplication->setStyleSheet(stream->readAll());
	}
	else
	{
		_guiApplication->setStyleSheet({});
	}
}

void AssetManager::OnOpenGLDebugMessage(const QOpenGLDebugMessage& msg)
{
	qInfo(HLAM()) << msg;
}
