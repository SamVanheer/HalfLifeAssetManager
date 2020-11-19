#include <cassert>

#include <QWidget>

#include "graphics/Scene.hpp"
#include "ui/SceneWidget.hpp"

namespace ui
{
//TODO: implement rendering logic
SceneWidget::SceneWidget(graphics::Scene* scene, QWidget* parent)
	: QOpenGLWindow()
	, _scene(scene)
{
	assert(nullptr != scene);

	_container = QWidget::createWindowContainer(this, parent);
}

SceneWidget::~SceneWidget() = default;

void SceneWidget::initializeGL()
{
	//TODO: initialize
}

void SceneWidget::resizeGL(int w, int h)
{
	//TODO: notify listeners to recreate window sized resources
}

void SceneWidget::paintGL()
{
	_scene->Draw();
}
}
