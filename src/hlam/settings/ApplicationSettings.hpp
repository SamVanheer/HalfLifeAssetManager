#pragma once

#include <memory>

#include <QObject>
#include <QString>

#include "graphics/TextureLoader.hpp"

class ColorSettings;
class GameConfigurationsSettings;
class QSettings;
class RecentFilesSettings;

enum class GuidelinesAspectRatio
{
	FourThree,
	SixteenNine,
	SixteenTen
};

class ApplicationSettings final : public QObject
{
	Q_OBJECT

public:
	static constexpr bool DefaultUseSingleInstance{true};
	static constexpr bool DefaultPauseAnimationsOnTimelineClick{true};
	static constexpr bool DefaultTransparentScreenshots{false};
	static constexpr bool DefaultAllowTabCloseWithMiddleClick{false};
	static constexpr bool DefaultOneAssetAtATime{false};
	static constexpr bool DefaultPromptExternalProgramLaunch{true};

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

	static constexpr GuidelinesAspectRatio DefaultGuidelinesAspectRatio{GuidelinesAspectRatio::SixteenNine};

	explicit ApplicationSettings(QSettings* settings);
	~ApplicationSettings() override;

	QSettings* GetSettings() const { return _settings; }

	RecentFilesSettings* GetRecentFiles() const { return _recentFiles.get(); }

	ColorSettings* GetColorSettings() const { return _colorSettings.get(); }

	GameConfigurationsSettings* GetGameConfigurations() const { return _gameConfigurations.get(); }

	void LoadSettings();
	void SaveSettings();

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

	QString GetStudiomdlCompilerFileName() const;
	void SetStudiomdlCompilerFileName(const QString& fileName);

	QString GetStudiomdlDecompilerFileName() const;
	void SetStudiomdlDecompilerFileName(const QString& fileName);

	QString GetXashModelViewerFileName() const;
	void SetXashModelViewerFileName(const QString& fileName);

	QString GetQuake1ModelViewerFileName() const;
	void SetQuake1ModelViewerFileName(const QString& fileName);

	QString GetSource1ModelViewerFileName() const;
	void SetSource1ModelViewerFileName(const QString& fileName);

	QString GetSavedPath(const QString& pathName);
	void SetSavedPath(const QString& pathName, const QString& path);

	QString GetStylePath() const;
	void SetStylePath(const QString& stylePath);

signals:
	void SettingsSaved();

	void TickRateChanged(int value);

	void ResizeTexturesToPowerOf2Changed(bool value);

	void TextureFiltersChanged(
		graphics::TextureFilter minFilter, graphics::TextureFilter magFilter, graphics::MipmapFilter mipmapFilter);

	void MSAALevelChanged(int msaaLevel);

	void StylePathChanged(const QString& stylePath);

public:
	bool PauseAnimationsOnTimelineClick{DefaultPauseAnimationsOnTimelineClick};
	bool OneAssetAtATime{DefaultOneAssetAtATime};
	bool PromptExternalProgramLaunch{DefaultPromptExternalProgramLaunch};
	bool TransparentScreenshots{DefaultTransparentScreenshots};

	GuidelinesAspectRatio GuidelinesAspectRatio{DefaultGuidelinesAspectRatio};

private:
	QSettings* const _settings;

	const std::unique_ptr<RecentFilesSettings> _recentFiles;
	const std::unique_ptr<ColorSettings> _colorSettings;
	const std::unique_ptr<GameConfigurationsSettings> _gameConfigurations;

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
};
