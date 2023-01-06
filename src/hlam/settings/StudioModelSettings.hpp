#pragma once

#include <QObject>
#include <QString>

class QSettings;

class StudioModelSettings final : public QObject
{
public:
	static constexpr bool DefaultAutodetectViewmodels{true};
	static constexpr bool DefaultActivateTextureViewWhenTexturesPanelOpened{true};

	static constexpr int MinimumFloorLength = 0;
	static constexpr int MaximumFloorLength = 2048;
	static constexpr int DefaultFloorLength = 100;

	explicit StudioModelSettings(QSettings* settings)
		: _settings(settings)
	{
	}

	~StudioModelSettings() = default;

	void LoadSettings();
	void SaveSettings();

	bool ShouldAutodetectViewmodels() const { return _autodetectViewModels; }

	void SetAutodetectViewmodels(bool value)
	{
		if (_autodetectViewModels != value)
		{
			_autodetectViewModels = value;
		}
	}

	bool ShouldActivateTextureViewWhenTexturesPanelOpened() const
	{
		return _activateTextureViewWhenTexturesPanelOpened;
	}

	void SetActivateTextureViewWhenTexturesPanelOpened(bool value)
	{
		_activateTextureViewWhenTexturesPanelOpened = value;
	}

	int GetFloorLength() const { return _floorLength; }

	void SetFloorLength(int value)
	{
		_floorLength = value;
	}

	float GetCameraFOV(const QString& name, float defaultValue) const;
	void SetCameraFOV(const QString& name, float value);

private:
	QSettings* const _settings;

	bool _autodetectViewModels{DefaultAutodetectViewmodels};
	bool _activateTextureViewWhenTexturesPanelOpened{DefaultActivateTextureViewWhenTexturesPanelOpened};

	int _floorLength = DefaultFloorLength;
};
