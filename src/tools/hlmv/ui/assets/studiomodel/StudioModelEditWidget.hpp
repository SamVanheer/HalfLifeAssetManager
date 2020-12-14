#pragma once

#include <memory>

#include <QTabWidget>
#include <QWidget>

#include "ui/assets/studiomodel/StudioModelAsset.hpp"

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
class Timeline;

class StudioModelEditWidget final : public QWidget
{
	Q_OBJECT

public:
	StudioModelEditWidget(EditorContext* editorContext, StudioModelAsset* asset, QWidget* parent = nullptr);
	~StudioModelEditWidget();

	StudioModelAsset* GetAsset() const { return _asset; }

	SceneWidget* GetSceneWidget() const { return _sceneWidget; }

signals:
	void DockPanelChanged(QWidget* current, QWidget* previous);

private slots:
	void OnTick();

	void OnTabChanged(int index);

private:
	StudioModelAsset* const _asset;

	SceneWidget* _sceneWidget;

	QWidget* _controlAreaWidget;

	QTabWidget* _dockPanels;

	QWidget* _currentTab{};

	Timeline* _timeline;
};
}
}
