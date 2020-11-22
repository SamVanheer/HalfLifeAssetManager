#pragma once

#include <QAbstractButton>
#include <QDialog>
#include <QSettings>

#include "ui_OptionsDialog.h"

namespace ui
{
class EditorUIContext;

namespace options
{
class OptionsPageGeneral;

class OptionsDialog final : public QDialog
{
	Q_OBJECT

public:
	OptionsDialog(EditorUIContext* editorContext, QWidget* parent = nullptr);
	~OptionsDialog();

signals:
	void SaveChanges(QSettings& settings);

private slots:
	void OnButtonClicked(QAbstractButton* button);

private:
	ui::Ui_OptionsDialog _ui;

	EditorUIContext* const _editorContext;

	OptionsPageGeneral* _pageGeneral;
};
}
}
