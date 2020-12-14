#pragma once

#include <memory>
#include <utility>
#include <vector>

#include <QObject>
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
class GameConfiguration;
class GameEnvironment;
class OptionsPageRegistry;
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
	static constexpr int MinimumFloorLength = 0;
	static constexpr int MaximumFloorLength = 2048;
	static constexpr int DefaultFloorLength = 100;

	EditorContext(
		std::unique_ptr<options::OptionsPageRegistry>&& optionsPageRegistry,
		std::unique_ptr<assets::IAssetProviderRegistry>&& assetProviderRegistry, QObject* parent = nullptr);
	~EditorContext();
	EditorContext(const EditorContext&) = delete;
	EditorContext& operator=(const EditorContext&) = delete;

	QTimer* GetTimer() const { return _timer; }

	options::OptionsPageRegistry* GetOptionsPageRegistry() const { return _optionsPageRegistry.get(); }

	filesystem::IFileSystem* GetFileSystem() const { return _fileSystem.get(); }

	soundsystem::ISoundSystem* GetSoundSystem() const { return _soundSystem.get(); }

	assets::IAssetProviderRegistry* GetAssetProviderRegistry() const { return _assetProviderRegistry.get(); }

	std::vector<LoadedAsset>& GetLoadedAssets() { return _loadedAssets; }

	std::vector<options::GameEnvironment*> GetGameEnvironments() const;

	options::GameEnvironment* GetGameEnvironmentById(const QUuid& id) const;

	void AddGameEnvironment(std::unique_ptr<options::GameEnvironment>&& gameEnvironment);

	void RemoveGameEnvironment(const QUuid& id);

	std::pair<options::GameEnvironment*, options::GameConfiguration*> GetActiveConfiguration() const { return _activeConfiguration; }

	void SetActiveConfiguration(const std::pair<options::GameEnvironment*, options::GameConfiguration*>& configuration)
	{
		if (_activeConfiguration != configuration)
		{
			const auto previous = _activeConfiguration;

			_activeConfiguration = configuration;
			
			emit ActiveConfigurationChanged(configuration, previous);
		}
	}

	int GetFloorLength() const { return _floorLength; }

signals:
	/**
	*	@brief Emitted every time a frame tick occurs
	*/
	void Tick();

	void GameEnvironmentAdded(options::GameEnvironment* gameEnvironment);
	void GameEnvironmentRemoved(options::GameEnvironment* gameEnvironment);

	void ActiveConfigurationChanged(const std::pair<options::GameEnvironment*, options::GameConfiguration*>& current,
		const std::pair<options::GameEnvironment*, options::GameConfiguration*>& previous);

	void FloorLengthChanged(int length);

public slots:
	void SetFloorLength(int value)
	{
		if (_floorLength != value)
		{
			_floorLength = value;

			emit FloorLengthChanged(_floorLength);
		}
	}

private slots:
	void OnTimerTick();

private:
	QTimer* const _timer;

	const std::unique_ptr<options::OptionsPageRegistry> _optionsPageRegistry;

	std::unique_ptr<filesystem::IFileSystem> _fileSystem;
	std::unique_ptr<soundsystem::ISoundSystem> _soundSystem;

	std::unique_ptr<assets::IAssetProviderRegistry> _assetProviderRegistry;

	std::vector<LoadedAsset> _loadedAssets;

	//TODO: game environments should be moved to a separate collection type
	std::vector<std::unique_ptr<options::GameEnvironment>> _gameEnvironments;

	std::pair<options::GameEnvironment*, options::GameConfiguration*> _activeConfiguration{};

	int _floorLength = DefaultFloorLength;
};
}
