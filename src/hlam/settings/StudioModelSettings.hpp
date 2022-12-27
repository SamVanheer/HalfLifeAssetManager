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

	StudioModelSettings(QObject* parent = nullptr)
		: QObject(parent)
	{
	}

	~StudioModelSettings() = default;

	void LoadSettings(QSettings& settings)
	{
		settings.beginGroup("Assets/StudioModel");
		_autodetectViewModels = settings.value("AutodetectViewmodels", DefaultAutodetectViewmodels).toBool();
		_activateTextureViewWhenTexturesPanelOpened = settings.value(
			"ActivateTextureViewWhenTexturesPanelOpened", DefaultActivateTextureViewWhenTexturesPanelOpened).toBool();
		_floorLength = std::clamp(settings.value("FloorLength", DefaultFloorLength).toInt(), MinimumFloorLength, MaximumFloorLength);
		settings.endGroup();
	}

	void SaveSettings(QSettings& settings)
	{
		settings.beginGroup("Assets/StudioModel");
		settings.setValue("AutodetectViewmodels", _autodetectViewModels);
		settings.setValue("ActivateTextureViewWhenTexturesPanelOpened", _activateTextureViewWhenTexturesPanelOpened);
		settings.setValue("FloorLength", _floorLength);
		settings.endGroup();
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
	bool _autodetectViewModels{DefaultAutodetectViewmodels};
	bool _activateTextureViewWhenTexturesPanelOpened{DefaultActivateTextureViewWhenTexturesPanelOpened};

	int _floorLength = DefaultFloorLength;
};
