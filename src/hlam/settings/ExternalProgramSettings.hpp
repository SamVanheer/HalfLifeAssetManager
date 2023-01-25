#pragma once

#include <QMap>
#include <QString>

#include "settings/BaseSettings.hpp"

struct ExternalProgram
{
	QString Name;
	QString ExecutablePath;
	QString AdditionalArguments;
};

class ExternalProgramSettings final : public BaseSettings
{
public:
	static constexpr bool DefaultPromptExternalProgramLaunch{true};

	using BaseSettings::BaseSettings;

	void LoadSettings() override;
	void SaveSettings() override;

	const QMap<QString, ExternalProgram>& GetMap() const { return _externalPrograms; }

	void AddProgram(const QString& key, const QString& name);

	QString GetName(const QString& key) const;

	QString GetProgram(const QString& key) const;

	QString GetAdditionalArguments(const QString& key) const;

	void SetProgram(const QString& key, const QString& value);

	void SetAdditionalArguments(const QString& key, const QString& value);

	bool PromptExternalProgramLaunch{DefaultPromptExternalProgramLaunch};

private:
	QMap<QString, ExternalProgram> _externalPrograms;
};
