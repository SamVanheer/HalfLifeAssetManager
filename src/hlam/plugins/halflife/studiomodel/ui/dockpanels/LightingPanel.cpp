#include "entity/HLMVStudioModelEntity.hpp"

#include "plugins/halflife/studiomodel/StudioModelAsset.hpp"
#include "plugins/halflife/studiomodel/ui/dockpanels/SkyLightPanel.hpp"
#include "plugins/halflife/studiomodel/ui/dockpanels/LightingPanel.hpp"

namespace studiomodel
{
LightingPanel::LightingPanel(StudioModelAssetProvider* provider)
	: _provider(provider)
{
	_ui.setupUi(this);

	connect(_ui.Lights, &QListWidget::currentRowChanged, _ui.LightSettingsContainer, &QStackedWidget::setCurrentIndex);

	auto skyLightPanel = new SkyLightPanel(_provider);

	AddLight("Sky Light", skyLightPanel);

	_ui.Lights->setCurrentRow(0);

	connect(this, &LightingPanel::LayoutDirectionChanged, skyLightPanel, &SkyLightPanel::OnLayoutDirectionChanged);
}

LightingPanel::~LightingPanel() = default;

void LightingPanel::OnLayoutDirectionChanged(QBoxLayout::Direction direction)
{
	DockableWidget::OnLayoutDirectionChanged(direction);

	emit LayoutDirectionChanged(direction);
}

void LightingPanel::AddLight(const QString& name, QWidget* settingsPanel)
{
	_ui.LightSettingsContainer->addWidget(settingsPanel);
	_ui.Lights->addItem(name);
}
}
