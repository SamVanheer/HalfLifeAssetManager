#include <algorithm>

#include <QSettings>

#include "plugins/halflife/studiomodel/settings/StudioModelSettings.hpp"

void StudioModelSettings::LoadSettings()
{
	_settings->beginGroup("Assets/StudioModel");
	_autodetectViewModels = _settings->value("AutodetectViewmodels", DefaultAutodetectViewmodels).toBool();
	_activateTextureViewWhenTexturesPanelOpened = _settings->value(
		"ActivateTextureViewWhenTexturesPanelOpened", DefaultActivateTextureViewWhenTexturesPanelOpened).toBool();
	_groundLength = std::clamp(_settings->value(
		"GroundLength", DefaultGroundLength).toInt(), MinimumGroundLength, MaximumGroundLength);

	_soundEventIds.clear();
	const int soundEventIdsCount = _settings->beginReadArray("SoundEventIds");
	for (int i = 0; i < soundEventIdsCount; ++i)
	{
		_settings->setArrayIndex(i);

		// 0 is not allowed since it's the default event id.
		if (const int id = _settings->value("Id", 0).toInt(); id > 0)
		{
			_soundEventIds.insert(id);
		}
	}
	_settings->endArray();

	_settings->endGroup();
}

void StudioModelSettings::SaveSettings()
{
	_settings->beginGroup("Assets/StudioModel");
	_settings->setValue("AutodetectViewmodels", _autodetectViewModels);
	_settings->setValue("ActivateTextureViewWhenTexturesPanelOpened", _activateTextureViewWhenTexturesPanelOpened);
	_settings->setValue("GroundLength", _groundLength);

	_settings->beginWriteArray("SoundEventIds", _soundEventIds.size());
	for (int i = 0; auto id : _soundEventIds)
	{
		_settings->setArrayIndex(i++);
		_settings->setValue("Id", id);
	}
	_settings->endArray();

	_settings->endGroup();
}

bool StudioModelSettings::IsControlsBarVisible() const
{
	return _settings->value("Assets/StudioModel/UI/IsControlsBarVisible", true).toBool();
}

void StudioModelSettings::SetControlsBarVisible(bool value)
{
	_settings->setValue("Assets/StudioModel/UI/IsControlsBarVisible", value);
}

bool StudioModelSettings::IsTimelineVisible() const
{
	return _settings->value("Assets/StudioModel/UI/IsTimelineVisible", true).toBool();
}

void StudioModelSettings::SetTimelineVisible(bool value)
{
	_settings->setValue("Assets/StudioModel/UI/IsTimelineVisible", value);
}

bool StudioModelSettings::AreEditControlsVisible() const
{
	return _settings->value("Assets/StudioModel/UI/AreEditControlsVisible", true).toBool();
}

void StudioModelSettings::SetEditControlsVisible(bool value)
{
	_settings->setValue("Assets/StudioModel/UI/AreEditControlsVisible", value);
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
