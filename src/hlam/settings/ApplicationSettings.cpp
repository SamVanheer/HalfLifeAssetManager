#include <algorithm>
#include <cassert>
#include <cmath>

#include <QFileInfo>
#include <QSettings>

#include "settings/ApplicationSettings.hpp"
#include "settings/BaseSettings.hpp"
#include "settings/ColorSettings.hpp"
#include "settings/ExternalProgramSettings.hpp"
#include "settings/GameConfigurationsSettings.hpp"
#include "settings/RecentFilesSettings.hpp"

ApplicationSettings::ApplicationSettings(QSettings* settings, std::shared_ptr<spdlog::logger> fileSystemLogger)
	: _settings(settings)
	, _recentFiles(std::make_unique<RecentFilesSettings>(settings))
	, _colorSettings(std::make_unique<ColorSettings>(settings))
	, _gameConfigurations(std::make_unique<GameConfigurationsSettings>(settings, fileSystemLogger))
	, _externalPrograms(std::make_unique<ExternalProgramSettings>(settings))
{
	_settings->setParent(this);
	_settingsObjects.push_back(_recentFiles.get());
	_settingsObjects.push_back(_colorSettings.get());
	_settingsObjects.push_back(_gameConfigurations.get());
	_settingsObjects.push_back(_externalPrograms.get());
}

ApplicationSettings::~ApplicationSettings() = default;

void ApplicationSettings::RegisterSettings(BaseSettings* settings)
{
	if (!settings)
	{
		assert(!"Null settings object passed");
		return;
	}

	if (_settingsObjects.contains(settings))
	{
		assert(!"Cannot add settings objects twice!");
		return;
	}

	_settingsObjects.push_back(settings);

	connect(settings, &BaseSettings::destroyed, this, &ApplicationSettings::OnSettingsDestroyed);
}

void ApplicationSettings::OnSettingsDestroyed(QObject* object)
{
	auto settings = static_cast<BaseSettings*>(object);

	auto index = _settingsObjects.indexOf(settings);

	if (index == -1)
	{
		assert(!"Couldn't find settings object being destroyed");
		return;
	}

	_settingsObjects.remove(index);
}

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
	_aspectRatio.x = std::clamp(_settings->value("AspectRatio/X", static_cast<int>(DefaultAspectRatio.x)).toInt(),
		MinimumAspectRatio, MaximumAspectRatio);
	_aspectRatio.y = std::clamp(_settings->value("AspectRatio/Y", static_cast<int>(DefaultAspectRatio.y)).toInt(),
		MinimumAspectRatio, MaximumAspectRatio);
	_settings->endGroup();

	for (auto settings : _settingsObjects)
	{
		settings->LoadSettings();
	}

	emit SettingsLoaded();
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
	_settings->setValue("AspectRatio/X", static_cast<int>(_aspectRatio.x));
	_settings->setValue("AspectRatio/Y", static_cast<int>(_aspectRatio.y));
	_settings->endGroup();

	for (auto settings : _settingsObjects)
	{
		settings->SaveSettings();
	}

	emit SettingsSaved();
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
	if (ShouldEnableVSync() != value)
	{
		_settings->setValue("Video/EnableVSync", value);
		emit SceneWidgetSettingsChanged();
	}
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

QString ApplicationSettings::GetFileListFilter() const
{
	return _settings->value("FileList/Filter", QString{}).toString();
}

void ApplicationSettings::SetFileListFilter(const QString& filterName)
{
	_settings->setValue("FileList/Filter", filterName);
}

QString ApplicationSettings::GetFileListRootDirectory() const
{
	return _settings->value("FileList/RootDirectory", QString{}).toString();
}

void ApplicationSettings::SetFileListRootDirectory(const QString& directory)
{
	_settings->setValue("FileList/RootDirectory", directory);
}
