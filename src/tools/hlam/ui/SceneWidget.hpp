#pragma once

#include <memory>

#include <qopenglfunctions_1_1.h>
#include <QOpenGLWindow>
#include <QPointer>

#include "graphics/IGraphicsContext.hpp"

namespace graphics
{
class Scene;
}

namespace ui
{
class OpenGLGraphicsContext final : public graphics::IGraphicsContext
{
public:
	OpenGLGraphicsContext(QOpenGLWindow* window)
		: _window(window)
	{
	}

	void Begin() override
	{
		_window->makeCurrent();
	}

	void End() override
	{
		_window->doneCurrent();
	}

private:
	QOpenGLWindow* const _window;
};

/**
*	@brief Renders a scene to an OpenGL window
*	TODO: rework this so it isn't tied directly to OpenGL (allow D3D or Vulkan backends)
*/
class SceneWidget final : public QOpenGLWindow, protected QOpenGLFunctions_1_1
{
	Q_OBJECT

public:
	SceneWidget(graphics::Scene* scene, QWidget* parent = nullptr);
	~SceneWidget();

	QWidget* GetContainer() { return _container; }

	graphics::Scene* GetScene() { return _scene; }

signals:
	void MouseEvent(QMouseEvent* event);

	void WheelEvent(QWheelEvent* event);

private slots:
	void OnFocusObjectChanged(QObject* object)
	{
		if (object != this)
		{
			_previousFocusObject = object;
		}
	}

protected:
	bool event(QEvent* event) override final;

	void initializeGL() override;
	void resizeGL(int w, int h) override;
	void paintGL() override;

private:
	QWidget* const _container;
	graphics::Scene* const _scene;
	QPointer<QObject> _previousFocusObject;
};
}
