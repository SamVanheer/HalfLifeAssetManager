#include <QBoxLayout>

#include "ui/SceneWidget.hpp"
#include "ui/assets/studiomodel/StudioModelEditWidget.hpp"

#include "ui/assets/studiomodel/dockpanels/StudioModelDisplayPanel.hpp"

namespace ui::assets::studiomodel
{
StudioModelEditWidget::StudioModelEditWidget(StudioModelAsset* asset, QWidget* parent)
	: QWidget(parent)
	, _asset(asset)
{
	_sceneWidget = new SceneWidget(this);

	_dockPanels = new QTabWidget(this);

	_dockPanels->addTab(new StudioModelDisplayPanel(), "Model Display");

	auto layout = new QVBoxLayout(this);

	layout->setContentsMargins(0, 0, 0, 0);

	layout->addWidget(_sceneWidget->GetContainer(), 1);

	layout->addWidget(_dockPanels);

	setLayout(layout);
}

StudioModelEditWidget::~StudioModelEditWidget() = default;
}
