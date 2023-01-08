#pragma once

#include <vector>

#include <QAbstractButton>
#include <QDialog>

#include "ui_OptionsDialog.h"

class AssetManager;

class OptionsPage;

class OptionsDialog final : public QDialog
{
public:
	OptionsDialog(AssetManager* application, QWidget* parent);
	~OptionsDialog();

private slots:
	void OnPageSelected(QTreeWidgetItem* current, QTreeWidgetItem* previous);

	void OnButtonClicked(QAbstractButton* button);

private:
	Ui_OptionsDialog _ui;

	AssetManager* const _application;

	std::vector<OptionsPage*> _pages;
};
