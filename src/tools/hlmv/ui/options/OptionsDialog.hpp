#pragma once

#include <QAbstractButton>
#include <QDialog>
#include <QSettings>

#include "ui_OptionsDialog.h"

namespace ui
{
class EditorContext;

namespace options
{
class OptionsPageGeneral;

class OptionsDialog final : public QDialog
{
	Q_OBJECT

public:
	OptionsDialog(EditorContext* editorContext, QWidget* parent = nullptr);
	~OptionsDialog();

signals:
	void SaveChanges(QSettings& settings);

private slots:
	void OnButtonClicked(QAbstractButton* button);

private:
	ui::Ui_OptionsDialog _ui;

	EditorContext* const _editorContext;

	OptionsPageGeneral* _pageGeneral;
};
}
}
