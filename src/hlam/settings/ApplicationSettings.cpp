#include <QFileInfo>
#include <QSettings>

#include "settings/ApplicationSettings.hpp"
#include "settings/ColorSettings.hpp"
#include "settings/GameConfigurationsSettings.hpp"
#include "settings/RecentFilesSettings.hpp"

static const QString StudiomdlCompilerFileNameKey{QStringLiteral("ExternalPrograms/StudioMdlCompilerFileName")};
static const QString StudiomdlDecompilerFileNameKey{QStringLiteral("ExternalPrograms/StudioMdlDecompilerFileName")};
static const QString XashModelViewerFileNameKey{QStringLiteral("ExternalPrograms/XashModelViewerFileName")};
static const QString Quake1ModelViewerFileNameKey{QStringLiteral("ExternalPrograms/Quake1ModelViewerFileName")};
static const QString Source1ModelViewerFileNameKey{QStringLiteral("ExternalPrograms/Source1ModelViewerFileName")};

ApplicationSettings::ApplicationSettings(QSettings* settings)
	: _settings(settings)
	, _recentFiles(std::make_unique<RecentFilesSettings>(settings))
	, _colorSettings(std::make_unique<ColorSettings>(settings))
	, _gameConfigurations(std::make_unique<GameConfigurationsSettings>(settings))
{
	_settings->setParent(this);
}

ApplicationSettings::~ApplicationSettings() = default;

void ApplicationSettings::LoadSettings()
{
	_settings->beginGroup("General");
	PauseAnimationsOnTimelineClick = _settings->value("PauseAnimationsOnTimelineClick", DefaultPauseAnimationsOnTimelineClick).toBool();
	OneAssetAtATime = _settings->value("OneAssetAtATime", DefaultOneAssetAtATime).toBool();
	PromptExternalProgramLaunch = _settings->value(
		"PromptExternalProgramLaunch", DefaultPromptExternalProgramLaunch).toBool();
	_tickRate = std::clamp(_settings->value("TickRate", DefaultTickRate).toInt(), MinimumTickRate, MaximumTickRate);

	GuidelinesAspectRatio = static_cast<::GuidelinesAspectRatio>(_settings->value(
		"GuidelinesAspectRatio", static_cast<int>(GuidelinesAspectRatio::SixteenNine)).toInt());

	GuidelinesAspectRatio = std::clamp(
		GuidelinesAspectRatio, ::GuidelinesAspectRatio::FourThree, ::GuidelinesAspectRatio::SixteenTen);
	_settings->endGroup();

	_settings->beginGroup("Mouse");
	_invertMouseX = _settings->value("InvertMouseX", false).toBool();
	_invertMouseY = _settings->value("InvertMouseY", false).toBool();
	_mouseSensitivity = std::clamp(_settings->value("MouseSensitivity", DefaultMouseSensitivity).toInt(), MinimumMouseSensitivity, MaximumMouseSensitivity);
	_mouseWheelSpeed = std::clamp(_settings->value("MouseWheelSpeed", DefaultMouseWheelSpeed).toInt(), MinimumMouseWheelSpeed, MaximumMouseWheelSpeed);
	_settings->endGroup();

	_settings->beginGroup("Audio");
	_enableAudioPlayback = _settings->value("EnableAudioPlayback", DefaultEnableAudioPlayback).toBool();
	PlaySounds = _settings->value("PlaySounds", DefaultPlaySounds).toBool();
	FramerateAffectsPitch = _settings->value("FramerateAffectsPitch", DefaultFramerateAffectsPitch).toBool();
	_settings->endGroup();

	_settings->beginGroup("Graphics");

	_powerOf2Textures = _settings->value("PowerOf2Textures", DefaultPowerOf2Textures).toBool();

	_settings->beginGroup("TextureFilters");
	_minFilter = static_cast<graphics::TextureFilter>(std::clamp(
		_settings->value("Min", static_cast<int>(DefaultMinFilter)).toInt(),
		static_cast<int>(graphics::TextureFilter::First),
		static_cast<int>(graphics::TextureFilter::Last)));

	_magFilter = static_cast<graphics::TextureFilter>(std::clamp(
		_settings->value("Mag", static_cast<int>(DefaultMagFilter)).toInt(),
		static_cast<int>(graphics::TextureFilter::First),
		static_cast<int>(graphics::TextureFilter::Last)));

	_mipmapFilter = static_cast<graphics::MipmapFilter>(std::clamp(
		_settings->value("Mipmap", static_cast<int>(DefaultMipmapFilter)).toInt(),
		static_cast<int>(graphics::MipmapFilter::First),
		static_cast<int>(graphics::MipmapFilter::Last)));
	_settings->endGroup();

	_msaaLevel = _settings->value("MSAALevel", DefaultMSAALevel).toInt();
	TransparentScreenshots = _settings->value("TransparentScreenshots", DefaultTransparentScreenshots).toBool();
	_settings->endGroup();

	_recentFiles->LoadSettings();
	_colorSettings->LoadSettings();
	_gameConfigurations->LoadSettings();
}

