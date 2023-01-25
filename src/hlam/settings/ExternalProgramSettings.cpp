#include <QSettings>

#include "settings/ExternalProgramSettings.hpp"

void ExternalProgramSettings::LoadSettings()
{
	_settings->beginGroup("ExternalPrograms");
	PromptExternalProgramLaunch = _settings->value(
		"PromptExternalProgramLaunch", DefaultPromptExternalProgramLaunch).toBool();

	_settings->beginGroup("Programs");
	for (auto& program : _externalPrograms.keys())
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
	_externalPrograms[key].ExecutablePath = value.trimmed();
}
