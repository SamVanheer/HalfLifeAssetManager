#include <algorithm>
#include <cassert>
#include <iterator>
#include <stdexcept>

#include <QMessageBox>

#include "core/shared/CWorldTime.hpp"
#include "core/shared/Utility.hpp"

#include "filesystem/CFileSystem.hpp"
#include "filesystem/IFileSystem.hpp"

#include "soundsystem/CSoundSystem.hpp"
#include "soundsystem/ISoundSystem.hpp"

#include "ui/EditorContext.hpp"

#include "ui/assets/Assets.hpp"

#include "ui/options/OptionsPageRegistry.hpp"

#include "ui/settings/GameConfigurationsSettings.hpp"
#include "ui/settings/GeneralSettings.hpp"
#include "ui/settings/RecentFilesSettings.hpp"

namespace ui
{
LoadedAsset::LoadedAsset(std::unique_ptr<assets::Asset>&& asset, QWidget* editWidget)
	: _asset(std::move(asset))
	, _editWidget(editWidget)
{
}

LoadedAsset::~LoadedAsset() = default;

EditorContext::EditorContext(
	QSettings* settings,
	const std::shared_ptr<settings::GeneralSettings>& generalSettings,
	const std::shared_ptr<settings::RecentFilesSettings>& recentFilesSettings,
	const std::shared_ptr<settings::GameConfigurationsSettings>& gameConfigurationsSettings,
	std::unique_ptr<options::OptionsPageRegistry>&& optionsPageRegistry,
	std::unique_ptr<assets::IAssetProviderRegistry>&& assetProviderRegistry, QObject* parent)
	: QObject(parent)
	, _settings(settings)
	, _generalSettings(generalSettings)
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
		QMessageBox::critical(nullptr, "Fatal Error", "Failed to initialize file system");
		//TODO: ensure error handling cleans up properly
		throw std::runtime_error("Failed to initialize file system");
	}

	if (!_soundSystem->Initialize(_fileSystem.get()))
	{
		QMessageBox::critical(nullptr, "Fatal Error", "Failed to initialize sound system");
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
