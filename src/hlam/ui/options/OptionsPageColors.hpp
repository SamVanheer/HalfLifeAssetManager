#pragma once

#include <QColor>
#include <QString>
#include <QWidget>

#include "ui_OptionsPageColors.h"

#include "ui/options/OptionsPage.hpp"

class ApplicationSettings;
class ColorSettings;
class EditorContext;

extern const QString OptionsPageColorsId;

class OptionsPageColors : public OptionsPage
{
public:
	OptionsPageColors(
		const std::shared_ptr<ApplicationSettings>& applicationSettings);
	~OptionsPageColors();

private:
	const std::shared_ptr<ApplicationSettings> _applicationSettings;
};

class OptionsPageColorsWidget final : public OptionsWidget
{
public:
	OptionsPageColorsWidget(
		EditorContext* editorContext, ColorSettings* colorSettings);
	~OptionsPageColorsWidget();

	void ApplyChanges() override;

private:
	void SetPreviewColor(const QColor& color);

private slots:
	void OnKeyChanged(int index);

	void OnChangeColor();

	void OnResetColor();

private:
	Ui_OptionsPageColors _ui;

	EditorContext* const _editorContext;

	ColorSettings* const _colorSettings;
};
