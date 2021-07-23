#pragma once

#include <memory>

#include <QWidget>

#include "ui/assets/studiomodel/StudioModelAsset.hpp"

class QMainWindow;

namespace graphics
{
class Scene;
}

namespace ui
{
class EditorContext;
class SceneWidget;

namespace camera_operators
{
class CameraOperator;
class CamerasPanel;
}

namespace assets::studiomodel
{
class StudioModelAsset;
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

private:
	StudioModelAsset* const _asset;

	QMainWindow* _window;

	QWidget* _centralWidget;

	SceneWidget* _sceneWidget;

	camera_operators::CamerasPanel* _camerasPanel;

	Timeline* _timeline;
};
}
}
