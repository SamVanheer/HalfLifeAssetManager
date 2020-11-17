#pragma once

#include <QAbstractButton>
#include <QDialog>
#include <QSettings>

#include "ui_OptionsDialog.h"

namespace ui::options
{
class OptionsPageGeneral;

class OptionsDialog final : public QDialog
{
	Q_OBJECT

public:
	OptionsDialog(QWidget* parent = nullptr);
	~OptionsDialog();

signals:
	void SaveChanges(QSettings& settings);

private slots:
	void OnButtonClicked(QAbstractButton* button);

private:
	ui::Ui_OptionsDialog _ui;

	OptionsPageGeneral* _pageGeneral;
};
}
