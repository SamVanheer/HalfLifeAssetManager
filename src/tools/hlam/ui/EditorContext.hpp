#pragma once

#include <memory>
#include <utility>
#include <vector>

#include <QObject>
#include <QSettings>
#include <QTimer>
#include <QUuid>

class QOffscreenSurface;
class QOpenGLContext;

class WorldTime;

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
class DragNDropEventFilter;

namespace assets
{
class IAssetProviderRegistry;
}

namespace options
{
class OptionsPageRegistry;
}

namespace settings
{
class ColorSettings;
class GameConfigurationsSettings;
class GeneralSettings;
class RecentFilesSettings;
}

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
		const std::shared_ptr<settings::ColorSettings>& colorSettings,
		const std::shared_ptr<settings::RecentFilesSettings>& recentFilesSettings,
		const std::shared_ptr<settings::GameConfigurationsSettings>& gameConfigurationsSettings,
		std::unique_ptr<options::OptionsPageRegistry>&& optionsPageRegistry,
		std::unique_ptr<assets::IAssetProviderRegistry>&& assetProviderRegistry,
		QObject* parent = nullptr);
	~EditorContext();
	EditorContext(const EditorContext&) = delete;
	EditorContext& operator=(const EditorContext&) = delete;

	QSettings* GetSettings() const { return _settings; }

	DragNDropEventFilter* GetDragNDropEventFilter() const { return _dragNDropEventFilter; }

	settings::GeneralSettings* GetGeneralSettings() const { return _generalSettings.get(); }

	settings::ColorSettings* GetColorSettings() const { return _colorSettings.get(); }

	settings::RecentFilesSettings* GetRecentFiles() const { return _recentFilesSettings.get(); }

	settings::GameConfigurationsSettings* GetGameConfigurations() const { return _gameConfigurationsSettings.get(); }

	QTimer* GetTimer() const { return _timer; }

	options::OptionsPageRegistry* GetOptionsPageRegistry() const { return _optionsPageRegistry.get(); }

	filesystem::IFileSystem* GetFileSystem() const { return _fileSystem.get(); }

	soundsystem::ISoundSystem* GetSoundSystem() const { return _soundSystem.get(); }

	WorldTime* GetWorldTime() const { return _worldTime.get(); }

	assets::IAssetProviderRegistry* GetAssetProviderRegistry() const { return _assetProviderRegistry.get(); }

	QOpenGLContext* GetOffscreenContext() const { return _offscreenContext; }

	void SetOffscreenContext(QOpenGLContext* offscreenContext);

	QOffscreenSurface* GetOffscreenSurface() const { return _offscreenSurface; }

	void SetOffscreenSurface(QOffscreenSurface* offscreenSurface);

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

	const std::shared_ptr<settings::GeneralSettings> _generalSettings;
	const std::shared_ptr<settings::ColorSettings> _colorSettings;
	const std::shared_ptr<settings::RecentFilesSettings> _recentFilesSettings;
	const std::shared_ptr<settings::GameConfigurationsSettings> _gameConfigurationsSettings;

	QTimer* const _timer;

	const std::unique_ptr<options::OptionsPageRegistry> _optionsPageRegistry;

	const std::unique_ptr<filesystem::IFileSystem> _fileSystem;
	const std::unique_ptr<soundsystem::ISoundSystem> _soundSystem;
	const std::unique_ptr<WorldTime> _worldTime;

	const std::unique_ptr<assets::IAssetProviderRegistry> _assetProviderRegistry;

	QOpenGLContext* _offscreenContext{};
	QOffscreenSurface* _offscreenSurface{};
};
}
