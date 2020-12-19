#pragma once

#include <algorithm>

#include <QObject>
#include <QSettings>

namespace ui::settings
{
class StudioModelSettings final : public QObject
{
	Q_OBJECT

public:
	static constexpr bool DefaultAutodetectViewmodels{true};

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
		settings.beginGroup("assets/studiomodel");
		_autodetectViewModels = settings.value("AutodetectViewmodels", DefaultAutodetectViewmodels).toBool();
		_floorLength = std::clamp(settings.value("FloorLength", DefaultFloorLength).toInt(), MinimumFloorLength, MaximumFloorLength);
		settings.endGroup();
	}

	void SaveSettings(QSettings& settings)
	{
		settings.beginGroup("assets/studiomodel");
		settings.setValue("AutodetectViewmodels", _autodetectViewModels);
		settings.setValue("FloorLength", _floorLength);
		settings.endGroup();
	}

	bool ShouldAutodetectViewmodels() const { return _autodetectViewModels; }

	int GetFloorLength() const { return _floorLength; }

signals:
	void FloorLengthChanged(int length);

public slots:
	void SetAutodetectViewmodels(bool value)
	{
		if (_autodetectViewModels != value)
		{
			_autodetectViewModels = value;
		}
	}

	void SetFloorLength(int value)
	{
		if (_floorLength != value)
		{
			_floorLength = value;

			emit FloorLengthChanged(_floorLength);
		}
	}

private:
	bool _autodetectViewModels{DefaultAutodetectViewmodels};
	int _floorLength = DefaultFloorLength;
};
}