void ApplicationSettings::SaveSettings()
{
	_settings->beginGroup("General");
	_settings->setValue("PauseAnimationsOnTimelineClick", PauseAnimationsOnTimelineClick);
	_settings->setValue("OneAssetAtATime", OneAssetAtATime);
	_settings->setValue("PromptExternalProgramLaunch", PromptExternalProgramLaunch);
	_settings->setValue("TickRate", _tickRate);
	_settings->setValue("GuidelinesAspectRatio", static_cast<int>(GuidelinesAspectRatio));
	_settings->endGroup();

	_settings->beginGroup("Mouse");
	_settings->setValue("InvertMouseX", _invertMouseX);
	_settings->setValue("InvertMouseY", _invertMouseY);
	_settings->setValue("MouseSensitivity", _mouseSensitivity);
	_settings->setValue("MouseWheelSpeed", _mouseWheelSpeed);
	_settings->endGroup();

	_settings->beginGroup("Audio");
	_settings->setValue("EnableAudioPlayback", _enableAudioPlayback);
	_settings->setValue("PlaySounds", PlaySounds);
	_settings->setValue("FramerateAffectsPitch", FramerateAffectsPitch);
	_settings->endGroup();

	_settings->beginGroup("Graphics");
	_settings->setValue("PowerOf2Textures", _powerOf2Textures);

	_settings->beginGroup("TextureFilters");
	_settings->setValue("Min", static_cast<int>(_minFilter));
	_settings->setValue("Mag", static_cast<int>(_magFilter));
	_settings->setValue("Mipmap", static_cast<int>(_mipmapFilter));
	_settings->endGroup();

	_settings->setValue("MSAALevel", _msaaLevel);
	_settings->setValue("TransparentScreenshots", DefaultTransparentScreenshots);
	_settings->endGroup();

	_recentFiles->SaveSettings();
	_colorSettings->SaveSettings();
	_gameConfigurations->SaveSettings();

	emit SettingsSaved();
}

bool ApplicationSettings::ShouldUseSingleInstance() const
{
	return _settings->value("Startup/UseSingleInstance", DefaultUseSingleInstance).toBool();
}

void ApplicationSettings::SetUseSingleInstance(bool value)
{
	_settings->setValue("Startup/UseSingleInstance", value);
}

bool ApplicationSettings::ShouldAllowTabCloseWithMiddleClick() const
{
	return _settings->value("General/AllowTabCloseWithMiddleClick", DefaultAllowTabCloseWithMiddleClick).toBool();
}

void ApplicationSettings::SetAllowTabCloseWithMiddleClick(bool value)
{
	_settings->setValue("General/AllowTabCloseWithMiddleClick", value);
}

bool ApplicationSettings::ShouldEnableVSync() const
{
	return _settings->value("Graphics/EnableVSync", DefaultEnableVSync).toBool();
}

void ApplicationSettings::SetEnableVSync(bool value)
{
	_settings->setValue("Graphics/EnableVSync", value);
}

QString ApplicationSettings::GetStudiomdlCompilerFileName() const
{
	return _settings->value(StudiomdlCompilerFileNameKey).toString();
}

void ApplicationSettings::SetStudiomdlCompilerFileName(const QString& fileName)
{
	_settings->setValue(StudiomdlCompilerFileNameKey, fileName);
}

QString ApplicationSettings::GetStudiomdlDecompilerFileName() const
{
	return _settings->value(StudiomdlDecompilerFileNameKey).toString();
}

void ApplicationSettings::SetStudiomdlDecompilerFileName(const QString& fileName)
{
	_settings->setValue(StudiomdlDecompilerFileNameKey, fileName);
}

QString ApplicationSettings::GetXashModelViewerFileName() const
{
	return _settings->value(XashModelViewerFileNameKey).toString();
}

void ApplicationSettings::SetXashModelViewerFileName(const QString& fileName)
{
	_settings->setValue(XashModelViewerFileNameKey, fileName);
}

QString ApplicationSettings::GetQuake1ModelViewerFileName() const
{
	return _settings->value(Quake1ModelViewerFileNameKey).toString();
}

void ApplicationSettings::SetQuake1ModelViewerFileName(const QString& fileName)
{
	_settings->setValue(Quake1ModelViewerFileNameKey, fileName);
}

QString ApplicationSettings::GetSource1ModelViewerFileName() const
{
	return _settings->value(Source1ModelViewerFileNameKey).toString();
}

void ApplicationSettings::SetSource1ModelViewerFileName(const QString& fileName)
{
	_settings->setValue(Source1ModelViewerFileNameKey, fileName);
}

QString ApplicationSettings::GetSavedPath(const QString& pathName)
{
	_settings->beginGroup("Paths");
	const QString path{_settings->value(pathName).toString()};
	_settings->endGroup();

	return path;
}

void ApplicationSettings::SetSavedPath(const QString& pathName, const QString& path)
{
	const QFileInfo info(path);

	_settings->beginGroup("Paths");
	_settings->setValue(pathName, info.isFile() ? info.absolutePath() : info.absoluteFilePath());
	_settings->endGroup();
}

QString ApplicationSettings::GetStylePath() const
{
	return _settings->value("Style/CurrentStyle").toString();
}

void ApplicationSettings::SetStylePath(const QString& stylePath)
{
	if (GetStylePath() != stylePath)
	{
		_settings->setValue("Style/CurrentStyle", stylePath);
		emit StylePathChanged(stylePath);
	}
}
