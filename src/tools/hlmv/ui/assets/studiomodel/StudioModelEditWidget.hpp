#pragma once

#include <memory>

#include <QTabWidget>
#include <QWidget>

#include "ui/assets/studiomodel/StudioModelContext.hpp"

namespace graphics
{
class Scene;
}

namespace ui
{
class EditorUIContext;
class SceneWidget;

namespace camera_operators
{
class CameraOperator;
}

namespace assets::studiomodel
{
class StudioModelAsset;

class StudioModelEditWidget final : public QWidget
{
public:
	StudioModelEditWidget(EditorUIContext* editorContext, StudioModelAsset* asset, QWidget* parent = nullptr);
	~StudioModelEditWidget();

private slots:
	void OnTick();

	void OnMouseEvent(QMouseEvent* event);

private:
	StudioModelAsset* const _asset;
	const std::unique_ptr<graphics::Scene> _scene;

	StudioModelContext* const _context;

	SceneWidget* _sceneWidget;

	QTabWidget* _dockPanels;

	//TODO: temporary; will need to be set up somewhere else eventually
	std::unique_ptr<camera_operators::CameraOperator> _cameraOperator;
};
}
}
