#pragma once

#include <memory>
#include <utility>
#include <vector>

#include <QObject>
#include <QSettings>
#include <QTimer>
#include <QUuid>

class CWorldTime;

namespace filesystem
{
class IFileSystem;
}

namespace soundsystem
{
class ISoundSystem;
}

namespace ui
{
namespace assets
{
class Asset;
class IAssetProviderRegistry;
}

namespace options
{
class OptionsPageRegistry;
}

namespace settings
{
class GameConfigurationsSettings;
class GeneralSettings;
class RecentFilesSettings;
}

class LoadedAsset
{
public:

	LoadedAsset(std::unique_ptr<assets::Asset>&& asset, QWidget* editWidget);
	~LoadedAsset();

	LoadedAsset(const LoadedAsset&) = delete;
	LoadedAsset& operator=(const LoadedAsset&) = delete;
	LoadedAsset(LoadedAsset&&) = default;
	LoadedAsset& operator=(LoadedAsset&&) = default;

	assets::Asset* GetAsset() const { return _asset.get(); }

	QWidget* GetEditWidget() const { return _editWidget; }

private:
	std::unique_ptr<assets::Asset> _asset;
	QWidget* _editWidget;
};

/**
*	@brief Used to communicate between the main window and edit widgets
*/
class EditorContext final : public QObject
{
	Q_OBJECT

public:
	EditorContext(
		QSettings* settings,
		const std::shared_ptr<settings::GeneralSettings>& generalSettings,
		const std::shared_ptr<settings::RecentFilesSettings>& recentFilesSettings,
		const std::shared_ptr<settings::GameConfigurationsSettings>& gameConfigurationsSettings,
		std::unique_ptr<options::OptionsPageRegistry>&& optionsPageRegistry,
		std::unique_ptr<assets::IAssetProviderRegistry>&& assetProviderRegistry, QObject* parent = nullptr);
	~EditorContext();
	EditorContext(const EditorContext&) = delete;
	EditorContext& operator=(const EditorContext&) = delete;

	QSettings* GetSettings() const { return _settings; }

	settings::GeneralSettings* GetGeneralSettings() const { return _generalSettings.get(); }

	settings::RecentFilesSettings* GetRecentFiles() const { return _recentFilesSettings.get(); }

	settings::GameConfigurationsSettings* GetGameConfigurations() const { return _gameConfigurationsSettings.get(); }

	QTimer* GetTimer() const { return _timer; }

	options::OptionsPageRegistry* GetOptionsPageRegistry() const { return _optionsPageRegistry.get(); }

	filesystem::IFileSystem* GetFileSystem() const { return _fileSystem.get(); }

	soundsystem::ISoundSystem* GetSoundSystem() const { return _soundSystem.get(); }

	CWorldTime* GetWorldTime() const { return _worldTime.get(); }

	assets::IAssetProviderRegistry* GetAssetProviderRegistry() const { return _assetProviderRegistry.get(); }

	std::vector<LoadedAsset>& GetLoadedAssets() { return _loadedAssets; }

signals:
	/**
	*	@brief Emitted every time a frame tick occurs
	*/
	void Tick();

private slots:
	void OnTimerTick();

private:
	QSettings* const _settings;

	const std::shared_ptr<settings::GeneralSettings> _generalSettings;
	const std::shared_ptr<settings::RecentFilesSettings> _recentFilesSettings;
	const std::shared_ptr<settings::GameConfigurationsSettings> _gameConfigurationsSettings;

	QTimer* const _timer;

	const std::unique_ptr<options::OptionsPageRegistry> _optionsPageRegistry;

	const std::unique_ptr<filesystem::IFileSystem> _fileSystem;
	const std::unique_ptr<soundsystem::ISoundSystem> _soundSystem;
	const std::unique_ptr<CWorldTime> _worldTime;

	const std::unique_ptr<assets::IAssetProviderRegistry> _assetProviderRegistry;

	std::vector<LoadedAsset> _loadedAssets;
};
}
