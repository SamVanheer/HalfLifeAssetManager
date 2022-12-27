#pragma once


#include <QVector>
#include <QWidget>

#include "ui_StudioModelEditWidget.h"

#include "plugins/halflife/studiomodel/StudioModelAsset.hpp"

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

	QByteArray SaveState();

	void RestoreState(const QByteArray& state);

	void ResetToInitialState();

	void AttachSceneWidget();

	void DetachSceneWidget();

	bool IsControlsBarVisible() const;

	void SetControlsBarVisible(bool state);

	bool IsTimelineVisible() const;
	
	void SetTimelineVisible(bool state);

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

	Timeline* _timeline;

	//Stored separately to maintain list order
	QVector<QDockWidget*> _dockWidgets;
	QByteArray _initialState;

	CamerasPanel* _camerasPanel{};
};
}
