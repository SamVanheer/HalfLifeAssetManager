#pragma once

#include <QDialog>

#include "ui_EditGameConfigurationsDialog.h"

class QStandardItemModel;
class QString;

class EditGameConfigurationsDialog final : public QDialog
{
	Q_OBJECT

public:
	EditGameConfigurationsDialog(
		QStandardItemModel* gameConfigurationsModel, int defaultConfigurationIndex, QWidget* parent);
	~EditGameConfigurationsDialog();

signals:
	void ConfigurationAdded(const QString& name);
	void ConfigurationRemoved(int index);
	void ConfigurationCopied(int index);
	void ConfigurationRenamed(int index, const QString& name);

private:
	Ui_EditGameConfigurationsDialog _ui;
	QStandardItemModel* const _gameConfigurationsModel;
};
