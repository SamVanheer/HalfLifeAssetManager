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
}

namespace options
{
extern const QString OptionsPageGeneralId;

class OptionsPageGeneral : public OptionsPage
{
public:
	OptionsPageGeneral(std::unique_ptr<settings::GeneralSettings>&& generalSettings);
	~OptionsPageGeneral();

private:
	const std::unique_ptr<settings::GeneralSettings> _generalSettings;
};

class OptionsPageGeneralWidget final : public OptionsWidget
{
public:
	OptionsPageGeneralWidget(EditorContext* editorContext, settings::GeneralSettings* generalSettings, QWidget* parent = nullptr);
	~OptionsPageGeneralWidget();

	void ApplyChanges(QSettings& settings) override;

private:
	Ui_OptionsPageGeneral _ui;

	EditorContext* const _editorContext;

	settings::GeneralSettings* const _generalSettings;
};
}
}
