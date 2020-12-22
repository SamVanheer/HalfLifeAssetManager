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
		std::unique_ptr<assets::IAssetProviderRegistry>&& assetProviderRegistry, QObject* parent = nullptr);
	~EditorContext();
	EditorContext(const EditorContext&) = delete;
	EditorContext& operator=(const EditorContext&) = delete;

	QSettings* GetSettings() const { return _settings; }

	settings::GeneralSettings* GetGeneralSettings() const { return _generalSettings.get(); }

	settings::ColorSettings* GetColorSettings() const { return _colorSettings.get(); }

	settings::RecentFilesSettings* GetRecentFiles() const { return _recentFilesSettings.get(); }

	settings::GameConfigurationsSettings* GetGameConfigurations() const { return _gameConfigurationsSettings.get(); }

	QTimer* GetTimer() const { return _timer; }

	options::OptionsPageRegistry* GetOptionsPageRegistry() const { return _optionsPageRegistry.get(); }

	filesystem::IFileSystem* GetFileSystem() const { return _fileSystem.get(); }

	soundsystem::ISoundSystem* GetSoundSystem() const { return _soundSystem.get(); }

	CWorldTime* GetWorldTime() const { return _worldTime.get(); }

	assets::IAssetProviderRegistry* GetAssetProviderRegistry() const { return _assetProviderRegistry.get(); }

	void StartTimer();

signals:
	/**
	*	@brief Emitted every time a frame tick occurs
	*/
	void Tick();

private slots:
	void OnTimerTick();

	void OnMaxFPSChanged(float value);

private:
	QSettings* const _settings;

	const std::shared_ptr<settings::GeneralSettings> _generalSettings;
	const std::shared_ptr<settings::ColorSettings> _colorSettings;
	const std::shared_ptr<settings::RecentFilesSettings> _recentFilesSettings;
	const std::shared_ptr<settings::GameConfigurationsSettings> _gameConfigurationsSettings;

	QTimer* const _timer;

	const std::unique_ptr<options::OptionsPageRegistry> _optionsPageRegistry;

	const std::unique_ptr<filesystem::IFileSystem> _fileSystem;
	const std::unique_ptr<soundsystem::ISoundSystem> _soundSystem;
	const std::unique_ptr<CWorldTime> _worldTime;

	const std::unique_ptr<assets::IAssetProviderRegistry> _assetProviderRegistry;
};
}
