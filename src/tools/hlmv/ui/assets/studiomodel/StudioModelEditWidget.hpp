#pragma once

#include <memory>

#include <QTabWidget>
#include <QWidget>

namespace graphics
{
class Scene;
}

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

	std::unique_ptr<graphics::Scene> _scene;
};
}
}
