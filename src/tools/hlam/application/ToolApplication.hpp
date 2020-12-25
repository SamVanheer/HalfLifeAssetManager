#pragma once

#include <utility>

#include <QMainWindow>
#include <QObject>
#include <QScopedPointer>
#include <QString>

#include "application/SingleInstance.hpp"

class QApplication;
class QOffscreenSurface;
class QOpenGLContext;

namespace ui
{
class EditorContext;
class MainWindow;
}

/**
*	@brief Handles program startup and shutdown
*/
class ToolApplication final : public QObject
{
	Q_OBJECT

public:
	ToolApplication() = default;
	~ToolApplication() = default;

	int Run(int argc, char* argv[]);

private:
	void ConfigureApplication(const QString& programName);
	ui::EditorContext* CreateEditorContext();
	void ConfigureOpenGL();

	QString ParseCommandLine(QApplication& application);

	bool CheckSingleInstance(const QString& programName, const QString& fileName);

	std::pair<QOpenGLContext*, QOffscreenSurface*> InitializeOpenGL();

private slots:
	void OnExit();

	void OnFileNameReceived(const QString& fileName);

private:
	ui::EditorContext* _editorContext{};
	ui::MainWindow* _mainWindow{};

	QScopedPointer<SingleInstance> _singleInstance;
};
