#pragma once

#include <memory>

#include <QOpenGLWindow>
#include <QPointer>

class AssetManager;
class QOpenGLFunctions_1_1;

namespace graphics
{
class Scene;
class SceneContext;
class TextureLoader;
}

/**
*	@brief Renders a scene to an OpenGL window
*	TODO: rework this so it isn't tied directly to OpenGL (allow D3D or Vulkan backends)
*/
class SceneWidget final : public QOpenGLWindow
{
	Q_OBJECT

public:
	SceneWidget(
		AssetManager* application,
		QOpenGLFunctions_1_1* openglFunctions, graphics::TextureLoader* textureLoader);
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
	const std::unique_ptr<graphics::SceneContext> _sceneContext;
	graphics::Scene* _scene{};
	QPointer<QObject> _previousFocusObject;
};
