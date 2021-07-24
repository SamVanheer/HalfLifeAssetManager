#pragma once

#include <memory>

#include <QWidget>

#include "ui/assets/studiomodel/StudioModelAsset.hpp"

class QMainWindow;
class QStackedWidget;
class QTabBar;

namespace graphics
{
class Scene;
}

namespace ui
{
class EditorContext;
class SceneWidget;
class TextureWidget;

namespace camera_operators
{
class CameraOperator;
class CamerasPanel;
}

namespace assets::studiomodel
{
class StudioModelAsset;
class StudioModelTexturesPanel;
class Timeline;

class StudioModelEditWidget final : public QWidget
{
	Q_OBJECT

public:
	StudioModelEditWidget(EditorContext* editorContext, StudioModelAsset* asset, QWidget* parent = nullptr);
	~StudioModelEditWidget();

	StudioModelAsset* GetAsset() const { return _asset; }

	SceneWidget* GetSceneWidget() const { return _sceneWidget; }

private slots:
	void OnDockLocationChanged(Qt::DockWidgetArea area);

	void OnAssetCameraChanged(camera_operators::CameraOperator* previous, camera_operators::CameraOperator* current);

	void OnCameraChanged(int index);

	void OnTexturesDockVisibilityChanged(bool visible);

	void SetTextureBackgroundColor();

	void OnTextureViewChanged();

private:
	EditorContext* const _editorContext;
	StudioModelAsset* const _asset;

	QMainWindow* _window;

	QWidget* _centralWidget;

	QStackedWidget* _viewWidget;

	SceneWidget* _sceneWidget;

	TextureWidget* _textureWidget;

	QTabBar* _viewSelectionWidget;

	camera_operators::CamerasPanel* _camerasPanel;
	StudioModelTexturesPanel* _texturesPanel;

	Timeline* _timeline;
};
}
}
