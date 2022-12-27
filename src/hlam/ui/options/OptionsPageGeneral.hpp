#pragma once

#include <QSettings>
#include <QString>
#include <QWidget>

#include "ui_OptionsPageGeneral.h"

#include "ui/options/OptionsPage.hpp"

class EditorContext;

class ApplicationSettings;
class RecentFilesSettings;

extern const QString OptionsPageGeneralCategory;

class OptionsPageGeneral : public OptionsPage
{
public:
	OptionsPageGeneral(const std::shared_ptr<ApplicationSettings>& applicationSettings,
		const std::shared_ptr<RecentFilesSettings>& recentFilesSettings);
	~OptionsPageGeneral();

private:
	const std::shared_ptr<ApplicationSettings> _applicationSettings;
	const std::shared_ptr<RecentFilesSettings> _recentFilesSettings;
};

class OptionsPageGeneralWidget final : public OptionsWidget
{
public:
	OptionsPageGeneralWidget(EditorContext* editorContext, ApplicationSettings* applicationSettings,
		RecentFilesSettings* recentFilesSettings);
	~OptionsPageGeneralWidget();

	void ApplyChanges(QSettings& settings) override;

private:
	Ui_OptionsPageGeneral _ui;

	EditorContext* const _editorContext;

	ApplicationSettings* const _applicationSettings;
	RecentFilesSettings* const _recentFilesSettings;
};
