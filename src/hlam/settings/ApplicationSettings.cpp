#include <cmath>

#include <QFileInfo>
#include <QSettings>

#include "settings/ApplicationSettings.hpp"
#include "settings/ColorSettings.hpp"
#include "settings/GameConfigurationsSettings.hpp"
#include "settings/RecentFilesSettings.hpp"

void ExternalProgramSettings::LoadSettings()
{
	_settings->beginGroup("ExternalPrograms");
	PromptExternalProgramLaunch = _settings->value(
		"PromptExternalProgramLaunch", DefaultPromptExternalProgramLaunch).toBool();

	_settings->beginGroup("Programs");
	// Load all settings that are in the config file even if they haven't been added yet.
	for (auto& program : _settings->childKeys())
	{
		SetProgram(program, _settings->value(program, QString{}).toString());
	}
	_settings->endGroup();
	_settings->endGroup();
}

void ExternalProgramSettings::SaveSettings()
{
	_settings->beginGroup("ExternalPrograms");
	_settings->setValue("PromptExternalProgramLaunch", PromptExternalProgramLaunch);

	_settings->beginGroup("Programs");
	for (auto& program : _externalPrograms.keys())
	{
		_settings->setValue(program, _externalPrograms[program].ExecutablePath);
	}
	_settings->endGroup();
	_settings->endGroup();
}

void ExternalProgramSettings::AddProgram(const QString& key, const QString& name)
{
	// Insert or update entry.
	_externalPrograms[key].Name = name;
}

QString ExternalProgramSettings::GetName(const QString& key) const
{
	return _externalPrograms.value(key).Name;
}

QString ExternalProgramSettings::GetProgram(const QString& key) const
{
	return _externalPrograms.value(key).ExecutablePath;
}

void ExternalProgramSettings::SetProgram(const QString& key, const QString& value)
{
	_externalPrograms[key].ExecutablePath = value;
}

ApplicationSettings::ApplicationSettings(QSettings* settings)
	: _settings(settings)
	, _recentFiles(std::make_unique<RecentFilesSettings>(settings))
	, _colorSettings(std::make_unique<ColorSettings>(settings))
	, _gameConfigurations(std::make_unique<GameConfigurationsSettings>(settings))
	, _externalPrograms(std::make_unique<ExternalProgramSettings>(settings))
{
	_settings->setParent(this);
	_settingsObjects.push_back(_externalPrograms.get());
}

ApplicationSettings::~ApplicationSettings() = default;

void ApplicationSettings::LoadSettings()
{
	_settings->beginGroup("General");
	PauseAnimationsOnTimelineClick = _settings->value("PauseAnimationsOnTimelineClick", DefaultPauseAnimationsOnTimelineClick).toBool();
	OneAssetAtATime = _settings->value("OneAssetAtATime", DefaultOneAssetAtATime).toBool();
	_tickRate = std::clamp(_settings->value("TickRate", DefaultTickRate).toInt(), MinimumTickRate, MaximumTickRate);
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

	_settings->beginGroup("Video");

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
	_aspectRatio.x = std::floor(_settings->value("AspectRatio/X", DefaultAspectRatio.x).toFloat());
	_aspectRatio.y = std::floor(_settings->value("AspectRatio/Y", DefaultAspectRatio.y).toFloat());
	_settings->endGroup();

	_recentFiles->LoadSettings();
	_colorSettings->LoadSettings();
	_gameConfigurations->LoadSettings();

	for (auto settings : _settingsObjects)
	{
		settings->LoadSettings();
	}
}

void ApplicationSettings::SaveSettings()
{
	_settings->beginGroup("General");
	_settings->setValue("PauseAnimationsOnTimelineClick", PauseAnimationsOnTimelineClick);
	_settings->setValue("OneAssetAtATime", OneAssetAtATime);
	_settings->setValue("TickRate", _tickRate);
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

	_settings->beginGroup("Video");
	_settings->setValue("PowerOf2Textures", _powerOf2Textures);

	_settings->beginGroup("TextureFilters");
	_settings->setValue("Min", static_cast<int>(_minFilter));
	_settings->setValue("Mag", static_cast<int>(_magFilter));
	_settings->setValue("Mipmap", static_cast<int>(_mipmapFilter));
	_settings->endGroup();

	_settings->setValue("MSAALevel", _msaaLevel);
	_settings->setValue("TransparentScreenshots", DefaultTransparentScreenshots);
	_settings->setValue("AspectRatio/X", _aspectRatio.x);
	_settings->setValue("AspectRatio/Y", _aspectRatio.y);
	_settings->endGroup();

	_recentFiles->SaveSettings();
	_colorSettings->SaveSettings();
	_gameConfigurations->SaveSettings();

	for (auto settings : _settingsObjects)
	{
		settings->SaveSettings();
	}

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
	return _settings->value("Video/EnableVSync", DefaultEnableVSync).toBool();
}

void ApplicationSettings::SetEnableVSync(bool value)
{
	_settings->setValue("Video/EnableVSync", value);
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
