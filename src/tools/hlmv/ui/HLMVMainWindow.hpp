#pragma once

#include <QMainWindow>

#include "ui_HLMVMainWindow.h"

namespace ui
{
class HLMVMainWindow final : public QMainWindow
{
	Q_OBJECT

public:
	HLMVMainWindow();
	~HLMVMainWindow();

private slots:
	void OnShowAbout();

private:
	Ui_HLMVMainWindow _ui;
};
}
