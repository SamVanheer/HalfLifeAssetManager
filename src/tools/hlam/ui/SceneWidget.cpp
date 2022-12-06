#include <cassert>

#include <QApplication>
#include <QWheelEvent>
#include <QWidget>

#include "graphics/Scene.hpp"
#include "qt/QtLogging.hpp"
#include "ui/SceneWidget.hpp"

namespace ui
{
SceneWidget::SceneWidget(graphics::Scene* scene, QWidget* parent)
	: QOpenGLWindow()
	, _container(QWidget::createWindowContainer(this, parent))
	, _scene(scene)
{
	assert(nullptr != _scene);

	_container->setFocusPolicy(Qt::FocusPolicy::WheelFocus);

	connect(this, &SceneWidget::frameSwapped, this, qOverload<>(&SceneWidget::update));

	connect(qGuiApp, &QGuiApplication::focusObjectChanged, this, &SceneWidget::OnFocusObjectChanged);

	_previousFocusObject = qGuiApp->focusObject();

	_scene->SetOpenGLFunctions(this);
}

SceneWidget::~SceneWidget()
{
	makeCurrent();

	_scene->Shutdown();

	doneCurrent();
}

bool SceneWidget::event(QEvent* event)
{
	switch (event->type())
	{
	case QEvent::Type::MouseButtonPress:
		[[fallthrough]];

	case QEvent::Type::MouseButtonRelease:
		[[fallthrough]];

	case QEvent::Type::MouseMove:
	{
		emit MouseEvent(static_cast<QMouseEvent*>(event));
		break;
	}

	case QEvent::Type::KeyPress:
		[[fallthrough]];

	case QEvent::Type::KeyRelease:
	{
		//Forward key input to the previous focus object, if any
		if (_previousFocusObject)
		{
			QApplication::sendEvent(_previousFocusObject, event);
		}

		else
		{
			event->ignore();
		}
		break;
	}

	case QEvent::Type::Wheel:
	{
		auto wheelEvent = static_cast<QWheelEvent*>(event);
		//Ugly hack: when this window has focus it eats all wheel events even when the mouse is not over it.
		//To prevent this from breaking other widgets (e.g. combo box), we manually check if the mouse is inside the widget area,
		//and only handle it here if so.
		//Otherwise, try to forward it to the widget under the mouse cursor to get the original behavior.
		if (_container->rect().contains(wheelEvent->position().toPoint()))
		{
			emit WheelEvent(wheelEvent);
		}
		else
		{
			if (auto widget = qApp->widgetAt(wheelEvent->globalPosition().toPoint()); widget)
			{
				QApplication::sendEvent(widget, wheelEvent);
			}
			else
			{
				wheelEvent->ignore();
			}
		}
	}

	default: return QOpenGLWindow::event(event);
	}

	return true;
}

void SceneWidget::initializeGL()
{
	qCDebug(logging::HLAM) << "Initializing OpenGL";

	assert(initializeOpenGLFunctions());

	//TODO: since we're sharing contexts this can probably be done elsewhere to avoid multiple calls
	_scene->Initialize();

	emit CreateDeviceResources();

	qCDebug(logging::HLAM) << "Initialized OpenGL";
}

void SceneWidget::resizeGL(int w, int h)
{
	_scene->UpdateWindowSize(static_cast<unsigned int>(w), static_cast<unsigned int>(h));
}

void SceneWidget::paintGL()
{
	const QSize size{this->size()};

	//Only draw something if the window has a size
	//Otherwise problems could occur when the size is used to determine aspect ratios, viewports, etc
	if (size.isValid())
	{
		//TODO: this is temporary until window sized resources can be decoupled from the scene class
		_scene->UpdateWindowSize(static_cast<unsigned int>(size.width()), static_cast<unsigned int>(size.height()));
		_scene->Draw();
	}
}
}
