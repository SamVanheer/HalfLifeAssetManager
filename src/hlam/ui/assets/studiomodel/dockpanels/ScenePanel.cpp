#include "ui/assets/studiomodel/dockpanels/ScenePanel.hpp"
#include "ui/assets/studiomodel/dockpanels/objects/BackgroundPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/objects/GroundPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/objects/ModelPanel.hpp"

namespace ui::assets::studiomodel
{
ScenePanel::ScenePanel(StudioModelAsset* asset)
	: _asset(asset)
{
	_ui.setupUi(this);

	connect(_ui.ObjectList, &QListWidget::currentRowChanged, _ui.ObjectStack, &QStackedWidget::setCurrentIndex);

	InitializeUI();
}

void ScenePanel::InitializeUI()
{
	auto modelPanel = new ModelPanel(_asset);
	auto groundPanel = new GroundPanel(_asset);

	AddObject(modelPanel, "Model");
	AddObject(groundPanel, "Ground");
	AddObject(new BackgroundPanel(_asset), "Background");

	//Ensure first row is selected
	_ui.ObjectList->setCurrentRow(0);

	connect(this, &ScenePanel::LayoutDirectionChanged, modelPanel, &ModelPanel::OnLayoutDirectionChanged);
	connect(this, &ScenePanel::LayoutDirectionChanged, groundPanel, &GroundPanel::OnLayoutDirectionChanged);
}

void ScenePanel::AddObject(QWidget* widget, const QString& label)
{
	_ui.ObjectList->addItem(label);
	_ui.ObjectStack->addWidget(widget);
}

void ScenePanel::OnLayoutDirectionChanged(QBoxLayout::Direction direction)
{
	DockableWidget::OnLayoutDirectionChanged(direction);

	emit LayoutDirectionChanged(direction);
}
}
