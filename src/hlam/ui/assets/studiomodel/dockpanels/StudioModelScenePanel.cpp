#include "ui/assets/studiomodel/dockpanels/StudioModelScenePanel.hpp"
#include "ui/assets/studiomodel/dockpanels/objects/StudioModelBackgroundPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/objects/StudioModelGroundPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/objects/StudioModelModelPanel.hpp"

namespace ui::assets::studiomodel
{
StudioModelScenePanel::StudioModelScenePanel(StudioModelAsset* asset)
	: _asset(asset)
{
	_ui.setupUi(this);

	connect(_ui.ObjectList, &QListWidget::currentRowChanged, _ui.ObjectStack, &QStackedWidget::setCurrentIndex);

	InitializeUI();
}

void StudioModelScenePanel::InitializeUI()
{
	auto modelPanel = new StudioModelModelPanel(_asset);
	auto groundPanel = new StudioModelGroundPanel(_asset);

	AddObject(modelPanel, "Model");
	AddObject(groundPanel, "Ground");
	AddObject(new StudioModelBackgroundPanel(_asset), "Background");

	//Ensure first row is selected
	_ui.ObjectList->setCurrentRow(0);

	connect(this, &StudioModelScenePanel::LayoutDirectionChanged, modelPanel, &StudioModelModelPanel::OnLayoutDirectionChanged);
	connect(this, &StudioModelScenePanel::LayoutDirectionChanged, groundPanel, &StudioModelGroundPanel::OnLayoutDirectionChanged);
}

void StudioModelScenePanel::AddObject(QWidget* widget, const QString& label)
{
	_ui.ObjectList->addItem(label);
	_ui.ObjectStack->addWidget(widget);
}

void StudioModelScenePanel::OnLayoutDirectionChanged()
{
	emit LayoutDirectionChanged(_ui.MainLayout->direction());
}
}
