#pragma once


#include <QVector>
#include <QWidget>

#include "ui_StudioModelEditWidget.h"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"

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
class StudioModelView;

class StudioModelEditWidget final : public QWidget
{
	Q_OBJECT

public:
	StudioModelEditWidget(EditorContext* editorContext, StudioModelAsset* asset);
	~StudioModelEditWidget();

	StudioModelAsset* GetAsset() const { return _asset; }

	SceneWidget* GetSceneWidget() const { return _sceneWidget; }

	const QVector<QDockWidget*>& GetDockWidgets() const { return _dockWidgets; }

	int GetSceneIndex() const;

signals:
	void SceneIndexChanged(int index);

	void PoseChanged(Pose pose);

public slots:
	void SetSceneIndex(int index);

private slots:
	void OnDockLocationChanged(Qt::DockWidgetArea area);

	void OnDockVisibilityChanged(bool visible);

private:
	EditorContext* const _editorContext;
	StudioModelAsset* const _asset;

	Ui_StudioModelEditWidget _ui;

	StudioModelView* _view;

	SceneWidget* _sceneWidget;

	//Stored separately to maintain list order
	QVector<QDockWidget*> _dockWidgets;
};
}
