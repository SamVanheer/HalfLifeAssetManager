#pragma once

#include <algorithm>

#include <QObject>
#include <QSettings>
#include <QString>

#include "graphics/TextureLoader.hpp"

namespace ui::settings
{
class StudioModelSettings final : public QObject
{
	Q_OBJECT

public:
	static constexpr bool DefaultAutodetectViewmodels{true};
	static constexpr bool DefaultPowerOf2Textures{true};
	static constexpr bool DefaultActivateTextureViewWhenTexturesPanelOpened{true};

	static constexpr int MinimumFloorLength = 0;
	static constexpr int MaximumFloorLength = 2048;
	static constexpr int DefaultFloorLength = 100;

	static constexpr graphics::TextureFilter DefaultMinFilter{graphics::TextureFilter::Linear};
	static constexpr graphics::TextureFilter DefaultMagFilter{graphics::TextureFilter::Linear};
	static constexpr graphics::MipmapFilter DefaultMipmapFilter{graphics::MipmapFilter::None};

	StudioModelSettings(QObject* parent = nullptr)
		: QObject(parent)
	{
	}

	~StudioModelSettings() = default;

	void LoadSettings(QSettings& settings)
	{
		settings.beginGroup("assets/studiomodel");
		_autodetectViewModels = settings.value("AutodetectViewmodels", DefaultAutodetectViewmodels).toBool();
		_powerOf2Textures = settings.value("PowerOf2Textures", DefaultPowerOf2Textures).toBool();
		_activateTextureViewWhenTexturesPanelOpened = settings.value(
			"ActivateTextureViewWhenTexturesPanelOpened", DefaultActivateTextureViewWhenTexturesPanelOpened).toBool();
		_floorLength = std::clamp(settings.value("FloorLength", DefaultFloorLength).toInt(), MinimumFloorLength, MaximumFloorLength);
		_studiomdlCompilerFileName = settings.value("CompilerFileName").toString();
		_studiomdlDecompilerFileName = settings.value("DecompilerFileName").toString();

		settings.beginGroup("TextureFilters");
		_minFilter = static_cast<graphics::TextureFilter>(std::clamp(
			settings.value("Min", static_cast<int>(DefaultMinFilter)).toInt(),
			static_cast<int>(graphics::TextureFilter::First),
			static_cast<int>(graphics::TextureFilter::Last)));

		_magFilter = static_cast<graphics::TextureFilter>(std::clamp(
			settings.value("Mag", static_cast<int>(DefaultMagFilter)).toInt(),
			static_cast<int>(graphics::TextureFilter::First),
			static_cast<int>(graphics::TextureFilter::Last)));

		_mipmapFilter = static_cast<graphics::MipmapFilter>(std::clamp(
			settings.value("Mipmap", static_cast<int>(DefaultMipmapFilter)).toInt(),
			static_cast<int>(graphics::MipmapFilter::First),
			static_cast<int>(graphics::MipmapFilter::Last)));
		settings.endGroup();

		settings.endGroup();
	}

	void SaveSettings(QSettings& settings)
	{
		settings.beginGroup("assets/studiomodel");
		settings.setValue("AutodetectViewmodels", _autodetectViewModels);
		settings.setValue("PowerOf2Textures", _powerOf2Textures);
		settings.setValue("ActivateTextureViewWhenTexturesPanelOpened", _activateTextureViewWhenTexturesPanelOpened);
		settings.setValue("FloorLength", _floorLength);
		settings.setValue("CompilerFileName", _studiomdlCompilerFileName);
		settings.setValue("DecompilerFileName", _studiomdlDecompilerFileName);

		settings.beginGroup("TextureFilters");
		settings.setValue("Min", static_cast<int>(_minFilter));
		settings.setValue("Mag", static_cast<int>(_magFilter));
		settings.setValue("Mipmap", static_cast<int>(_mipmapFilter));
		settings.endGroup();

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

	graphics::TextureFilter GetMinFilter() const { return _minFilter; }

	graphics::TextureFilter GetMagFilter() const { return _magFilter; }

	graphics::MipmapFilter GetMipmapFilter() const { return _mipmapFilter; }

	void SetTextureFilters(graphics::TextureFilter minFilter, graphics::TextureFilter magFilter, graphics::MipmapFilter mipmapFilter)
	{
		_minFilter = minFilter;
		_magFilter = magFilter;
		_mipmapFilter = mipmapFilter;
	}

	bool ShouldResizeTexturesToPowerOf2() const { return _powerOf2Textures; }

	void SetResizeTexturesToPowerOf2(bool value)
	{
		_powerOf2Textures = value;
	}

	bool ShouldActivateTextureViewWhenTexturesPanelOpened() const { return _activateTextureViewWhenTexturesPanelOpened; }

	void SetActivateTextureViewWhenTexturesPanelOpened(bool value)
	{
		_activateTextureViewWhenTexturesPanelOpened = value;
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
	bool _powerOf2Textures{DefaultPowerOf2Textures};
	bool _activateTextureViewWhenTexturesPanelOpened{DefaultActivateTextureViewWhenTexturesPanelOpened};

	int _floorLength = DefaultFloorLength;

	QString _studiomdlCompilerFileName;
	QString _studiomdlDecompilerFileName;

	graphics::TextureFilter _minFilter{DefaultMinFilter};
	graphics::TextureFilter _magFilter{DefaultMagFilter};
	graphics::MipmapFilter _mipmapFilter{DefaultMipmapFilter};
};
}
