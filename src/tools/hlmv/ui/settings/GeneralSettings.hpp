#pragma once

#include <QObject>
#include <QSettings>

namespace ui::settings
{
class GeneralSettings final : public QObject
{
public:
	GeneralSettings() = default;

	void LoadSettings(QSettings& settings)
	{
		settings.beginGroup("startup");
		_useSingleInstance = settings.value("UseSingleInstance", false).toBool();
		_invertMouseX = settings.value("InvertMouseX", false).toBool();
		_invertMouseY = settings.value("InvertMouseY", false).toBool();
		settings.endGroup();
	}

	void SaveSettings(QSettings& settings)
	{
		settings.beginGroup("startup");
		settings.setValue("UseSingleInstance", _useSingleInstance);
		settings.setValue("InvertMouseX", _invertMouseX);
		settings.setValue("InvertMouseY", _invertMouseY);
		settings.endGroup();
	}

	bool ShouldUseSingleInstance() const { return _useSingleInstance; }

	void SetUseSingleInstance(bool value)
	{
		_useSingleInstance = value;
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

private:
	bool _useSingleInstance{false};
	bool _invertMouseX{false};
	bool _invertMouseY{false};
};
}
