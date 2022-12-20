#pragma once


#include <QVector>
#include <QWidget>

#include "ui_StudioModelEditWidget.h"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"

class CamerasPanel;
class QDockWidget;

namespace graphics
{
class Scene;
}

class EditorContext;
class SceneWidget;

namespace studiomodel
{
class StudioModelAsset;
class StudioModelAssetProvider;
class StudioModelView;
class Timeline;

class StudioModelEditWidget final : public QWidget
{
	Q_OBJECT

public:
	StudioModelEditWidget(EditorContext* editorContext, StudioModelAssetProvider* provider);
	~StudioModelEditWidget();

	SceneWidget* GetSceneWidget() const { return _sceneWidget; }

	void RecreateSceneWidget();

	const QVector<QDockWidget*>& GetDockWidgets() const { return _dockWidgets; }

	int GetSceneIndex() const;

signals:
	void SceneIndexChanged(int index);

	void PoseChanged(Pose pose);

public slots:
	void SetAsset(StudioModelAsset* asset);

	void SetSceneIndex(int index);

private slots:
	void OnDockLocationChanged(Qt::DockWidgetArea area);

	void OnDockVisibilityChanged(bool visible);

private:
	EditorContext* const _editorContext;
	StudioModelAssetProvider* const _provider;

	Ui_StudioModelEditWidget _ui;

	StudioModelView* _view;

	SceneWidget* _sceneWidget{};

	Timeline* _timeline;

	//Stored separately to maintain list order
	QVector<QDockWidget*> _dockWidgets;

	CamerasPanel* _camerasPanel{};
};
}
