#include <QBoxLayout>

#include "graphics/Scene.hpp"

#include "ui/SceneWidget.hpp"
#include "ui/assets/studiomodel/StudioModelEditWidget.hpp"

#include "ui/assets/studiomodel/dockpanels/StudioModelDisplayPanel.hpp"

namespace ui::assets::studiomodel
{
StudioModelEditWidget::StudioModelEditWidget(StudioModelAsset* asset, QWidget* parent)
	: QWidget(parent)
	, _context(new StudioModelContext(asset, this))
	, _asset(asset)
{
	connect(_context, &StudioModelContext::BackgroundColorChanged, this, &StudioModelEditWidget::OnBackgroundColorChanged);

	_scene = std::make_unique<graphics::Scene>();

	_sceneWidget = new SceneWidget(_scene.get(), this);

	_dockPanels = new QTabWidget(this);

	_dockPanels->setStyleSheet("QTabWidget::pane { padding: 0px; }");

	_dockPanels->addTab(new StudioModelDisplayPanel(_context), "Model Display");

	auto layout = new QVBoxLayout(this);

	layout->setContentsMargins(0, 0, 0, 0);

	layout->addWidget(_sceneWidget->GetContainer(), 1);

	layout->addWidget(_dockPanels);

	setLayout(layout);

	//TODO: need to initialize the background color to its default value here, as specified in the options dialog
	_context->SetBackgroundColor({63, 127, 127});
}

StudioModelEditWidget::~StudioModelEditWidget() = default;

void StudioModelEditWidget::OnBackgroundColorChanged(QColor color)
{
	_scene->SetBackgroundColor({color.redF(), color.greenF(), color.blueF()});
}
}
