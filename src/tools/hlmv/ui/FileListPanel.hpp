#pragma once

#include <QString>
#include <QWidget>

#include "ui_FileListPanel.h"

class QFileSystemModel;

namespace ui
{
class EditorContext;

class FileListPanel final : public QWidget
{
	Q_OBJECT

public:
	FileListPanel(EditorContext* editorContext, QWidget* parent = nullptr);
	~FileListPanel();

signals:
	void FileSelected(const QString& fileName);

private slots:
	void OnFilterChanged();

	void OnFileSelected(const QModelIndex& index);

private:
	Ui_FileListPanel _ui;
	QFileSystemModel* _model;
};
}
