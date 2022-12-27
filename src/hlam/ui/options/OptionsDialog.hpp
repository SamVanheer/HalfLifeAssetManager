#pragma once

#include <vector>

#include <QAbstractButton>
#include <QDialog>

#include "ui_OptionsDialog.h"

class EditorContext;

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
	Ui_OptionsDialog _ui;

	EditorContext* const _editorContext;

	std::vector<OptionsPage*> _pages;
};
