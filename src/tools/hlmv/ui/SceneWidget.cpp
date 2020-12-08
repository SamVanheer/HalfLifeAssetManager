#include <cassert>

#include <QWidget>

#include "graphics/Scene.hpp"
#include "ui/SceneWidget.hpp"

namespace ui
{
SceneWidget::SceneWidget(std::unique_ptr<graphics::Scene>&& scene, QWidget* parent)
	: QOpenGLWindow()
	, _container(QWidget::createWindowContainer(this, parent))
	, _scene(std::move(scene))
{
	assert(nullptr != _scene);

	_scene->SetGraphicsContext(std::make_unique<OpenGLGraphicsContext>(this));
}

SceneWidget::~SceneWidget()
{
	makeCurrent();

	_scene->Shutdown();

	doneCurrent();
}

void SceneWidget::initializeGL()
{
	_scene->Initialize();

	emit CreateDeviceResources();
}

void SceneWidget::resizeGL(int w, int h)
{
	_scene->UpdateWindowSize(static_cast<unsigned int>(w), static_cast<unsigned int>(h));
}

void SceneWidget::paintGL()
{
	_scene->Draw();
}
}
