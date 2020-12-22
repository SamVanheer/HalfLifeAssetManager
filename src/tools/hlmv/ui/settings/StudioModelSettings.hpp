#pragma once

#include <algorithm>

#include <QObject>
#include <QSettings>
#include <QString>

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
		_studiomdlCompilerFileName = settings.value("CompilerFileName").toString();
		_studiomdlDecompilerFileName = settings.value("DecompilerFileName").toString();
		settings.endGroup();
	}

	void SaveSettings(QSettings& settings)
	{
		settings.beginGroup("assets/studiomodel");
		settings.setValue("AutodetectViewmodels", _autodetectViewModels);
		settings.setValue("FloorLength", _floorLength);
		settings.setValue("CompilerFileName", _studiomdlCompilerFileName);
		settings.setValue("DecompilerFileName", _studiomdlDecompilerFileName);
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

	int GetFloorLength() const { return _floorLength; }

	void SetFloorLength(int value)
	{
		if (_floorLength != value)
		{
			_floorLength = value;

			emit FloorLengthChanged(_floorLength);
		}
	}

	QString GetStudiomdlCompilerFileName() const { return _studiomdlCompilerFileName; }

	void SetStudiomdlCompilerFileName(const QString& fileName)
	{
		_studiomdlCompilerFileName = fileName;
	}

	QString GetStudiomdlDecompilerFileName() const { return _studiomdlDecompilerFileName; }

	void SetStudiomdlDecompilerFileName(const QString& fileName)
	{
		_studiomdlDecompilerFileName = fileName;
	}

signals:
	void FloorLengthChanged(int length);

private:
	bool _autodetectViewModels{DefaultAutodetectViewmodels};
	int _floorLength = DefaultFloorLength;

	QString _studiomdlCompilerFileName;
	QString _studiomdlDecompilerFileName;
};
}
