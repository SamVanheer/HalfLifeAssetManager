#pragma once

#include <memory>
#include <utility>

#include <QMainWindow>
#include <QObject>
#include <QPointer>
#include <QScopedPointer>
#include <QSettings>
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
	
	void ConfigureOpenGL();

	std::tuple<bool, bool, QString> ParseCommandLine(QApplication& application);

	std::unique_ptr<QSettings> CreateSettings(const QString& programName, bool isPortable);

	bool CheckSingleInstance(const QString& programName, const QString& fileName, QSettings& settings);

	ui::EditorContext* CreateEditorContext(std::unique_ptr<QSettings>&& settings);

	std::pair<QOpenGLContext*, QOffscreenSurface*> InitializeOpenGL();

private slots:
	void OnExit();

	void OnFileNameReceived(const QString& fileName);

	void OnStylePathChanged(const QString& stylePath);

private:
	QApplication* _application{};

	ui::EditorContext* _editorContext{};
	QPointer<ui::MainWindow> _mainWindow;

	QScopedPointer<SingleInstance> _singleInstance;
};
