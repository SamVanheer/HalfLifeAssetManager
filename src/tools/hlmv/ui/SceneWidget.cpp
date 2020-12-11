#include <cassert>

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
	
	//TODO: need to rework this to use the shared context instead since this can tie a context to a destroyed scene widget used in fullscreen mode
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
