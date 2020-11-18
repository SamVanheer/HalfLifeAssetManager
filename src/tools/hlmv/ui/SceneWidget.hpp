#pragma once

#include <QOpenGLWindow>

namespace ui
{
/**
*	@brief Renders a scene to an OpenGL window
*	TODO: rework this so it isn't tied directly to OpenGL (allow D3D or Vulkan backends)
*/
class SceneWidget final : public QOpenGLWindow
{
public:
	SceneWidget(QWidget* parent = nullptr);
	~SceneWidget();

	QWidget* GetContainer() { return _container; }

private:
	QWidget* _container;
};
}
