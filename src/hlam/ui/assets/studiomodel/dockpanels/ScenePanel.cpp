#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/dockpanels/ScenePanel.hpp"
#include "ui/assets/studiomodel/dockpanels/objects/BackgroundPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/objects/GroundPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/objects/ModelPanel.hpp"

namespace studiomodel
{
ScenePanel::ScenePanel(StudioModelAssetProvider* provider)
	: _provider(provider)
{
	_ui.setupUi(this);

	connect(_ui.ObjectList, &QListWidget::currentRowChanged, _ui.ObjectStack, &QStackedWidget::setCurrentIndex);

	auto modelPanel = new ModelPanel(_provider);
	auto groundPanel = new GroundPanel(_provider);

	AddObject(modelPanel, "Model");
	AddObject(groundPanel, "Ground");
	AddObject(new BackgroundPanel(_provider), "Background");

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
