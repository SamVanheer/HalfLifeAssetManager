#pragma once

#include <QSettings>

namespace ui::settings
{
class GeneralSettings final
{
public:
	GeneralSettings() = default;

	void LoadSettings(QSettings& settings)
	{
		settings.beginGroup("startup");
		_useSingleInstance = settings.value("UseSingleInstance", false).toBool();
		settings.endGroup();
	}

	void SaveSettings(QSettings& settings)
	{
		settings.beginGroup("startup");
		settings.setValue("UseSingleInstance", _useSingleInstance);
		settings.endGroup();
	}

	bool ShouldUseSingleInstance() const { return _useSingleInstance; }

	void SetUseSingleInstance(bool value)
	{
		_useSingleInstance = value;
	}

private:
	bool _useSingleInstance{false};
};
}
