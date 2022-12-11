#pragma once

#include <vector>

#include <QAbstractButton>
#include <QDialog>
#include <QSettings>

#include "ui_OptionsDialog.h"

namespace ui
{
class EditorContext;

namespace options
{
class OptionsPage;

class OptionsDialog final : public QDialog
{
public:
	OptionsDialog(EditorContext* editorContext, QWidget* parent);
	~OptionsDialog();

private slots:
	void OnPageSelected(QTreeWidgetItem* current, QTreeWidgetItem* previous);

	void OnButtonClicked(QAbstractButton* button);

private:
	ui::Ui_OptionsDialog _ui;

	EditorContext* const _editorContext;

	std::vector<OptionsPage*> _pages;
};
}
}
