#pragma once

#include <memory>

#include <QTabWidget>
#include <QWidget>

#include "ui/IInputSink.hpp"
#include "ui/assets/studiomodel/StudioModelContext.hpp"

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
}

namespace assets::studiomodel
{
class StudioModelAsset;
class Timeline;

class StudioModelEditWidget final : public QWidget, public IInputSink
{
	Q_OBJECT

public:
	StudioModelEditWidget(EditorContext* editorContext, StudioModelAsset* asset, QWidget* parent = nullptr);
	~StudioModelEditWidget();

	void OnMouseEvent(QMouseEvent* event) override;

signals:
	void DockPanelChanged(QWidget* current, QWidget* previous);

private slots:
	void OnTick();

	void OnSceneWidgetMouseEvent(QMouseEvent* event);

	void OnFloorLengthChanged(int length);

	void OnTabChanged(int index);

private:
	StudioModelAsset* const _asset;
	const std::unique_ptr<graphics::Scene> _scene;

	StudioModelContext* const _context;

	SceneWidget* _sceneWidget;

	QWidget* _controlAreaWidget;

	QTabWidget* _dockPanels;

	QWidget* _currentTab{};

	Timeline* _timeline;

	//TODO: temporary; will need to be set up somewhere else eventually
	std::unique_ptr<camera_operators::CameraOperator> _cameraOperator;
};
}
}
