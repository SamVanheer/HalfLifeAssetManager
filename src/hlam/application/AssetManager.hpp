#pragma once

#include <memory>
#include <utility>
#include <vector>

#include <QObject>
#include <QPointer>
#include <QString>
#include <QTimer>

class ApplicationSettings;
class AssetList;
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
class AssetManager final : public QObject
{
	Q_OBJECT

public:
	AssetManager(
		const std::shared_ptr<ApplicationSettings>& applicationSettings,
		std::unique_ptr<graphics::IGraphicsContext>&& graphicsContext,
		std::unique_ptr<AssetProviderRegistry>&& assetProviderRegistry,
		std::unique_ptr<OptionsPageRegistry>&& optionsPageRegistry,
		QObject* parent = nullptr);
	~AssetManager();
	AssetManager(const AssetManager&) = delete;
	AssetManager& operator=(const AssetManager&) = delete;

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

	AssetList* GetAssets() const { return _assets.get(); }

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

	FullscreenWidget* GetFullscreenWidget() const { return _fullscreenWidget.get(); }

	void ToggleFullscreen();

	void ExitFullscreen();

	void StartTimer();

	/**
	*	@brief Pause the timer to suspend Tick signals.
	*	Each call to PauseTimer must be matched with a call to @ref ResumeTimer.
	*/
	void PauseTimer();

	/**
	*	@see PauseTimer
	*/
	void ResumeTimer();

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

	void SettingsChanged();

	void SceneWidgetRecreated();

	void FullscreenWidgetChanged();

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

	int _timerPauseCount{0};

	const std::unique_ptr<ISoundSystem> _soundSystem;
	const std::unique_ptr<WorldTime> _worldTime;

	const std::unique_ptr<AssetList> _assets;

	QWidget* _mainWindow{};
	QPointer<SceneWidget> _sceneWidget;
	std::unique_ptr<FullscreenWidget> _fullscreenWidget;
};

struct TimerSuspender final
{
	explicit TimerSuspender(AssetManager* application)
		: Application(application)
	{
		Application->PauseTimer();
	}

	~TimerSuspender()
	{
		Application->ResumeTimer();
	}

	AssetManager* const Application;
};
