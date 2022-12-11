#pragma once

#include <QSettings>
#include <QString>
#include <QWidget>

#include "ui_OptionsPageGeneral.h"

#include "ui/options/OptionsPage.hpp"

namespace ui
{
class EditorContext;

namespace settings
{
class GeneralSettings;
class RecentFilesSettings;
}

namespace options
{
extern const QString OptionsPageGeneralCategory;

class OptionsPageGeneral : public OptionsPage
{
public:
	OptionsPageGeneral(
		const std::shared_ptr<settings::GeneralSettings>& generalSettings, const std::shared_ptr<settings::RecentFilesSettings>& recentFilesSettings);
	~OptionsPageGeneral();

private:
	const std::shared_ptr<settings::GeneralSettings> _generalSettings;
	const std::shared_ptr<settings::RecentFilesSettings> _recentFilesSettings;
};

class OptionsPageGeneralWidget final : public OptionsWidget
{
public:
	OptionsPageGeneralWidget(
		EditorContext* editorContext, settings::GeneralSettings* generalSettings, settings::RecentFilesSettings* recentFilesSettings);
	~OptionsPageGeneralWidget();

	void ApplyChanges(QSettings& settings) override;

private:
	Ui_OptionsPageGeneral _ui;

	EditorContext* const _editorContext;

	settings::GeneralSettings* const _generalSettings;
	settings::RecentFilesSettings* const _recentFilesSettings;
};
}
}
