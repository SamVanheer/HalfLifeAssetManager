#pragma once

#include <QSettings>
#include <QWidget>

#include "ui_OptionsPageGeneral.h"

namespace ui
{
class EditorUIContext;

namespace options
{
class OptionsPageGeneral final : public QWidget
{
	Q_OBJECT

public:
	OptionsPageGeneral(EditorUIContext* editorContext, QWidget* parent = nullptr);
	~OptionsPageGeneral();

public slots:
	void OnSaveChanges(QSettings& settings);

	void OnResetFloorLength();

private:
	Ui_OptionsPageGeneral _ui;

	EditorUIContext* const _editorContext;
};
}
}
