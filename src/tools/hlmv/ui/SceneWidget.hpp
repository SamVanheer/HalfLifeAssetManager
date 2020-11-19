#pragma once

#include <QOpenGLWindow>

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
	SceneWidget(graphics::Scene* scene, QWidget* parent = nullptr);
	~SceneWidget();

	QWidget* GetContainer() { return _container; }

	graphics::Scene* GetScene() { return _scene; }

signals:
	void MouseEvent(QMouseEvent* event);

protected:
	void mousePressEvent(QMouseEvent* event) override final
	{
		emit MouseEvent(event);
	}

	void mouseReleaseEvent(QMouseEvent* event) override final
	{
		emit MouseEvent(event);
	}

	void mouseMoveEvent(QMouseEvent* event) override final
	{
		emit MouseEvent(event);
	}

	void initializeGL() override;
	void resizeGL(int w, int h) override;
	void paintGL() override;

private:
	QWidget* const _container;
	graphics::Scene* const _scene;
};
}
