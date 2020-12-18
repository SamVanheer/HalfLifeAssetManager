#pragma once

#include <QObject>
#include <QSettings>

namespace ui::settings
{
class GeneralSettings final : public QObject
{
	Q_OBJECT

public:
	static constexpr float DefaultMaxFPS{60.f};

	GeneralSettings() = default;

	void LoadSettings(QSettings& settings)
	{
		settings.beginGroup("startup");
		_useSingleInstance = settings.value("UseSingleInstance", false).toBool();
		_maxFPS = settings.value("MaxFPS", DefaultMaxFPS).toFloat();
		_invertMouseX = settings.value("InvertMouseX", false).toBool();
		_invertMouseY = settings.value("InvertMouseY", false).toBool();
		settings.endGroup();
	}

	void SaveSettings(QSettings& settings)
	{
		settings.beginGroup("startup");
		settings.setValue("UseSingleInstance", _useSingleInstance);
		settings.setValue("MaxFPS", _maxFPS);
		settings.setValue("InvertMouseX", _invertMouseX);
		settings.setValue("InvertMouseY", _invertMouseY);
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

signals:
	void MaxFPSChanged(float value);

private:
	bool _useSingleInstance{false};

	float _maxFPS{DefaultMaxFPS};

	bool _invertMouseX{false};
	bool _invertMouseY{false};
};
}
