#pragma once

#include <memory>

#include <QOpenGLWindow>
#include <QPointer>

namespace graphics
{
class Scene;
}

namespace ui
{
/**
*	@brief Renders a scene to an OpenGL window
*	TODO: rework this so it isn't tied directly to OpenGL (allow D3D or Vulkan backends)
*/
class SceneWidget final : public QOpenGLWindow
{
	Q_OBJECT

public:
	SceneWidget(QWidget* parent = nullptr);
	~SceneWidget();

	QWidget* GetContainer() { return _container; }

	graphics::Scene* GetScene() { return _scene; }

	void SetScene(graphics::Scene* scene);

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
	graphics::Scene* _scene = nullptr;
	QPointer<QObject> _previousFocusObject;
};
}
