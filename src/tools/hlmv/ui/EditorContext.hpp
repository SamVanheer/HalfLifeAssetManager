#pragma once

#include <memory>
#include <utility>
#include <vector>

#include <QObject>
#include <QSettings>
#include <QTimer>
#include <QUuid>

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
class GameConfiguration;
class GameEnvironment;
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
		std::unique_ptr<options::OptionsPageRegistry>&& optionsPageRegistry,
		std::unique_ptr<assets::IAssetProviderRegistry>&& assetProviderRegistry, QObject* parent = nullptr);
	~EditorContext();
	EditorContext(const EditorContext&) = delete;
	EditorContext& operator=(const EditorContext&) = delete;

	QSettings* GetSettings() const { return _settings; }

	settings::GeneralSettings* GetGeneralSettings() const { return _generalSettings.get(); }

	settings::RecentFilesSettings* GetRecentFiles() const { return _recentFilesSettings.get(); }

	QTimer* GetTimer() const { return _timer; }

	options::OptionsPageRegistry* GetOptionsPageRegistry() const { return _optionsPageRegistry.get(); }

	filesystem::IFileSystem* GetFileSystem() const { return _fileSystem.get(); }

	soundsystem::ISoundSystem* GetSoundSystem() const { return _soundSystem.get(); }

	assets::IAssetProviderRegistry* GetAssetProviderRegistry() const { return _assetProviderRegistry.get(); }

	std::vector<LoadedAsset>& GetLoadedAssets() { return _loadedAssets; }

	std::vector<settings::GameEnvironment*> GetGameEnvironments() const;

	settings::GameEnvironment* GetGameEnvironmentById(const QUuid& id) const;

	void AddGameEnvironment(std::unique_ptr<settings::GameEnvironment>&& gameEnvironment);

	void RemoveGameEnvironment(const QUuid& id);

	std::pair<settings::GameEnvironment*, settings::GameConfiguration*> GetActiveConfiguration() const { return _activeConfiguration; }

	void SetActiveConfiguration(const std::pair<settings::GameEnvironment*, settings::GameConfiguration*>& configuration)
	{
		if (_activeConfiguration != configuration)
		{
			const auto previous = _activeConfiguration;

			_activeConfiguration = configuration;
			
			emit ActiveConfigurationChanged(configuration, previous);
		}
	}

signals:
	/**
	*	@brief Emitted every time a frame tick occurs
	*/
	void Tick();

	void GameEnvironmentAdded(settings::GameEnvironment* gameEnvironment);
	void GameEnvironmentRemoved(settings::GameEnvironment* gameEnvironment);

	void ActiveConfigurationChanged(const std::pair<settings::GameEnvironment*, settings::GameConfiguration*>& current,
		const std::pair<settings::GameEnvironment*, settings::GameConfiguration*>& previous);

private slots:
	void OnTimerTick();

private:
	QSettings* const _settings;

	const std::shared_ptr<settings::GeneralSettings> _generalSettings;
	const std::shared_ptr<settings::RecentFilesSettings> _recentFilesSettings;

	QTimer* const _timer;

	const std::unique_ptr<options::OptionsPageRegistry> _optionsPageRegistry;

	std::unique_ptr<filesystem::IFileSystem> _fileSystem;
	std::unique_ptr<soundsystem::ISoundSystem> _soundSystem;

	std::unique_ptr<assets::IAssetProviderRegistry> _assetProviderRegistry;

	std::vector<LoadedAsset> _loadedAssets;

	//TODO: game environments should be moved to a separate collection type
	std::vector<std::unique_ptr<settings::GameEnvironment>> _gameEnvironments;

	std::pair<settings::GameEnvironment*, settings::GameConfiguration*> _activeConfiguration{};
};
}
