#pragma once

#include <algorithm>

#include <QObject>
#include <QSettings>
#include <QString>

class StudioModelSettings final : public QObject
{
	Q_OBJECT

public:
	static constexpr bool DefaultAutodetectViewmodels{true};
	static constexpr bool DefaultActivateTextureViewWhenTexturesPanelOpened{true};

	static constexpr int MinimumFloorLength = 0;
	static constexpr int MaximumFloorLength = 2048;
	static constexpr int DefaultFloorLength = 100;

	StudioModelSettings(QSettings* settings)
		: _settings(settings)
	{
	}

	~StudioModelSettings() = default;

	void LoadSettings()
	{
		_settings->beginGroup("Assets/StudioModel");
		_autodetectViewModels = _settings->value("AutodetectViewmodels", DefaultAutodetectViewmodels).toBool();
		_activateTextureViewWhenTexturesPanelOpened = _settings->value(
			"ActivateTextureViewWhenTexturesPanelOpened", DefaultActivateTextureViewWhenTexturesPanelOpened).toBool();
		_floorLength = std::clamp(_settings->value("FloorLength", DefaultFloorLength).toInt(), MinimumFloorLength, MaximumFloorLength);
		_settings->endGroup();
	}

	void SaveSettings()
	{
		_settings->beginGroup("Assets/StudioModel");
		_settings->setValue("AutodetectViewmodels", _autodetectViewModels);
		_settings->setValue("ActivateTextureViewWhenTexturesPanelOpened", _activateTextureViewWhenTexturesPanelOpened);
		_settings->setValue("FloorLength", _floorLength);
		_settings->endGroup();
	}

	bool ShouldAutodetectViewmodels() const { return _autodetectViewModels; }

	void SetAutodetectViewmodels(bool value)
	{
		if (_autodetectViewModels != value)
		{
			_autodetectViewModels = value;
		}
	}

	bool ShouldActivateTextureViewWhenTexturesPanelOpened() const { return _activateTextureViewWhenTexturesPanelOpened; }

	void SetActivateTextureViewWhenTexturesPanelOpened(bool value)
	{
		_activateTextureViewWhenTexturesPanelOpened = value;
	}

	int GetFloorLength() const { return _floorLength; }

	void SetFloorLength(int value)
	{
		_floorLength = value;
	}

private:
	QSettings* const _settings;

	bool _autodetectViewModels{DefaultAutodetectViewmodels};
	bool _activateTextureViewWhenTexturesPanelOpened{DefaultActivateTextureViewWhenTexturesPanelOpened};

	int _floorLength = DefaultFloorLength;
};
