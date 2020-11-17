#include "ui/HLMVMainWindow.hpp"

namespace ui
{
HLMVMainWindow::HLMVMainWindow()
	: QMainWindow()
{
	_ui.setupUi(this);
}

HLMVMainWindow::~HLMVMainWindow() = default;
}
