#pragma once

#include <QMainWindow>
#include <QObject>
#include <QScopedPointer>
#include <QString>

#include "application/SingleInstance.hpp"

namespace ui
{
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
	ui::HLMVMainWindow* _mainWindow;

	//Used to manage single-instance functionality
	QScopedPointer<SingleInstance> singleInstance;
};
