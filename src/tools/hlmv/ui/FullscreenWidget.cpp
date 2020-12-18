#include <QKeyEvent>
#include <QtPlatformHeaders/QWindowsWindowFunctions>

#include "ui/EditorContext.hpp"
#include "ui/FullscreenWidget.hpp"

namespace ui
{
FullscreenWidget::FullscreenWidget(QWidget* parent)
	: QMainWindow(parent)
{
	//This is not a primary window so don't keep the app running if we're still alive
	setAttribute(Qt::WidgetAttribute::WA_QuitOnClose, false);

	//This has to be a native window for there to be a window handle
	setAttribute(Qt::WidgetAttribute::WA_NativeWindow, true);

	//Without this going fullscreen will cause black flickering
	QWindowsWindowFunctions::setHasBorderInFullScreen(this->windowHandle(), true);
}

FullscreenWidget::~FullscreenWidget() = default;

void FullscreenWidget::ExitFullscreen()
{
	this->hide();
	//This ensures the old widget is deleted now to avoid having a dangling reference to an unloaded asset
	auto oldWidget = this->takeCentralWidget();

	delete oldWidget;
}

bool FullscreenWidget::eventFilter(QObject* object, QEvent* event)
{
	if (event->type() == QEvent::Type::KeyPress)
	{
		const auto keyEvent = static_cast<QKeyEvent*>(event);

		if (keyEvent->key() == ExitFullscreenKey)
		{
			this->ExitFullscreen();
			return true;
		}
	}

	return false;
}

void FullscreenWidget::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == ExitFullscreenKey)
	{
		this->ExitFullscreen();
		return;
	}

	QMainWindow::keyPressEvent(event);
}
}
