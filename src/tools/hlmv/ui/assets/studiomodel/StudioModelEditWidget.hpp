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
class SceneWidget;

namespace assets::studiomodel
{
class StudioModelAsset;

class StudioModelEditWidget final : public QWidget
{
public:
	StudioModelEditWidget(StudioModelAsset* asset, QWidget* parent = nullptr);
	~StudioModelEditWidget();

private slots:
	void OnBackgroundColorChanged(QColor color);

private:
	StudioModelContext* const _context;

	StudioModelAsset* const _asset;

	SceneWidget* _sceneWidget;

	QTabWidget* _dockPanels;

	std::unique_ptr<graphics::Scene> _scene;
};
}
}
