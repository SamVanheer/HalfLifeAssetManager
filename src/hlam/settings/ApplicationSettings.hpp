#pragma once

#include <memory>

#include <QObject>
#include <QString>
#include <QVector>

#include <spdlog/logger.h>

#include <glm/vec2.hpp>

#include "graphics/TextureLoader.hpp"

class BaseSettings;
class ColorSettings;
class ExternalProgramSettings;
class GameConfigurationsSettings;
class QSettings;
class RecentFilesSettings;

class ApplicationSettings final : public QObject
{
	Q_OBJECT

public:
	static constexpr bool DefaultUseSingleInstance{true};
	static constexpr bool DefaultPauseAnimationsOnTimelineClick{true};
	static constexpr bool DefaultTransparentScreenshots{false};
	static constexpr bool DefaultAllowTabCloseWithMiddleClick{false};
	static constexpr bool DefaultOneAssetAtATime{false};

	static constexpr int DefaultTickRate{60};
	static constexpr int MinimumTickRate{1};
	static constexpr int MaximumTickRate{1000};

	static constexpr int DefaultMouseSensitivity{5};
	static constexpr int MinimumMouseSensitivity{1};
	static constexpr int MaximumMouseSensitivity{20};

	static constexpr int DefaultMouseWheelSpeed{5};
	static constexpr int MinimumMouseWheelSpeed{1};
	static constexpr int MaximumMouseWheelSpeed{2048};

	static constexpr bool DefaultEnableAudioPlayback{true};
	static constexpr bool DefaultPlaySounds{true};
	static constexpr bool DefaultFramerateAffectsPitch{false};

	static constexpr bool DefaultEnableVSync{true};
	static constexpr bool DefaultPowerOf2Textures{false};

	static constexpr int DefaultMSAALevel{0};

	static constexpr graphics::TextureFilter DefaultMinFilter{graphics::TextureFilter::Linear};
	static constexpr graphics::TextureFilter DefaultMagFilter{graphics::TextureFilter::Linear};
	static constexpr graphics::MipmapFilter DefaultMipmapFilter{graphics::MipmapFilter::None};

	static constexpr int MinimumAspectRatio{1};
	static constexpr int MaximumAspectRatio{100};
	static constexpr glm::vec2 DefaultAspectRatio{16.f, 9.f};

	explicit ApplicationSettings(QSettings* settings, std::shared_ptr<spdlog::logger> fileSystemLogger);
	~ApplicationSettings() override;

	QSettings* GetSettings() const { return _settings; }

	RecentFilesSettings* GetRecentFiles() const { return _recentFiles.get(); }

	ColorSettings* GetColorSettings() const { return _colorSettings.get(); }

	GameConfigurationsSettings* GetGameConfigurations() const { return _gameConfigurations.get(); }

	ExternalProgramSettings* GetExternalPrograms() const { return _externalPrograms.get(); }

	/**
	*	@brief Registers a settings object to participate in saving and loading.
	*	Ownership is not transferred to this application settings object.
	*/
	void RegisterSettings(BaseSettings* settings);

	void LoadSettings();
	void SaveSettings();

	static bool ShouldUseSingleInstance(const QSettings& settings);

	bool ShouldUseSingleInstance() const;
	void SetUseSingleInstance(bool value);

	bool ShouldAllowTabCloseWithMiddleClick() const;
	void SetAllowTabCloseWithMiddleClick(bool value);

	int GetTickRate() const { return _tickRate; }

	void SetTickRate(int value)
	{
		if (_tickRate != value)
		{
			_tickRate = value;
			emit TickRateChanged(_tickRate);
		}
	}

	bool ShouldInvertMouseX() const { return _invertMouseX; }

	void SetInvertMouseX(bool value)
	{
		_invertMouseX = value;
	}

	bool ShouldInvertMouseY() const { return _invertMouseY; }

	void SetInvertMouseY(bool value)
	{
		_invertMouseY = value;
	}

	int GetMouseSensitivity() const { return _mouseSensitivity; }

	void SetMouseSensitivity(int value)
	{
		_mouseSensitivity = value;
	}

	float GetNormalizedMouseSensitivity() const
	{
		return static_cast<float>(_mouseSensitivity) / DefaultMouseSensitivity;
	}

	int GetMouseWheelSpeed() const { return _mouseWheelSpeed; }

