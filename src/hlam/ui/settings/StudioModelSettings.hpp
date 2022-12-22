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
		_studiomdlCompilerFileName = settings.value("CompilerFileName").toString();
		_studiomdlDecompilerFileName = settings.value("DecompilerFileName").toString();
		XashModelViewerFileName = settings.value("XashModelViewerFileName").toString();
		settings.endGroup();
	}

	void SaveSettings(QSettings& settings)
	{
		settings.beginGroup("Assets/StudioModel");
		settings.setValue("AutodetectViewmodels", _autodetectViewModels);
		settings.setValue("ActivateTextureViewWhenTexturesPanelOpened", _activateTextureViewWhenTexturesPanelOpened);
		settings.setValue("FloorLength", _floorLength);
		settings.setValue("CompilerFileName", _studiomdlCompilerFileName);
		settings.setValue("DecompilerFileName", _studiomdlDecompilerFileName);
		settings.setValue("XashModelViewerFileName", XashModelViewerFileName);
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

	QString XashModelViewerFileName;

private:
	bool _autodetectViewModels{DefaultAutodetectViewmodels};
	bool _activateTextureViewWhenTexturesPanelOpened{DefaultActivateTextureViewWhenTexturesPanelOpened};

	int _floorLength = DefaultFloorLength;

	// TODO: remove getters and setters
	QString _studiomdlCompilerFileName;
	QString _studiomdlDecompilerFileName;
};
