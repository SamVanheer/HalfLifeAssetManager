#include <algorithm>
#include <cassert>
#include <chrono>
#include <iterator>
#include <stdexcept>

#include <QFileInfo>
#include <QLayout>
#include <QMessageBox>
#include <QOpenGLFunctions_1_1>
#include <QProcess>

#include <spdlog/logger.h>

#include "assets/Assets.hpp"

#include "filesystem/FileSystem.hpp"
#include "filesystem/IFileSystem.hpp"

#include "graphics/IGraphicsContext.hpp"
#include "graphics/TextureLoader.hpp"

#include "qt/QtLogging.hpp"
#include "qt/QtLogSink.hpp"

#include "settings/ApplicationSettings.hpp"
#include "settings/GameConfigurationsSettings.hpp"

#include "soundsystem/DummySoundSystem.hpp"
#include "soundsystem/ISoundSystem.hpp"
#include "soundsystem/SoundSystem.hpp"

#include "ui/DragNDropEventFilter.hpp"
#include "application/AssetManager.hpp"
#include "ui/SceneWidget.hpp"

#include "ui/options/OptionsPageRegistry.hpp"

#include "utility/WorldTime.hpp"

AssetManager::AssetManager(
	const std::shared_ptr<ApplicationSettings>& applicationSettings,
	std::unique_ptr<graphics::IGraphicsContext>&& graphicsContext,
	std::unique_ptr<AssetProviderRegistry>&& assetProviderRegistry,
	std::unique_ptr<OptionsPageRegistry>&& optionsPageRegistry,
	QObject* parent)
	: QObject(parent)
	, _applicationSettings(applicationSettings)
	, _dragNDropEventFilter(new DragNDropEventFilter(this, this))

	, _graphicsContext(std::move(graphicsContext))
	, _openglFunctions(std::make_unique<QOpenGLFunctions_1_1>())
	, _textureLoader(std::make_unique<graphics::TextureLoader>(_openglFunctions.get()))

	, _assetProviderRegistry(std::move(assetProviderRegistry))
	, _optionsPageRegistry(std::move(optionsPageRegistry))

	, _timer(new QTimer(this))

	, _soundSystem(_applicationSettings->ShouldEnableAudioPlayback()
		? std::unique_ptr<ISoundSystem>(std::make_unique<SoundSystem>(CreateQtLoggerSt(logging::HLAMSoundSystem())))
		: std::make_unique<DummySoundSystem>())
	, _worldTime(std::make_unique<WorldTime>())
{
	qCDebug(logging::HLAM) << "Initializing OpenGL";

	_graphicsContext->Begin();
	const bool initializedOpenGLFunctions = _openglFunctions->initializeOpenGLFunctions();
	assert(initializedOpenGLFunctions);
	_graphicsContext->End();

	_textureLoader->SetResizeToPowerOf2(_applicationSettings->ShouldResizeTexturesToPowerOf2());
	_textureLoader->SetTextureFilters(_applicationSettings->GetMinFilter(), _applicationSettings->GetMagFilter(),
		_applicationSettings->GetMipmapFilter());

	qCDebug(logging::HLAM) << "Initialized OpenGL";

	_timer->setTimerType(Qt::TimerType::PreciseTimer);

	if (!_soundSystem->Initialize())
	{
		throw std::runtime_error("Failed to initialize sound system");
	}

	connect(_timer, &QTimer::timeout, this, &AssetManager::OnTimerTick);
	connect(_applicationSettings.get(), &ApplicationSettings::TickRateChanged, this, &AssetManager::OnTickRateChanged);

	connect(_applicationSettings.get(), &ApplicationSettings::ResizeTexturesToPowerOf2Changed,
		this, [this](bool value) { _textureLoader->SetResizeToPowerOf2(value); });
	connect(_applicationSettings.get(), &ApplicationSettings::TextureFiltersChanged,
		this, [this](graphics::TextureFilter minFilter, graphics::TextureFilter magFilter, graphics::MipmapFilter mipmapFilter)
		{
			_textureLoader->SetTextureFilters(minFilter, magFilter, mipmapFilter);
		});

	connect(_applicationSettings.get(), &ApplicationSettings::MSAALevelChanged, this, &AssetManager::RecreateSceneWidget);
}

AssetManager::~AssetManager()
{
	_soundSystem->Shutdown();

	if (_sceneWidget)
	{
		delete _sceneWidget->GetContainer();
	}
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
	const QString& programKey, const QStringList& arguments, const QString& message)
{
	const auto externalPrograms = GetApplicationSettings()->GetExternalPrograms();

	QString exeFileName = externalPrograms->GetProgram(programKey);

	exeFileName = exeFileName.trimmed();

	if (exeFileName.isEmpty())
	{
		return LaunchExternalProgramResult::Failed;
	}

	const QFileInfo info{exeFileName};

	if (!info.exists())
	{
		const QString errorMessage = QString{
			"The external program \"%1\" does not exist. Check the program settings to ensure the path is correct."}
				.arg(exeFileName);
		QMessageBox::critical(GetMainWindow(), "Error Launching External Program", errorMessage);
		return LaunchExternalProgramResult::Failed;
	}

	QMessageBox::StandardButton action = QMessageBox::StandardButton::Yes;

	if (externalPrograms->PromptExternalProgramLaunch)
	{
		action = QMessageBox::question(GetMainWindow(), "Launch External Program",
			QString{"%1\nLaunch \"%2\"?"}.arg(message).arg(info.fileName()),
			QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::Cancel);
	}

	if (action == QMessageBox::StandardButton::Cancel)
	{
		return LaunchExternalProgramResult::Cancelled;
	}

	// Make sure the working directory is the exe location to avoid bugs.
	if (!QProcess::startDetached(exeFileName, arguments, info.absolutePath()))
	{
		QMessageBox::critical(GetMainWindow(), "Error Launching External Program", 
			QString{"The external program \"%1\" could not be launched.\nArguments:\n%2"}
				.arg(exeFileName)
				.arg(arguments.join("\n")));
	}

	// If process start failed we've already logged an error, caller doesn't need to handle it.
	return LaunchExternalProgramResult::Success;
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

	emit Tick();
}

void AssetManager::OnTickRateChanged(int value)
{
	if (_timer->isActive())
	{
		StartTimer();
	}
}
