#include <algorithm>

#include <QSettings>

#include "settings/StudioModelSettings.hpp"

void StudioModelSettings::LoadSettings()
{
	_settings->beginGroup("Assets/StudioModel");
	_autodetectViewModels = _settings->value("AutodetectViewmodels", DefaultAutodetectViewmodels).toBool();
	_activateTextureViewWhenTexturesPanelOpened = _settings->value(
		"ActivateTextureViewWhenTexturesPanelOpened", DefaultActivateTextureViewWhenTexturesPanelOpened).toBool();
	_floorLength = std::clamp(_settings->value(
		"FloorLength", DefaultFloorLength).toInt(), MinimumFloorLength, MaximumFloorLength);
	_settings->endGroup();
}

void StudioModelSettings::SaveSettings()
{
	_settings->beginGroup("Assets/StudioModel");
	_settings->setValue("AutodetectViewmodels", _autodetectViewModels);
	_settings->setValue("ActivateTextureViewWhenTexturesPanelOpened", _activateTextureViewWhenTexturesPanelOpened);
	_settings->setValue("FloorLength", _floorLength);
	_settings->endGroup();
}

float StudioModelSettings::GetCameraFOV(const QString& name, float defaultValue) const
{
	_settings->beginGroup("Assets/StudioModel/Cameras/FOV");
	const float result = std::clamp(_settings->value(name, defaultValue).toFloat(), 0.f, 360.f);
	_settings->endGroup();

	return result;
}

void StudioModelSettings::SetCameraFOV(const QString& name, float value)
{
	_settings->beginGroup("Assets/StudioModel/Cameras/FOV");
	_settings->setValue(name, value);
	_settings->endGroup();
}