	void SetMouseWheelSpeed(int value)
	{
		_mouseWheelSpeed = value;
	}

	bool ShouldEnableAudioPlayback() const { return _enableAudioPlayback; }

	void SetEnableAudioPlayback(bool value)
	{
		_enableAudioPlayback = value;
	}

	bool PlaySounds = DefaultPlaySounds;
	bool FramerateAffectsPitch = DefaultFramerateAffectsPitch;

	static bool ShouldEnableVSync(const QSettings& settings);

	bool ShouldEnableVSync() const;
	void SetEnableVSync(bool value);

	bool ShouldResizeTexturesToPowerOf2() const { return _powerOf2Textures; }

	void SetResizeTexturesToPowerOf2(bool value)
	{
		if (_powerOf2Textures != value)
		{
			_powerOf2Textures = value;
			emit ResizeTexturesToPowerOf2Changed(value);
		}
	}

	graphics::TextureFilter GetMinFilter() const { return _minFilter; }

	graphics::TextureFilter GetMagFilter() const { return _magFilter; }

	graphics::MipmapFilter GetMipmapFilter() const { return _mipmapFilter; }

	void SetTextureFilters(
		graphics::TextureFilter minFilter, graphics::TextureFilter magFilter, graphics::MipmapFilter mipmapFilter)
	{
		if (_minFilter == minFilter && _magFilter == magFilter && _mipmapFilter == mipmapFilter)
		{
			return;
		}

		_minFilter = minFilter;
		_magFilter = magFilter;
		_mipmapFilter = mipmapFilter;

		emit TextureFiltersChanged(_minFilter, _magFilter, _mipmapFilter);
	}

	int GetMSAALevel() const { return _msaaLevel; }

	void SetMSAALevel(int msaaLevel)
	{
		if (_msaaLevel != msaaLevel)
		{
			_msaaLevel = msaaLevel;
			emit MSAALevelChanged(_msaaLevel);
		}
	}

	const glm::vec2 GetAspectRatio() const { return _aspectRatio; }

	void SetAspectRatio(const glm::vec2& value)
	{
		_aspectRatio = value;
	}

	QString GetSavedPath(const QString& pathName);
	void SetSavedPath(const QString& pathName, const QString& path);

	QString GetStylePath() const;
	void SetStylePath(const QString& stylePath);

	QString GetFileListFilter() const;
	void SetFileListFilter(const QString& filterName);

	QString GetFileListRootDirectory() const;
	void SetFileListRootDirectory(const QString& directory);

signals:
	void SettingsLoaded();
	void SettingsSaved();

	void TickRateChanged(int value);

	void ResizeTexturesToPowerOf2Changed(bool value);

	void TextureFiltersChanged(
		graphics::TextureFilter minFilter, graphics::TextureFilter magFilter, graphics::MipmapFilter mipmapFilter);

	void MSAALevelChanged(int msaaLevel);

	void StylePathChanged(const QString& stylePath);

private slots:
	void OnSettingsDestroyed(QObject* object);

public:
	bool PauseAnimationsOnTimelineClick{DefaultPauseAnimationsOnTimelineClick};
	bool OneAssetAtATime{DefaultOneAssetAtATime};
	bool TransparentScreenshots{DefaultTransparentScreenshots};

private:
	QSettings* const _settings;

	QVector<BaseSettings*> _settingsObjects;

	const std::unique_ptr<RecentFilesSettings> _recentFiles;
	const std::unique_ptr<ColorSettings> _colorSettings;
	const std::unique_ptr<GameConfigurationsSettings> _gameConfigurations;
	const std::unique_ptr<ExternalProgramSettings> _externalPrograms;

	int _tickRate{DefaultTickRate};

	bool _invertMouseX{false};
	bool _invertMouseY{false};

	int _mouseSensitivity{DefaultMouseSensitivity};

	int _mouseWheelSpeed{DefaultMouseWheelSpeed};

	bool _enableAudioPlayback{DefaultEnableAudioPlayback};

	bool _powerOf2Textures{DefaultPowerOf2Textures};

	graphics::TextureFilter _minFilter{DefaultMinFilter};
	graphics::TextureFilter _magFilter{DefaultMagFilter};
	graphics::MipmapFilter _mipmapFilter{DefaultMipmapFilter};

	int _msaaLevel{DefaultMSAALevel};

	glm::vec2 _aspectRatio{DefaultAspectRatio};
};
