#pragma once

#include <memory>
#include <utility>
#include <vector>

#include <QObject>
#include <QPointer>
#include <QString>
#include <QTimer>

class ApplicationSettings;
class Asset;
class AssetProviderRegistry;
class ColorSettings;
class DragNDropEventFilter;
class FullscreenWidget;
class GameConfigurationsSettings;
class ISoundSystem;
class OptionsPageRegistry;
class QOpenGLFunctions_1_1;
class QSettings;
class QStringList;
class QWidget;
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
		const std::shared_ptr<ApplicationSettings>& applicationSettings,
		std::unique_ptr<graphics::IGraphicsContext>&& graphicsContext,
		std::unique_ptr<AssetProviderRegistry>&& assetProviderRegistry,
		std::unique_ptr<OptionsPageRegistry>&& optionsPageRegistry,
		QObject* parent = nullptr);
	~EditorContext();
	EditorContext(const EditorContext&) = delete;
	EditorContext& operator=(const EditorContext&) = delete;

	QSettings* GetSettings() const;

	ApplicationSettings* GetApplicationSettings() const { return _applicationSettings.get(); }

	ColorSettings* GetColorSettings() const;

	GameConfigurationsSettings* GetGameConfigurations() const;

	DragNDropEventFilter* GetDragNDropEventFilter() const { return _dragNDropEventFilter; }

	graphics::IGraphicsContext* GetGraphicsContext() { return _graphicsContext.get(); }

	QOpenGLFunctions_1_1* GetOpenGLFunctions() { return _openglFunctions.get(); }

	graphics::TextureLoader* GetTextureLoader() { return _textureLoader.get(); }

	AssetProviderRegistry* GetAssetProviderRegistry() const { return _assetProviderRegistry.get(); }

	OptionsPageRegistry* GetOptionsPageRegistry() const { return _optionsPageRegistry.get(); }

	QTimer* GetTimer() const { return _timer; }

	ISoundSystem* GetSoundSystem() const { return _soundSystem.get(); }

	WorldTime* GetWorldTime() const { return _worldTime.get(); }

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
		const QString& programKey, const QStringList& arguments, const QString& message);

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
	const std::shared_ptr<ApplicationSettings> _applicationSettings;

	DragNDropEventFilter* const _dragNDropEventFilter;

	const std::unique_ptr<graphics::IGraphicsContext> _graphicsContext;
	const std::unique_ptr<QOpenGLFunctions_1_1> _openglFunctions;
	const std::unique_ptr<graphics::TextureLoader> _textureLoader;

	const std::unique_ptr<AssetProviderRegistry> _assetProviderRegistry;
	const std::unique_ptr<OptionsPageRegistry> _optionsPageRegistry;

	QTimer* const _timer;

	const std::unique_ptr<ISoundSystem> _soundSystem;
	const std::unique_ptr<WorldTime> _worldTime;

	QWidget* _mainWindow{};
	QPointer<SceneWidget> _sceneWidget;
	FullscreenWidget* _fullscreenWidget{};
};
