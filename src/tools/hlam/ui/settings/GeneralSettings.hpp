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
	static constexpr float DefaultMaxFPS{60.f};
	static constexpr int DefaultMouseSensitivity{5};
	static constexpr int MinimumMouseSensitivity{1};
	static constexpr int MaximumMouseSensitivity{20};
	static constexpr bool DefaultEnableAudioPlayback{true};

	GeneralSettings() = default;

	void LoadSettings(QSettings& settings)
	{
		settings.beginGroup("startup");
		_useSingleInstance = settings.value("UseSingleInstance", DefaultUseSingleInstance).toBool();
		settings.endGroup();

		settings.beginGroup("general");
		_maxFPS = settings.value("MaxFPS", DefaultMaxFPS).toFloat();
		settings.endGroup();

		settings.beginGroup("mouse");
		_invertMouseX = settings.value("InvertMouseX", false).toBool();
		_invertMouseY = settings.value("InvertMouseY", false).toBool();
		_mouseSensitivity = std::clamp(settings.value("MouseSensitivity", DefaultMouseSensitivity).toInt(), MinimumMouseSensitivity, MaximumMouseSensitivity);
		settings.endGroup();

		settings.beginGroup("audio");
		_enableAudioPlayback = settings.value("EnableAudioPlayback", DefaultEnableAudioPlayback).toBool();
		settings.endGroup();
	}

	void SaveSettings(QSettings& settings)
	{
		settings.beginGroup("startup");
		settings.setValue("UseSingleInstance", _useSingleInstance);
		settings.endGroup();

		settings.beginGroup("general");
		settings.setValue("MaxFPS", _maxFPS);
		settings.endGroup();

		settings.beginGroup("mouse");
		settings.setValue("InvertMouseX", _invertMouseX);
		settings.setValue("InvertMouseY", _invertMouseY);
		settings.setValue("MouseSensitivity", _mouseSensitivity);
		settings.endGroup();

		settings.beginGroup("audio");
		settings.setValue("EnableAudioPlayback", _enableAudioPlayback);
		settings.endGroup();
	}

	bool ShouldUseSingleInstance() const { return _useSingleInstance; }

	void SetUseSingleInstance(bool value)
	{
		_useSingleInstance = value;
	}

	float GetMaxFPS() const { return _maxFPS; }

	void SetMaxFPS(float value)
	{
		if (_maxFPS != value)
		{
			_maxFPS = value;
			emit MaxFPSChanged(_maxFPS);
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

	bool ShouldEnableAudioPlayback() const { return _enableAudioPlayback; }

	void SetEnableAudioPlayback(bool value)
	{
		_enableAudioPlayback = value;
	}

signals:
	void MaxFPSChanged(float value);

private:
	bool _useSingleInstance{DefaultUseSingleInstance};

	float _maxFPS{DefaultMaxFPS};

	bool _invertMouseX{false};
	bool _invertMouseY{false};

	int _mouseSensitivity{DefaultMouseSensitivity};

	bool _enableAudioPlayback{DefaultEnableAudioPlayback};
};
}
