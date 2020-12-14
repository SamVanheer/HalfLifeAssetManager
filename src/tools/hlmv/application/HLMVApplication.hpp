#pragma once

#include <QMainWindow>
#include <QObject>
#include <QScopedPointer>
#include <QString>

#include "application/SingleInstance.hpp"

namespace ui
{
class EditorContext;
class HLMVMainWindow;
}

/**
*	@brief Handles program startup and shutdown
*/
class HLMVApplication final : public QObject
{
	Q_OBJECT

public:
	HLMVApplication() = default;
	~HLMVApplication() = default;

	int Run(int argc, char* argv[]);

private slots:
	void OnExit();

	void OnFileNameReceived(const QString& fileName);

private:
	ui::EditorContext* _editorContext{};
	ui::HLMVMainWindow* _mainWindow{};

	QScopedPointer<SingleInstance> _singleInstance;
};
