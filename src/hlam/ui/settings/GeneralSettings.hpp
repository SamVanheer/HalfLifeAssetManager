#pragma once

#include <QObject>
#include <QSettings>

namespace ui::settings
{
class GeneralSettings final : public QObject
{
	Q_OBJECT

public:
	static constexpr bool DefaultUseSingleInstance{true};
	static constexpr bool DefaultPauseAnimationsOnTimelineClick{true};
	static constexpr bool DefaultAllowTabCloseWithMiddleClick{false};

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

	GeneralSettings() = default;

	static bool ShouldUseSingleInstance(QSettings& settings)
	{
		settings.beginGroup("startup");
		const bool useSingleInstance = settings.value("UseSingleInstance", DefaultUseSingleInstance).toBool();
		settings.endGroup();

		return useSingleInstance;
	}

	void LoadSettings(QSettings& settings)
	{
		_useSingleInstance = ShouldUseSingleInstance(settings);

		settings.beginGroup("general");
		PauseAnimationsOnTimelineClick = settings.value("PauseAnimationsOnTimelineClick", DefaultPauseAnimationsOnTimelineClick).toBool();
		_tickRate = std::clamp(settings.value("TickRate", DefaultTickRate).toInt(), MinimumTickRate, MaximumTickRate);
		settings.endGroup();

		settings.beginGroup("mouse");
		_invertMouseX = settings.value("InvertMouseX", false).toBool();
		_invertMouseY = settings.value("InvertMouseY", false).toBool();
		_mouseSensitivity = std::clamp(settings.value("MouseSensitivity", DefaultMouseSensitivity).toInt(), MinimumMouseSensitivity, MaximumMouseSensitivity);
		_mouseWheelSpeed = std::clamp(settings.value("MouseWheelSpeed", DefaultMouseWheelSpeed).toInt(), MinimumMouseWheelSpeed, MaximumMouseWheelSpeed);
		settings.endGroup();

		settings.beginGroup("audio");
		_enableAudioPlayback = settings.value("EnableAudioPlayback", DefaultEnableAudioPlayback).toBool();
		PlaySounds = settings.value("PlaySounds", DefaultPlaySounds).toBool();
		FramerateAffectsPitch = settings.value("FramerateAffectsPitch", DefaultFramerateAffectsPitch).toBool();
		settings.endGroup();
	}

	void SaveSettings(QSettings& settings)
	{
		settings.beginGroup("startup");
		settings.setValue("UseSingleInstance", _useSingleInstance);
		settings.endGroup();

		settings.beginGroup("general");
		settings.setValue("PauseAnimationsOnTimelineClick", PauseAnimationsOnTimelineClick);
		settings.setValue("TickRate", _tickRate);
		settings.endGroup();

		settings.beginGroup("mouse");
		settings.setValue("InvertMouseX", _invertMouseX);
		settings.setValue("InvertMouseY", _invertMouseY);
		settings.setValue("MouseSensitivity", _mouseSensitivity);
		settings.setValue("MouseWheelSpeed", _mouseWheelSpeed);
		settings.endGroup();

		settings.beginGroup("audio");
		settings.setValue("EnableAudioPlayback", _enableAudioPlayback);
		settings.setValue("PlaySounds", PlaySounds);
		settings.setValue("FramerateAffectsPitch", FramerateAffectsPitch);
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

signals:
	void TickRateChanged(int value);

public:
	bool PauseAnimationsOnTimelineClick{DefaultPauseAnimationsOnTimelineClick};

private:
	bool _useSingleInstance{DefaultUseSingleInstance};

	int _tickRate{DefaultTickRate};

	bool _invertMouseX{false};
	bool _invertMouseY{false};

	int _mouseSensitivity{DefaultMouseSensitivity};

	int _mouseWheelSpeed{DefaultMouseWheelSpeed};

	bool _enableAudioPlayback{DefaultEnableAudioPlayback};
};
}
