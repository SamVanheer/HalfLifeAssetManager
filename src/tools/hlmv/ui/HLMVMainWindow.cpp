#include <QMessageBox>

#include "Credits.hpp"

#include "ui/HLMVMainWindow.hpp"

namespace ui
{
HLMVMainWindow::HLMVMainWindow()
	: QMainWindow()
{
	_ui.setupUi(this);

	connect(_ui.ActionAbout, &QAction::triggered, this, &HLMVMainWindow::OnShowAbout);
}

HLMVMainWindow::~HLMVMainWindow() = default;

void HLMVMainWindow::OnShowAbout()
{
	QMessageBox::information(this, "About Half-Life Model Viewer",
		QString::fromUtf8(
			u8"Half-Life Model Viewer 3.0\n"
			u8"2020 Sam \"Solokiller\" Vanheer\n\n"
			u8"Email:    sam.vanheer@outlook.com\n\n"
			u8"Based on Jed's Half-Life Model Viewer v1.3 © 2004 Neil \'Jed\' Jedrzejewski\n"
			u8"Email:    jed@wunderboy.org\n"
			u8"Web:      http://www.wunderboy.org/\n\n"
			u8"Also based on Half-Life Model Viewer v1.25 © 2002 Mete Ciragan\n"
			u8"Email:    mete@swissquake.ch\n"
			u8"Web:      http://www.milkshape3d.com/\n\n"
			u8"%1")
			.arg(QString::fromUtf8(tools::GetSharedCredits().c_str()))
	);
}
}
