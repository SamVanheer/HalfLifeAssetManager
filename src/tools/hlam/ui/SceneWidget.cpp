#include <cassert>

#include <QApplication>
#include <QWheelEvent>
#include <QWidget>

#include "graphics/Scene.hpp"
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
}

SceneWidget::~SceneWidget()
{
	makeCurrent();

	_scene->Shutdown();

	doneCurrent();
}

void SceneWidget::wheelEvent(QWheelEvent* event)
{
	//Ugly hack: when this window has focus it eats all wheel events even when the mouse is not over it.
	//To prevent this from breaking other widgets (e.g. combo box), we manually check if the mouse is inside the widget area,
	//and only handle it here if so.
	//Otherwise, try to forward it to the widget under the mouse cursor to get the original behavior.
	if (_container->rect().contains(event->position().toPoint()))
	{
		emit WheelEvent(event);
	}
	else
	{
		if (auto widget = qApp->widgetAt(event->globalPosition().toPoint()); widget)
		{
			QApplication::sendEvent(widget, event);
		}
		else
		{
			event->ignore();
		}
	}
}

void SceneWidget::initializeGL()
{
	//TODO: since we're sharing contexts this can probably be done elsewhere to avoid multiple calls
	_scene->Initialize();

	emit CreateDeviceResources();
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
