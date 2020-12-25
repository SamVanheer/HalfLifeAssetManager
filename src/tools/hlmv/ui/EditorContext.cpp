#include <algorithm>
#include <cassert>
#include <iterator>
#include <stdexcept>

#include <QOffscreenSurface>
#include <QOpenGLContext>

#include "core/shared/CWorldTime.hpp"
#include "core/shared/Utility.hpp"

#include "filesystem/CFileSystem.hpp"
#include "filesystem/IFileSystem.hpp"

#include "soundsystem/CSoundSystem.hpp"
#include "soundsystem/ISoundSystem.hpp"

#include "ui/EditorContext.hpp"

#include "ui/assets/Assets.hpp"

#include "ui/options/OptionsPageRegistry.hpp"

#include "ui/settings/ColorSettings.hpp"
#include "ui/settings/GameConfigurationsSettings.hpp"
#include "ui/settings/GeneralSettings.hpp"
#include "ui/settings/RecentFilesSettings.hpp"

namespace ui
{
EditorContext::EditorContext(
	QSettings* settings,
	const std::shared_ptr<settings::GeneralSettings>& generalSettings,
	const std::shared_ptr<settings::ColorSettings>& colorSettings,
	const std::shared_ptr<settings::RecentFilesSettings>& recentFilesSettings,
	const std::shared_ptr<settings::GameConfigurationsSettings>& gameConfigurationsSettings,
	std::unique_ptr<options::OptionsPageRegistry>&& optionsPageRegistry,
	std::unique_ptr<assets::IAssetProviderRegistry>&& assetProviderRegistry,
	QObject* parent)
	: QObject(parent)
	, _settings(settings)
	, _generalSettings(generalSettings)
	, _colorSettings(colorSettings)
	, _recentFilesSettings(recentFilesSettings)
	, _gameConfigurationsSettings(gameConfigurationsSettings)
	, _timer(new QTimer(this))
	, _optionsPageRegistry(std::move(optionsPageRegistry))
	, _fileSystem(std::make_unique<filesystem::CFileSystem>())
	, _soundSystem(std::make_unique<soundsystem::CSoundSystem>())
	, _worldTime(std::make_unique<CWorldTime>())
	, _assetProviderRegistry(std::move(assetProviderRegistry))
{
	_settings->setParent(this);

	_timer->setTimerType(Qt::TimerType::PreciseTimer);

	if (!_fileSystem->Initialize())
	{
		throw std::runtime_error("Failed to initialize file system");
	}

	if (!_soundSystem->Initialize(_fileSystem.get()))
	{
		throw std::runtime_error("Failed to initialize sound system");
	}

	connect(_timer, &QTimer::timeout, this, &EditorContext::OnTimerTick);
	connect(_generalSettings.get(), &settings::GeneralSettings::MaxFPSChanged, this, &EditorContext::OnMaxFPSChanged);
}

EditorContext::~EditorContext()
{
	_soundSystem->Shutdown();
	_fileSystem->Shutdown();
}

void EditorContext::SetOffscreenContext(QOpenGLContext* offscreenContext)
{
	if (_offscreenContext)
	{
		delete _offscreenContext;
	}

	_offscreenContext = offscreenContext;

	if (_offscreenContext)
	{
		_offscreenContext->setParent(this);
	}
}

void EditorContext::SetOffscreenSurface(QOffscreenSurface* offscreenSurface)
{
	if (_offscreenSurface)
	{
		delete _offscreenSurface;
	}

	_offscreenSurface = offscreenSurface;

	if (_offscreenSurface)
	{
		_offscreenSurface->setParent(this);
	}
}

void EditorContext::StartTimer()
{
	_timer->start(static_cast<int>(1000.0 / _generalSettings->GetMaxFPS()));
}

void EditorContext::OnTimerTick()
{
	const double currentTime = GetCurrentTime();

	double flFrameTime = currentTime - _worldTime->GetPreviousRealTime();

	_worldTime->SetRealTime(currentTime);

	if (flFrameTime > 1.0)
	{
		flFrameTime = 0.1;
	}

	//TODO: investigate how to allow animation to work when framerate is very high
#if false
	if (flFrameTime < (1.0 / /*max_fps.GetFloat()*/60.0f))
	{
		return;
	}
#endif

	_worldTime->TimeChanged(currentTime);

	emit Tick();
}

void EditorContext::OnMaxFPSChanged(float value)
{
	if (_timer->isActive())
	{
		StartTimer();
	}
}
}
