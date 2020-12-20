#pragma once

#include <QMainWindow>
#include <QObject>
#include <QScopedPointer>
#include <QString>

#include "application/SingleInstance.hpp"

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

private slots:
	void OnExit();

	void OnFileNameReceived(const QString& fileName);

private:
	ui::EditorContext* _editorContext{};
	ui::MainWindow* _mainWindow{};

	QScopedPointer<SingleInstance> _singleInstance;
};
