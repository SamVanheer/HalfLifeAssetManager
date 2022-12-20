#include <QApplication>
#include <QSurfaceFormat>
#include <QWheelEvent>
#include <QWidget>

#include "graphics/Scene.hpp"
#include "graphics/SceneContext.hpp"
#include "ui/EditorContext.hpp"
#include "ui/SceneWidget.hpp"
#include "ui/settings/GeneralSettings.hpp"

SceneWidget::SceneWidget(EditorContext* editorContext,
	QOpenGLFunctions_1_1* openglFunctions, graphics::TextureLoader* textureLoader,
	QWidget* parent)
	: QOpenGLWindow()
	, _container(QWidget::createWindowContainer(this, parent))
	, _sceneContext(std::make_unique<graphics::SceneContext>(openglFunctions, textureLoader))
{
	// It's safe to modify this since it doesn't affect the underlying window, only the FBO used for MSAA.
	auto format = QSurfaceFormat::defaultFormat();

	const int msaaLevel = GeneralSettings::GetMSAALevel(*editorContext->GetSettings());

	format.setSamples(msaaLevel != -1 ? 1 << msaaLevel : -1);

	setFormat(format);

	_container->setFocusPolicy(Qt::FocusPolicy::WheelFocus);

	connect(this, &SceneWidget::frameSwapped, this, qOverload<>(&SceneWidget::update));

	connect(qGuiApp, &QGuiApplication::focusObjectChanged, this, &SceneWidget::OnFocusObjectChanged);

	_previousFocusObject = qGuiApp->focusObject();
}

SceneWidget::~SceneWidget() = default;

void SceneWidget::SetScene(graphics::Scene* scene)
{
	_scene = scene;

	if (_scene)
	{
		const QSize size{this->size()};
		_scene->UpdateWindowSize(static_cast<unsigned int>(size.width()), static_cast<unsigned int>(size.height()));
	}
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
	// Nothing. Initialization is done using the offscreen context.
}

void SceneWidget::resizeGL(int w, int h)
{
	_sceneContext->WindowWidth = w;
	_sceneContext->WindowHeight = h;

	if (_scene)
	{
		_scene->UpdateWindowSize(static_cast<unsigned int>(w), static_cast<unsigned int>(h));
	}
}

void SceneWidget::paintGL()
{
	const QSize size{this->size()};

	//Only draw something if the window has a size
	//Otherwise problems could occur when the size is used to determine aspect ratios, viewports, etc
	if (size.isValid())
	{
		if (_scene)
		{
			//TODO: this is temporary until window sized resources can be decoupled from the scene class
			_scene->UpdateWindowSize(static_cast<unsigned int>(size.width()), static_cast<unsigned int>(size.height()));
			_scene->Draw(*_sceneContext);
		}
	}
}
