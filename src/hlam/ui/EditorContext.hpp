#pragma once

#include <memory>
#include <utility>
#include <vector>

#include <QObject>
#include <QPointer>
#include <QSettings>
#include <QString>
#include <QTimer>
#include <QUuid>

class Asset;
class ColorSettings;
class DragNDropEventFilter;
class FullscreenWidget;
class GameConfigurationsSettings;
class GeneralSettings;
class IAssetProviderRegistry;
class IFileSystem;
class ISoundSystem;
class OptionsPageRegistry;
class QOpenGLFunctions_1_1;
class QStringList;
class QWidget;
class RecentFilesSettings;
class SceneWidget;
class WorldTime;

namespace graphics
{
class IGraphicsContext;
class TextureLoader;
}

enum class LaunchExternalProgramResult
{
	Success = 0,
	Failed,
	Cancelled
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
		std::unique_ptr<graphics::IGraphicsContext>&& graphicsContext,
		std::unique_ptr<IAssetProviderRegistry>&& assetProviderRegistry,
		std::unique_ptr<OptionsPageRegistry>&& optionsPageRegistry,
		const std::shared_ptr<GeneralSettings>& generalSettings,
		const std::shared_ptr<ColorSettings>& colorSettings,
		const std::shared_ptr<RecentFilesSettings>& recentFilesSettings,
		const std::shared_ptr<GameConfigurationsSettings>& gameConfigurationsSettings,
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

	QWidget* GetMainWindow() const { return _mainWindow; }

	void SetMainWindow(QWidget* widget)
	{
		_mainWindow = widget;
	}

	/**
	*	@brief Gets the shared scene widget.
	*/
	SceneWidget* GetSceneWidget();

	void RecreateSceneWidget();

	FullscreenWidget* GetFullscreenWidget() { return _fullscreenWidget; }

	void SetFullscreenWidget(FullscreenWidget* fullscreenWidget)
	{
		_fullscreenWidget = fullscreenWidget;
	}

	void StartTimer();

	QString GetPath(const QString& pathName) const;

	void SetPath(const QString& pathName, const QString& path);

	/**
	*	@brief Try to launch an external program.
	*	The user is asked what to do.
	*/
	LaunchExternalProgramResult TryLaunchExternalProgram(
		QString exeFileName, const QStringList& arguments, const QString& message);

signals:
	/**
	*	@brief Emitted every time a frame tick occurs
	*/
	void Tick();

	void TryingToLoadAsset(const QString& fileName);

	void ActiveAssetChanged(Asset* asset);

	void SettingsChanged();

	void SceneWidgetRecreated();

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

	const std::unique_ptr<graphics::IGraphicsContext> _graphicsContext;
	const std::unique_ptr<QOpenGLFunctions_1_1> _openglFunctions;
	const std::unique_ptr<graphics::TextureLoader> _textureLoader;

	const std::unique_ptr<IAssetProviderRegistry> _assetProviderRegistry;
	const std::unique_ptr<OptionsPageRegistry> _optionsPageRegistry;

	const std::shared_ptr<GeneralSettings> _generalSettings;
	const std::shared_ptr<ColorSettings> _colorSettings;
	const std::shared_ptr<RecentFilesSettings> _recentFilesSettings;
	const std::shared_ptr<GameConfigurationsSettings> _gameConfigurationsSettings;

	QTimer* const _timer;

	const std::unique_ptr<IFileSystem> _fileSystem;
	const std::unique_ptr<ISoundSystem> _soundSystem;
	const std::unique_ptr<WorldTime> _worldTime;

	QWidget* _mainWindow{};
	QPointer<SceneWidget> _sceneWidget;
	FullscreenWidget* _fullscreenWidget{};
};
