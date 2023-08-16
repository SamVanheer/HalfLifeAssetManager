#pragma once

#include <QString>
#include <QStringList>
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

	void Initialize();

private:
	void SetRootDirectory(const QString& directory);

	void MaybeOpenFiles(const QModelIndexList& indices);

signals:
	void FilesSelected(const QStringList& fileNames);

private slots:
	void OnFileSelected(const QModelIndex& index);
	void OnFileDoubleClicked(const QModelIndex& index);

private:
	Ui_FileBrowser _ui;
	AssetManager* const _application;
	QFileSystemModel* const _model;
	AssetFilterModel* const _filterModel;

	bool _initialized{false};
};
