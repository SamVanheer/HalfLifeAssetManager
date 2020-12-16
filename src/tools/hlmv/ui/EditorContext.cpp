#include <algorithm>
#include <cassert>
#include <iterator>
#include <stdexcept>

#include <QMessageBox>

#include "core/shared/CWorldTime.h"
#include "core/shared/Utility.h"

#include "filesystem/CFileSystem.h"
#include "filesystem/IFileSystem.h"

#include "soundsystem/CSoundSystem.h"
#include "soundsystem/ISoundSystem.h"

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

	//TODO: set up filesystem based on game configuration
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
}

EditorContext::~EditorContext()
{
	_soundSystem->Shutdown();
	_fileSystem->Shutdown();
}

void EditorContext::OnTimerTick()
{
	const double flCurTime = GetCurrentTime();

	double flFrameTime = flCurTime - _worldTime->GetPreviousRealTime();

	_worldTime->SetRealTime(flCurTime);

	if (flFrameTime > 1.0)
	{
		flFrameTime = 0.1;
	}

	//TODO: implement frame limiter setting
	//TODO: investigate how to allow animation to work when framerate is very high
	if (flFrameTime < (1.0 / /*max_fps.GetFloat()*/60.0f))
	{
		return;
	}

	_worldTime->TimeChanged(flCurTime);

	emit Tick();
}
}
