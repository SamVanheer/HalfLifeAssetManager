#pragma once

#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include <QObject>
#include <QLoggingCategory>
#include <QPointer>
#include <QString>
#include <QTimer>

#include <spdlog/logger.h>

class ApplicationSettings;
class AssetList;
class AssetProviderRegistry;
class ColorSettings;
class DragNDropEventFilter;
class FullscreenWidget;
class GameConfigurationsSettings;
class IAssetManagerPlugin;
class IFileSystem;
class ISoundSystem;
class MainWindow;
class OptionsPageRegistry;
class QApplication;
class QOpenGLDebugLogger;
class QOpenGLDebugMessage;
class QOpenGLFunctions_1_1;
class QSettings;
class QStringList;
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

Q_DECLARE_LOGGING_CATEGORY(HLAM)
Q_DECLARE_LOGGING_CATEGORY(HLAMFileSystem)

/**
*	@brief Used to communicate between the main window and edit widgets
*/
class AssetManager final : public QObject
{
	Q_OBJECT

public:
	AssetManager(
		QApplication* guiApplication,
		const std::shared_ptr<ApplicationSettings>& applicationSettings,
		std::unique_ptr<graphics::IGraphicsContext>&& graphicsContext,
		QObject* parent = nullptr);
	~AssetManager();
	AssetManager(const AssetManager&) = delete;
	AssetManager& operator=(const AssetManager&) = delete;

	QApplication* GetGUIApplication() const { return _guiApplication; }

	const std::shared_ptr<spdlog::logger>& GetLogger() const { return _logger; }

	QSettings* GetSettings() const;

	ApplicationSettings* GetApplicationSettings() const { return _applicationSettings.get(); }

	ColorSettings* GetColorSettings() const;

	GameConfigurationsSettings* GetGameConfigurations() const;

	DragNDropEventFilter* GetDragNDropEventFilter() const { return _dragNDropEventFilter; }

	graphics::IGraphicsContext* GetGraphicsContext() { return _graphicsContext.get(); }

	QOpenGLFunctions_1_1* GetOpenGLFunctions() { return _openglFunctions.get(); }

	graphics::TextureLoader* GetTextureLoader() { return _textureLoader.get(); }

	bool AddPlugin(std::unique_ptr<IAssetManagerPlugin> plugin);

	AssetProviderRegistry* GetAssetProviderRegistry() const { return _assetProviderRegistry.get(); }

	OptionsPageRegistry* GetOptionsPageRegistry() const { return _optionsPageRegistry.get(); }

	ISoundSystem* GetSoundSystem() const { return _soundSystem.get(); }

	WorldTime* GetWorldTime() const { return _worldTime.get(); }

	AssetList* GetAssets() const { return _assets.get(); }

	MainWindow* GetMainWindow() const { return _mainWindow; }

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
	LaunchExternalProgramResult TryLaunchExternalProgram(const QString& programKey, const QStringList& arguments,
		const std::optional<QString> confirmMessage = std::nullopt);

	void Start();

	void OnMainWindowClosing();

	void OnExit();

	void LoadFile(const QString& fileName);

	void InitializeFileSystem(IFileSystem& fileSystem, const QString& fileName);

private:
	template<typename TFunction, typename... Args>
	void CallPlugins(TFunction&& function, Args&&... args)
	{
		for (auto& plugin : _plugins)
		{
			(*plugin.*function)(std::forward<Args>(args)...);
		}
	}

signals:
	void SettingsChanged();

	void SceneWidgetRecreated();

	void FullscreenWidgetChanged();

	void LogMessageReceived(QtMsgType type, const QMessageLogContext& context, const QString& msg);

private slots:
	void OnApplicationStateChanged(Qt::ApplicationState state);

	void OnSettingsChanged();

	void OnTimerTick();

	void OnTickRateChanged(int value);

	void OnStylePathChanged(const QString& stylePath);

	void OnOpenGLDebugMessage(const QOpenGLDebugMessage& msg);

private:
	QApplication* const _guiApplication;

	const std::shared_ptr<spdlog::logger> _logger;

	const std::shared_ptr<ApplicationSettings> _applicationSettings;

	DragNDropEventFilter* const _dragNDropEventFilter;

	const std::unique_ptr<graphics::IGraphicsContext> _graphicsContext;
	QOpenGLDebugLogger* _openGLLogger{};
	const std::unique_ptr<QOpenGLFunctions_1_1> _openglFunctions;
	const std::unique_ptr<graphics::TextureLoader> _textureLoader;

	// Plugin destructors should be called after the asset and options page destructors,
	// so it has to be listed before the registries.
	std::vector<std::unique_ptr<IAssetManagerPlugin>> _plugins;

	const std::unique_ptr<AssetProviderRegistry> _assetProviderRegistry;
	const std::unique_ptr<OptionsPageRegistry> _optionsPageRegistry;

	QTimer* const _timer;

	int _timerPauseCount{0};

	const std::unique_ptr<ISoundSystem> _soundSystem;
	const std::unique_ptr<WorldTime> _worldTime;

	const std::unique_ptr<AssetList> _assets;

	MainWindow* _mainWindow{};
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
