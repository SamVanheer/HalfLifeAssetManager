#pragma once

#include <QSettings>
#include <QString>
#include <QWidget>

#include "ui_OptionsPageStyle.h"

#include "ui/options/OptionsPage.hpp"

class EditorContext;

class StyleSettings;

class OptionsPageStyle : public OptionsPage
{
public:
	OptionsPageStyle(const std::shared_ptr<StyleSettings>& styleSettings);
	~OptionsPageStyle();

private:
	const std::shared_ptr<StyleSettings> _styleSettings;
};

class OptionsPageStyleWidget final : public OptionsWidget
{
public:
	OptionsPageStyleWidget(EditorContext* editorContext, StyleSettings* styleSettings);
	~OptionsPageStyleWidget();

	void ApplyChanges(QSettings& settings) override;

private slots:
	void OnBrowseStyle();

private:
	Ui_OptionsPageStyle _ui;

	EditorContext* const _editorContext;

	StyleSettings* const _styleSettings;
};
