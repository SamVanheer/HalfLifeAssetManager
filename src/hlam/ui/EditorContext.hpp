#pragma once

#include <memory>
#include <utility>
#include <vector>

#include <QObject>
#include <QSettings>
#include <QTimer>
#include <QUuid>

class QOpenGLFunctions_1_1;
class WorldTime;

class IFileSystem;

namespace graphics
{
class IGraphicsContext;
class TextureLoader;
}

class ISoundSystem;

class DragNDropEventFilter;

class IAssetProviderRegistry;

class OptionsPageRegistry;

class ColorSettings;
class GameConfigurationsSettings;
class GeneralSettings;
class RecentFilesSettings;

/**
*	@brief Used to communicate between the main window and edit widgets
*/
class EditorContext final : public QObject
{
	Q_OBJECT

public:
	EditorContext(
		QSettings* settings,
		const std::shared_ptr<GeneralSettings>& generalSettings,
		const std::shared_ptr<ColorSettings>& colorSettings,
		const std::shared_ptr<RecentFilesSettings>& recentFilesSettings,
		const std::shared_ptr<GameConfigurationsSettings>& gameConfigurationsSettings,
		std::unique_ptr<OptionsPageRegistry>&& optionsPageRegistry,
		std::unique_ptr<IAssetProviderRegistry>&& assetProviderRegistry,
		std::unique_ptr<graphics::IGraphicsContext>&& graphicsContext,
		QObject* parent = nullptr);
	~EditorContext();
	EditorContext(const EditorContext&) = delete;
	EditorContext& operator=(const EditorContext&) = delete;

	QSettings* GetSettings() const { return _settings; }

	DragNDropEventFilter* GetDragNDropEventFilter() const { return _dragNDropEventFilter; }

	GeneralSettings* GetGeneralSettings() const { return _generalSettings.get(); }

	ColorSettings* GetColorSettings() const { return _colorSettings.get(); }

	RecentFilesSettings* GetRecentFiles() const { return _recentFilesSettings.get(); }

	GameConfigurationsSettings* GetGameConfigurations() const { return _gameConfigurationsSettings.get(); }

	QTimer* GetTimer() const { return _timer; }

	OptionsPageRegistry* GetOptionsPageRegistry() const { return _optionsPageRegistry.get(); }

	IFileSystem* GetFileSystem() const { return _fileSystem.get(); }

	ISoundSystem* GetSoundSystem() const { return _soundSystem.get(); }

	WorldTime* GetWorldTime() const { return _worldTime.get(); }

	IAssetProviderRegistry* GetAssetProviderRegistry() const { return _assetProviderRegistry.get(); }

	graphics::IGraphicsContext* GetGraphicsContext() { return _graphicsContext.get(); }

	QOpenGLFunctions_1_1* GetOpenGLFunctions() { return _openglFunctions.get(); }

	graphics::TextureLoader* GetTextureLoader() { return _textureLoader.get(); }

	void StartTimer();

signals:
	/**
	*	@brief Emitted every time a frame tick occurs
	*/
	void Tick();

	void TryingToLoadAsset(const QString& fileName);

	void SettingsChanged();

public slots:
	void TryLoadAsset(const QString& fileName)
	{
		emit TryingToLoadAsset(fileName);
	}

private slots:
	void OnTimerTick();

	void OnTickRateChanged(int value);

private:
	QSettings* const _settings;
	DragNDropEventFilter* const _dragNDropEventFilter;

	const std::shared_ptr<GeneralSettings> _generalSettings;
	const std::shared_ptr<ColorSettings> _colorSettings;
	const std::shared_ptr<RecentFilesSettings> _recentFilesSettings;
	const std::shared_ptr<GameConfigurationsSettings> _gameConfigurationsSettings;

	QTimer* const _timer;

	const std::unique_ptr<OptionsPageRegistry> _optionsPageRegistry;

	const std::unique_ptr<IFileSystem> _fileSystem;
	const std::unique_ptr<ISoundSystem> _soundSystem;
	const std::unique_ptr<WorldTime> _worldTime;

	const std::unique_ptr<IAssetProviderRegistry> _assetProviderRegistry;

	const std::unique_ptr<graphics::IGraphicsContext> _graphicsContext;
	const std::unique_ptr<QOpenGLFunctions_1_1> _openglFunctions;
	const std::unique_ptr<graphics::TextureLoader> _textureLoader;
};
