#pragma once


#include <QList>
#include <QWidget>

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui_StudioModelEditWidget.h"

class QDockWidget;

namespace graphics
{
class Scene;
}

namespace ui
{
class EditorContext;
class SceneWidget;

namespace assets::studiomodel
{
class StudioModelAsset;
class StudioModelView;

class StudioModelEditWidget final : public QWidget
{
	Q_OBJECT

public:
	StudioModelEditWidget(EditorContext* editorContext, StudioModelAsset* asset);
	~StudioModelEditWidget();

	StudioModelAsset* GetAsset() const { return _asset; }

	SceneWidget* GetSceneWidget() const { return _sceneWidget; }

	QList<QDockWidget*> GetDockWidgets() const { return _dockWidgets; }

	graphics::Scene* GetCurrentScene();

private slots:
	void SetSceneIndex(int index);

	void OnDockLocationChanged(Qt::DockWidgetArea area);

	void OnTexturesDockVisibilityChanged(bool visible);

private:
	EditorContext* const _editorContext;
	StudioModelAsset* const _asset;

	Ui_StudioModelEditWidget _ui;

	StudioModelView* _view;

	SceneWidget* _sceneWidget;

	//Stored separately to maintain list order
	QList<QDockWidget*> _dockWidgets;
};
}
}
