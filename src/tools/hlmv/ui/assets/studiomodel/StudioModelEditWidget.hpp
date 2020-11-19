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

namespace assets::studiomodel
{
class StudioModelAsset;

class StudioModelEditWidget final : public QWidget
{
public:
	StudioModelEditWidget(EditorUIContext* editorContext, StudioModelAsset* asset, QWidget* parent = nullptr);
	~StudioModelEditWidget();

public slots:
	void OnTick();

private:
	StudioModelAsset* const _asset;
	const std::unique_ptr<graphics::Scene> _scene;

	StudioModelContext* const _context;

	SceneWidget* _sceneWidget;

	QTabWidget* _dockPanels;
};
}
}
