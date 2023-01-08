#pragma once

#include <QString>
#include <QWidget>

#include "ui_OptionsPageExternalPrograms.h"

#include "ui/options/OptionsPage.hpp"

class ApplicationSettings;
class AssetManager;

extern const QString ExternalProgramsExeFilter;

class OptionsPageExternalPrograms : public OptionsPage
{
public:
	OptionsPageExternalPrograms(const std::shared_ptr<ApplicationSettings>& applicationSettings);
	~OptionsPageExternalPrograms();

private:
	const std::shared_ptr<ApplicationSettings> _applicationSettings;
};

class OptionsPageExternalProgramsWidget final : public OptionsWidget
{
public:
	OptionsPageExternalProgramsWidget(AssetManager* application, ApplicationSettings* applicationSettings);
	~OptionsPageExternalProgramsWidget();

	void ApplyChanges() override;

private:
	Ui_OptionsPageExternalPrograms _ui;

	AssetManager* const _application;

	ApplicationSettings* const _applicationSettings;
};
