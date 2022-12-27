#pragma once

#include <QString>
#include <QWidget>

#include "ui_OptionsPageGeneral.h"

#include "ui/options/OptionsPage.hpp"

class EditorContext;

class ApplicationSettings;

extern const QString OptionsPageGeneralCategory;

class OptionsPageGeneral : public OptionsPage
{
public:
	OptionsPageGeneral(const std::shared_ptr<ApplicationSettings>& applicationSettings);
	~OptionsPageGeneral();

private:
	const std::shared_ptr<ApplicationSettings> _applicationSettings;
};

class OptionsPageGeneralWidget final : public OptionsWidget
{
public:
	OptionsPageGeneralWidget(EditorContext* editorContext, ApplicationSettings* applicationSettings);
	~OptionsPageGeneralWidget();

	void ApplyChanges() override;

private:
	Ui_OptionsPageGeneral _ui;

	EditorContext* const _editorContext;

	ApplicationSettings* const _applicationSettings;
};
