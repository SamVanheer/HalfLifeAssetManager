#pragma once

#include <QSettings>
#include <QString>
#include <QWidget>

#include "ui_OptionsPageGeneral.h"

#include "ui/options/OptionsPage.hpp"

class EditorContext;

class GeneralSettings;
class RecentFilesSettings;

extern const QString OptionsPageGeneralCategory;

class OptionsPageGeneral : public OptionsPage
{
public:
	OptionsPageGeneral(
		const std::shared_ptr<GeneralSettings>& generalSettings, const std::shared_ptr<RecentFilesSettings>& recentFilesSettings);
	~OptionsPageGeneral();

private:
	const std::shared_ptr<GeneralSettings> _generalSettings;
	const std::shared_ptr<RecentFilesSettings> _recentFilesSettings;
};

class OptionsPageGeneralWidget final : public OptionsWidget
{
public:
	OptionsPageGeneralWidget(
		EditorContext* editorContext, GeneralSettings* generalSettings, RecentFilesSettings* recentFilesSettings);
	~OptionsPageGeneralWidget();

	void ApplyChanges(QSettings& settings) override;

private:
	Ui_OptionsPageGeneral _ui;

	EditorContext* const _editorContext;

	GeneralSettings* const _generalSettings;
	RecentFilesSettings* const _recentFilesSettings;
};
