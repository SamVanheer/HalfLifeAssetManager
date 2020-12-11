#include <QKeyEvent>
#include <QtPlatformHeaders/QWindowsWindowFunctions>

#include "ui/EditorContext.hpp"
#include "ui/FullscreenWidget.hpp"
#include "ui/SceneWidget.hpp"

namespace ui
{
constexpr Qt::Key ExitFullscreenKey{Qt::Key::Key_Escape};

FullscreenWidget::FullscreenWidget(EditorContext* editorContext, graphics::Scene* scene, QWidget* parent)
	: QMainWindow(parent)
{
	//This is not a primary window so don't keep the app running if we're still alive
	setAttribute(Qt::WidgetAttribute::WA_QuitOnClose, false);

	//The window has to be deleted when closed so as to not leave any dangling references behind
	setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose, true);

	//This has to be a native window for there to be a window handle
	setAttribute(Qt::WidgetAttribute::WA_NativeWindow, true);

	//Without this going fullscreen will cause black flickering
	QWindowsWindowFunctions::setHasBorderInFullScreen(this->windowHandle(), true);

	const auto sceneWidget = new SceneWidget(scene, this);

	setCentralWidget(sceneWidget->GetContainer());

	sceneWidget->connect(editorContext, &EditorContext::Tick, sceneWidget, &SceneWidget::requestUpdate);

	//Filter key events on the scene widget so we can capture escape even if it has focus
	sceneWidget->installEventFilter(this);

	//TODO: need to handle user input so they can control the camera
}

FullscreenWidget::~FullscreenWidget() = default;

bool FullscreenWidget::eventFilter(QObject* object, QEvent* event)
{
	if (event->type() == QEvent::Type::KeyPress)
	{
		const auto keyEvent = static_cast<QKeyEvent*>(event);

		if (keyEvent->key() == ExitFullscreenKey)
		{
			this->close();
			return true;
		}
	}

	return false;
}

void FullscreenWidget::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == ExitFullscreenKey)
	{
		this->close();
		return;
	}

	QMainWindow::keyPressEvent(event);
}

void FullscreenWidget::closeEvent(QCloseEvent* event)
{
	emit Closing(this);

	event->accept();
}
}
