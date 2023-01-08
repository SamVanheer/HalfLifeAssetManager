#pragma once

#include <QString>
#include <QWidget>

#include "ui_OptionsPageStyle.h"

#include "ui/options/OptionsPage.hpp"

class ApplicationSettings;
class AssetManager;

class OptionsPageStyle : public OptionsPage
{
public:
	OptionsPageStyle(const std::shared_ptr<ApplicationSettings>& applicationSettings);
	~OptionsPageStyle();

private:
	const std::shared_ptr<ApplicationSettings> _applicationSettings;
};

class OptionsPageStyleWidget final : public OptionsWidget
{
public:
	OptionsPageStyleWidget(AssetManager* application, ApplicationSettings* applicationSettings);
	~OptionsPageStyleWidget();

	void ApplyChanges() override;

private slots:
	void OnBrowseStyle();

private:
	Ui_OptionsPageStyle _ui;

	AssetManager* const _application;

	ApplicationSettings* const _applicationSettings;
};
