#pragma once

#include <QTabWidget>
#include <QWidget>

namespace ui
{
class SceneWidget;

namespace assets::studiomodel
{
class StudioModelAsset;

class StudioModelEditWidget final : public QWidget
{
public:
	StudioModelEditWidget(StudioModelAsset* asset, QWidget* parent = nullptr);
	~StudioModelEditWidget();

private:
	StudioModelAsset* const _asset;

	SceneWidget* _sceneWidget;

	QTabWidget* _dockPanels;
};
}
}
