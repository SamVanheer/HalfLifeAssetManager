#pragma once

#include <QObject>
#include <QSettings>

#include "graphics/TextureLoader.hpp"

enum class GuidelinesAspectRatio
{
	FourThree,
	SixteenNine,
	SixteenTen
};

class GeneralSettings final : public QObject
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

	static constexpr bool DefaultPowerOf2Textures{false};

	static constexpr int DefaultMSAALevel{0};

	static constexpr graphics::TextureFilter DefaultMinFilter{graphics::TextureFilter::Linear};
	static constexpr graphics::TextureFilter DefaultMagFilter{graphics::TextureFilter::Linear};
	static constexpr graphics::MipmapFilter DefaultMipmapFilter{graphics::MipmapFilter::None};

	static constexpr GuidelinesAspectRatio DefaultGuidelinesAspectRatio{GuidelinesAspectRatio::SixteenNine};

	GeneralSettings() = default;

	static bool ShouldUseSingleInstance(QSettings& settings)
	{
		return settings.value("Startup/UseSingleInstance", DefaultUseSingleInstance).toBool();
	}

	void LoadSettings(QSettings& settings)
	{
		_useSingleInstance = ShouldUseSingleInstance(settings);

		settings.beginGroup("General");
		PauseAnimationsOnTimelineClick = settings.value("PauseAnimationsOnTimelineClick", DefaultPauseAnimationsOnTimelineClick).toBool();
		OneAssetAtATime = settings.value("OneAssetAtATime", DefaultOneAssetAtATime).toBool();
		PromptExternalProgramLaunch = settings.value(
			"PromptExternalProgramLaunch", DefaultPromptExternalProgramLaunch).toBool();
		_tickRate = std::clamp(settings.value("TickRate", DefaultTickRate).toInt(), MinimumTickRate, MaximumTickRate);

		GuidelinesAspectRatio = static_cast<::GuidelinesAspectRatio>(settings.value(
			"GuidelinesAspectRatio", static_cast<int>(GuidelinesAspectRatio::SixteenNine)).toInt());

		GuidelinesAspectRatio = std::clamp(
			GuidelinesAspectRatio, ::GuidelinesAspectRatio::FourThree, ::GuidelinesAspectRatio::SixteenTen);
		settings.endGroup();

		settings.beginGroup("Mouse");
		_invertMouseX = settings.value("InvertMouseX", false).toBool();
		_invertMouseY = settings.value("InvertMouseY", false).toBool();
		_mouseSensitivity = std::clamp(settings.value("MouseSensitivity", DefaultMouseSensitivity).toInt(), MinimumMouseSensitivity, MaximumMouseSensitivity);
		_mouseWheelSpeed = std::clamp(settings.value("MouseWheelSpeed", DefaultMouseWheelSpeed).toInt(), MinimumMouseWheelSpeed, MaximumMouseWheelSpeed);
		settings.endGroup();

		settings.beginGroup("Audio");
		_enableAudioPlayback = settings.value("EnableAudioPlayback", DefaultEnableAudioPlayback).toBool();
		PlaySounds = settings.value("PlaySounds", DefaultPlaySounds).toBool();
		FramerateAffectsPitch = settings.value("FramerateAffectsPitch", DefaultFramerateAffectsPitch).toBool();
		settings.endGroup();

		settings.beginGroup("Graphics");

		_powerOf2Textures = settings.value("PowerOf2Textures", DefaultPowerOf2Textures).toBool();

		settings.beginGroup("TextureFilters");
		_minFilter = static_cast<graphics::TextureFilter>(std::clamp(
			settings.value("Min", static_cast<int>(DefaultMinFilter)).toInt(),
			static_cast<int>(graphics::TextureFilter::First),
			static_cast<int>(graphics::TextureFilter::Last)));

		_magFilter = static_cast<graphics::TextureFilter>(std::clamp(
			settings.value("Mag", static_cast<int>(DefaultMagFilter)).toInt(),
			static_cast<int>(graphics::TextureFilter::First),
			static_cast<int>(graphics::TextureFilter::Last)));

		_mipmapFilter = static_cast<graphics::MipmapFilter>(std::clamp(
			settings.value("Mipmap", static_cast<int>(DefaultMipmapFilter)).toInt(),
			static_cast<int>(graphics::MipmapFilter::First),
			static_cast<int>(graphics::MipmapFilter::Last)));
		settings.endGroup();

		_msaaLevel = settings.value("MSAALevel", DefaultMSAALevel).toInt();
		TransparentScreenshots = settings.value("TransparentScreenshots", DefaultTransparentScreenshots).toBool();
		settings.endGroup();
	}

	void SaveSettings(QSettings& settings)
	{
		settings.beginGroup("Startup");
		settings.setValue("UseSingleInstance", _useSingleInstance);
		settings.endGroup();

		settings.beginGroup("General");
		settings.setValue("PauseAnimationsOnTimelineClick", PauseAnimationsOnTimelineClick);
		settings.setValue("OneAssetAtATime", OneAssetAtATime);
		settings.setValue("PromptExternalProgramLaunch", PromptExternalProgramLaunch);
		settings.setValue("TickRate", _tickRate);
		settings.setValue("GuidelinesAspectRatio", static_cast<int>(GuidelinesAspectRatio));
		settings.endGroup();

		settings.beginGroup("Mouse");
		settings.setValue("InvertMouseX", _invertMouseX);
		settings.setValue("InvertMouseY", _invertMouseY);
		settings.setValue("MouseSensitivity", _mouseSensitivity);
		settings.setValue("MouseWheelSpeed", _mouseWheelSpeed);
		settings.endGroup();

		settings.beginGroup("Audio");
		settings.setValue("EnableAudioPlayback", _enableAudioPlayback);
		settings.setValue("PlaySounds", PlaySounds);
		settings.setValue("FramerateAffectsPitch", FramerateAffectsPitch);
		settings.endGroup();

		settings.beginGroup("Graphics");
		settings.setValue("PowerOf2Textures", _powerOf2Textures);

		settings.beginGroup("TextureFilters");
		settings.setValue("Min", static_cast<int>(_minFilter));
		settings.setValue("Mag", static_cast<int>(_magFilter));
		settings.setValue("Mipmap", static_cast<int>(_mipmapFilter));
		settings.endGroup();

		settings.setValue("MSAALevel", _msaaLevel);
		settings.setValue("TransparentScreenshots", DefaultTransparentScreenshots);
		settings.endGroup();
	}

	bool ShouldUseSingleInstance() const { return _useSingleInstance; }

	void SetUseSingleInstance(bool value)
	{
		_useSingleInstance = value;
	}

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

	void SetTextureFilters(graphics::TextureFilter minFilter, graphics::TextureFilter magFilter, graphics::MipmapFilter mipmapFilter)
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

signals:
	void TickRateChanged(int value);

	void ResizeTexturesToPowerOf2Changed(bool value);

	void TextureFiltersChanged(
		graphics::TextureFilter minFilter, graphics::TextureFilter magFilter, graphics::MipmapFilter mipmapFilter);

	void MSAALevelChanged(int msaaLevel);

public:
	bool PauseAnimationsOnTimelineClick{DefaultPauseAnimationsOnTimelineClick};
	bool OneAssetAtATime{DefaultOneAssetAtATime};
	bool PromptExternalProgramLaunch{DefaultPromptExternalProgramLaunch};
	bool TransparentScreenshots{DefaultTransparentScreenshots};

	GuidelinesAspectRatio GuidelinesAspectRatio{DefaultGuidelinesAspectRatio};

private:
	bool _useSingleInstance{DefaultUseSingleInstance};

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
