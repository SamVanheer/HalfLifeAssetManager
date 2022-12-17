#pragma once

#include <QString>
#include <QWidget>

#include "ui_FileListPanel.h"

class QFileSystemModel;

class EditorContext;

class GameConfiguration;
class GameEnvironment;

class FileListPanel final : public QWidget
{
	Q_OBJECT

public:
	FileListPanel(EditorContext* editorContext, QWidget* parent);
	~FileListPanel();

private:
	void SetRootDirectory(const QString& directory);

signals:
	void FileSelected(const QString& fileName);

private slots:
	void UpdateCurrentRootPath(std::pair<GameEnvironment*, GameConfiguration*> activeConfiguration);

	void OnFilterChanged();

	void OnFileSelected(const QModelIndex& index);

private:
	Ui_FileListPanel _ui;
	QFileSystemModel* _model;
};
