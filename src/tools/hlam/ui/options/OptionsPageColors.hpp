#pragma once

#include <QColor>
#include <QSettings>
#include <QString>
#include <QWidget>

#include "ui_OptionsPageColors.h"

#include "ui/options/OptionsPage.hpp"

namespace ui
{
class EditorContext;

namespace settings
{
class ColorSettings;
}

namespace options
{
extern const QString OptionsPageColorsId;

class OptionsPageColors : public OptionsPage
{
public:
	OptionsPageColors(
		const std::shared_ptr<settings::ColorSettings>& colorSettings);
	~OptionsPageColors();

private:
	const std::shared_ptr<settings::ColorSettings> _colorSettings;
};

class OptionsPageColorsWidget final : public OptionsWidget
{
public:
	OptionsPageColorsWidget(
		EditorContext* editorContext, settings::ColorSettings* colorSettings);
	~OptionsPageColorsWidget();

	void ApplyChanges(QSettings& settings) override;

private:
	void SetPreviewColor(const QColor& color);

private slots:
	void OnKeyChanged(int index);

	void OnChangeColor();

	void OnResetColor();

private:
	Ui_OptionsPageColors _ui;

	EditorContext* const _editorContext;

	settings::ColorSettings* const _colorSettings;
};
}
}
