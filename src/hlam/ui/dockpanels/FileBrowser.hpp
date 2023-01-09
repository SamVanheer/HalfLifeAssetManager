#pragma once

#include <QString>
#include <QWidget>

#include "ui_FileBrowser.h"

class AssetFilterModel;
class AssetManager;
class GameConfiguration;
class QFileSystemModel;

class FileBrowser final : public QWidget
{
	Q_OBJECT

public:
	FileBrowser(AssetManager* application, QWidget* parent);
	~FileBrowser();

private:
	void SetRootDirectory(const QString& directory);

signals:
	void FileSelected(const QString& fileName);

private slots:
	void OnFileSelected(const QModelIndex& index);

private:
	Ui_FileBrowser _ui;
	AssetManager* const _application;
	QFileSystemModel* const _model;
	AssetFilterModel* const _filterModel;
};
