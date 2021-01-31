#pragma once

#include <QSettings>
#include <QString>
#include <QWidget>

#include "ui_OptionsPageStyle.h"

#include "ui/options/OptionsPage.hpp"

namespace ui
{
class EditorContext;

namespace settings
{
class StyleSettings;
}

namespace options
{
class OptionsPageStyle : public OptionsPage
{
public:
	OptionsPageStyle(const std::shared_ptr<settings::StyleSettings>& styleSettings);
	~OptionsPageStyle();

private:
	const std::shared_ptr<settings::StyleSettings> _styleSettings;
};

class OptionsPageStyleWidget final : public OptionsWidget
{
public:
	OptionsPageStyleWidget(EditorContext* editorContext, settings::StyleSettings* styleSettings, QWidget* parent = nullptr);
	~OptionsPageStyleWidget();

	void ApplyChanges(QSettings& settings) override;

private slots:
	void OnBrowseStyle();

private:
	Ui_OptionsPageStyle _ui;

	EditorContext* const _editorContext;

	settings::StyleSettings* const _styleSettings;
};
}
}
