#pragma once

#include <QSettings>
#include <QWidget>

#include "ui_OptionsPageGeneral.h"

namespace ui
{
class EditorContext;

namespace options
{
class OptionsPageGeneral final : public QWidget
{
	Q_OBJECT

public:
	OptionsPageGeneral(EditorContext* editorContext, QWidget* parent = nullptr);
	~OptionsPageGeneral();

public slots:
	void OnSaveChanges(QSettings& settings);

	void OnResetFloorLength();

private:
	Ui_OptionsPageGeneral _ui;

	EditorContext* const _editorContext;
};
}
}
