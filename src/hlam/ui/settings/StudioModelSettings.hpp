#pragma once

#include <algorithm>

#include <QObject>
#include <QSettings>
#include <QString>

#include <glm/vec3.hpp>

#include "graphics/GraphicsConstants.hpp"
#undef PlaySound

namespace ui::settings
{
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
		settings.beginGroup("assets/studiomodel");
		_autodetectViewModels = settings.value("AutodetectViewmodels", DefaultAutodetectViewmodels).toBool();
		_activateTextureViewWhenTexturesPanelOpened = settings.value(
			"ActivateTextureViewWhenTexturesPanelOpened", DefaultActivateTextureViewWhenTexturesPanelOpened).toBool();
		_floorLength = std::clamp(settings.value("FloorLength", DefaultFloorLength).toInt(), MinimumFloorLength, MaximumFloorLength);
		_studiomdlCompilerFileName = settings.value("CompilerFileName").toString();
		_studiomdlDecompilerFileName = settings.value("DecompilerFileName").toString();
		settings.endGroup();
	}

	void SaveSettings(QSettings& settings)
	{
		settings.beginGroup("assets/studiomodel");
		settings.setValue("AutodetectViewmodels", _autodetectViewModels);
		settings.setValue("ActivateTextureViewWhenTexturesPanelOpened", _activateTextureViewWhenTexturesPanelOpened);
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

	// TODO: not persisted right now.
	RenderMode CurrentRenderMode = RenderMode::TEXTURE_SHADED;

	bool ShowHitboxes = false;
	bool ShowBones = false;
	bool ShowAttachments = false;
	bool ShowEyePosition = false;
	bool ShowBBox = false;
	bool ShowCBox = false;
	bool EnableBackfaceCulling = true;
	bool ShowGround = false;
	bool MirrorOnGround = false;
	bool ShowBackground = false;
	bool ShowWireframeOverlay = false;
	bool DrawShadows = false;
	bool FixShadowZFighting = false;
	bool ShowAxes = false;
	bool ShowNormals = false;
	bool ShowCrosshair = false;
	bool ShowGuidelines = false;
	bool ShowPlayerHitbox = false;

	bool EnableFloorTextureTiling{false};
	int FloorTextureLength{16};
	glm::vec3 FloorOrigin{0};

	bool CameraIsFirstPerson{false};

	int DrawSingleBoneIndex = -1;
	int DrawSingleAttachmentIndex = -1;
	int DrawSingleHitboxIndex = -1;

	bool PlaySequence = true;

private:
	bool _autodetectViewModels{DefaultAutodetectViewmodels};
	bool _activateTextureViewWhenTexturesPanelOpened{DefaultActivateTextureViewWhenTexturesPanelOpened};

	int _floorLength = DefaultFloorLength;

	QString _studiomdlCompilerFileName;
	QString _studiomdlDecompilerFileName;
};
}
