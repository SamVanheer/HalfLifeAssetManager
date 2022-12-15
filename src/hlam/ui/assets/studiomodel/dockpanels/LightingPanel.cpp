#include "entity/HLMVStudioModelEntity.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/dockpanels/SkyLightPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/LightingPanel.hpp"

namespace ui::assets::studiomodel
{
LightingPanel::LightingPanel(StudioModelAsset* asset)
	: _asset(asset)
{
	_ui.setupUi(this);

	connect(_ui.Lights, &QListWidget::currentRowChanged, _ui.LightSettingsContainer, &QStackedWidget::setCurrentIndex);

	AddLight("Sky Light", new SkyLightPanel(_asset));

	_ui.Lights->setCurrentRow(0);
}

LightingPanel::~LightingPanel() = default;

void LightingPanel::AddLight(const QString& name, QWidget* settingsPanel)
{
	_ui.LightSettingsContainer->addWidget(settingsPanel);
	_ui.Lights->addItem(name);
}
}