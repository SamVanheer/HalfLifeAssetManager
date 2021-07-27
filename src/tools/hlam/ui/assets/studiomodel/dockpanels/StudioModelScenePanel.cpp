#include "ui/assets/studiomodel/dockpanels/StudioModelScenePanel.hpp"
#include "ui/assets/studiomodel/dockpanels/objects/StudioModelModelPanel.hpp"

namespace ui::assets::studiomodel
{
StudioModelScenePanel::StudioModelScenePanel(StudioModelAsset* asset, QWidget* parent)
	: QWidget(parent)
	, _asset(asset)
{
	_ui.setupUi(this);

	connect(_ui.ObjectList, &QListWidget::currentRowChanged, _ui.ObjectStack, &QStackedWidget::setCurrentIndex);

	InitializeUI();
}

void StudioModelScenePanel::InitializeUI()
{
	AddObject(new StudioModelModelPanel(_asset, this), "Model");

	//Ensure first row is selected
	_ui.ObjectList->setCurrentRow(0);
}

void StudioModelScenePanel::AddObject(QWidget* widget, const QString& label)
{
	_ui.ObjectList->addItem(label);
	_ui.ObjectStack->addWidget(widget);
}
}
