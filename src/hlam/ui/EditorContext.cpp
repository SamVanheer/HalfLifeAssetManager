#include <algorithm>
#include <cassert>
#include <chrono>
#include <iterator>
#include <stdexcept>

#include <QFileInfo>
#include <QMessageBox>
#include <QOpenGLFunctions_1_1>
#include <QProcess>

#include <spdlog/logger.h>

#include "filesystem/FileSystem.hpp"
#include "filesystem/IFileSystem.hpp"

#include "graphics/IGraphicsContext.hpp"
#include "graphics/TextureLoader.hpp"

#include "qt/QtLogging.hpp"
#include "qt/QtLogSink.hpp"

#include "soundsystem/DummySoundSystem.hpp"
#include "soundsystem/ISoundSystem.hpp"
#include "soundsystem/SoundSystem.hpp"

#include "ui/DragNDropEventFilter.hpp"
#include "ui/EditorContext.hpp"

#include "ui/assets/Assets.hpp"

#include "ui/options/OptionsPageRegistry.hpp"

#include "ui/settings/ColorSettings.hpp"
#include "ui/settings/GameConfigurationsSettings.hpp"
#include "ui/settings/GeneralSettings.hpp"
#include "ui/settings/PathSettings.hpp"
#include "ui/settings/RecentFilesSettings.hpp"

#include "utility/WorldTime.hpp"

EditorContext::EditorContext(
	QSettings* settings,
	std::unique_ptr<graphics::IGraphicsContext>&& graphicsContext,
	std::unique_ptr<IAssetProviderRegistry>&& assetProviderRegistry,
	std::unique_ptr<OptionsPageRegistry>&& optionsPageRegistry,
	const std::shared_ptr<GeneralSettings>& generalSettings,
	const std::shared_ptr<ColorSettings>& colorSettings,
	const std::shared_ptr<RecentFilesSettings>& recentFilesSettings,
	const std::shared_ptr<GameConfigurationsSettings>& gameConfigurationsSettings,
	QObject* parent)
	: QObject(parent)
	, _settings(settings)
	, _dragNDropEventFilter(new DragNDropEventFilter(this, this))
	, _graphicsContext(std::move(graphicsContext))
	, _openglFunctions(std::make_unique<QOpenGLFunctions_1_1>())
	, _textureLoader(std::make_unique<graphics::TextureLoader>(_openglFunctions.get()))
	, _assetProviderRegistry(std::move(assetProviderRegistry))
	, _optionsPageRegistry(std::move(optionsPageRegistry))
	, _fileSystem(std::make_unique<FileSystem>())
	, _soundSystem(_generalSettings->ShouldEnableAudioPlayback()
		? std::unique_ptr<ISoundSystem>(std::make_unique<SoundSystem>(CreateQtLoggerSt(logging::HLAMSoundSystem())))
		: std::make_unique<DummySoundSystem>())
	, _worldTime(std::make_unique<WorldTime>())
	, _generalSettings(generalSettings)
	, _colorSettings(colorSettings)
	, _recentFilesSettings(recentFilesSettings)
	, _gameConfigurationsSettings(gameConfigurationsSettings)
	, _timer(new QTimer(this))
{
	_settings->setParent(this);

	qCDebug(logging::HLAM) << "Initializing OpenGL";

	_graphicsContext->Begin();
	const bool initializedOpenGLFunctions = _openglFunctions->initializeOpenGLFunctions();
	assert(initializedOpenGLFunctions);
	_graphicsContext->End();

	_textureLoader->SetResizeToPowerOf2(_generalSettings->ShouldResizeTexturesToPowerOf2());
	_textureLoader->SetTextureFilters(
		_generalSettings->GetMinFilter(), _generalSettings->GetMagFilter(), _generalSettings->GetMipmapFilter());

	qCDebug(logging::HLAM) << "Initialized OpenGL";

	_timer->setTimerType(Qt::TimerType::PreciseTimer);

	if (!_soundSystem->Initialize(_fileSystem.get()))
	{
		throw std::runtime_error("Failed to initialize sound system");
	}

	connect(_timer, &QTimer::timeout, this, &EditorContext::OnTimerTick);
	connect(_generalSettings.get(), &GeneralSettings::TickRateChanged, this, &EditorContext::OnTickRateChanged);

	connect(_generalSettings.get(), &GeneralSettings::ResizeTexturesToPowerOf2Changed,
		this, [this](bool value) { _textureLoader->SetResizeToPowerOf2(value); });
	connect(_generalSettings.get(), &GeneralSettings::TextureFiltersChanged,
		this, [this](graphics::TextureFilter minFilter, graphics::TextureFilter magFilter, graphics::MipmapFilter mipmapFilter)
		{
			_textureLoader->SetTextureFilters(minFilter, magFilter, mipmapFilter);
		});
}

EditorContext::~EditorContext()
{
	_soundSystem->Shutdown();
}

void EditorContext::StartTimer()
{
	_timer->start(1000 / _generalSettings->GetTickRate());
}

QString EditorContext::GetPath(const QString& pathName) const
{
	return GetSavedPath(*_settings, pathName);
}

void EditorContext::SetPath(const QString& pathName, const QString& path)
{
	SetSavedPath(*_settings, pathName, path);
}

LaunchExternalProgramResult EditorContext::TryLaunchExternalProgram(QString exeFileName, const QStringList& arguments)
{
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

	if (_generalSettings->PromptExternalProgramLaunch)
	{
		action = QMessageBox::question(GetMainWindow(), "Launch External Program",
			QString{"Launch \"%1\"?"}.arg(info.fileName()),
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

void EditorContext::OnTimerTick()
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

void EditorContext::OnTickRateChanged(int value)
{
	if (_timer->isActive())
	{
		StartTimer();
	}
}
