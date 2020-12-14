#pragma once

#include <QSettings>
#include <QString>
#include <QWidget>

#include "ui_OptionsPageGeneral.h"

#include "ui/options/OptionsPage.hpp"

namespace ui
{
class EditorContext;

namespace options
{
extern const QString OptionsPageGeneralId;

class OptionsPageGeneral : public OptionsPage
{
public:
	OptionsPageGeneral();
};

class OptionsPageGeneralWidget final : public OptionsWidget
{
	Q_OBJECT

public:
	OptionsPageGeneralWidget(EditorContext* editorContext, QWidget* parent = nullptr);
	~OptionsPageGeneralWidget();

	void ApplyChanges(QSettings& settings) override;

public slots:
	void OnSaveChanges(QSettings& settings);

	void OnResetFloorLength();

private:
	Ui_OptionsPageGeneral _ui;

	EditorContext* const _editorContext;
};
}
}
