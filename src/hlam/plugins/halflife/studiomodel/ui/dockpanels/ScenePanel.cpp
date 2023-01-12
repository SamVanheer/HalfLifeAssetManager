#include "plugins/halflife/studiomodel/StudioModelAsset.hpp"
#include "plugins/halflife/studiomodel/ui/dockpanels/ScenePanel.hpp"
#include "plugins/halflife/studiomodel/ui/dockpanels/objects/BackgroundPanel.hpp"
#include "plugins/halflife/studiomodel/ui/dockpanels/objects/GroundPanel.hpp"
#include "plugins/halflife/studiomodel/ui/dockpanels/objects/ModelPanel.hpp"

namespace studiomodel
{
ScenePanel::ScenePanel(StudioModelAssetProvider* provider)
	: _provider(provider)
{
	_ui.setupUi(this);

	connect(_ui.ObjectList, &QListWidget::currentRowChanged, _ui.ObjectStack, &QStackedWidget::setCurrentIndex);

	AddObject(new ModelPanel(_provider), "Model");
	AddObject(new GroundPanel(_provider), "Ground");
	AddObject(new BackgroundPanel(_provider), "Background");

	//Ensure first row is selected
	_ui.ObjectList->setCurrentRow(0);
}

void ScenePanel::AddObject(QWidget* widget, const QString& label)
{
	_ui.ObjectList->addItem(label);
	_ui.ObjectStack->addWidget(widget);
}

void ScenePanel::OnLayoutDirectionChanged(QBoxLayout::Direction direction)
{
	DockableWidget::OnLayoutDirectionChanged(direction);
}
}
