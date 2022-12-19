#include "entity/HLMVStudioModelEntity.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/dockpanels/SkyLightPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/LightingPanel.hpp"

namespace studiomodel
{
LightingPanel::LightingPanel(StudioModelAssetProvider* provider)
	: _provider(provider)
{
	_ui.setupUi(this);

	connect(_ui.Lights, &QListWidget::currentRowChanged, _ui.LightSettingsContainer, &QStackedWidget::setCurrentIndex);

	AddLight("Sky Light", new SkyLightPanel(_provider));

	_ui.Lights->setCurrentRow(0);
}

LightingPanel::~LightingPanel() = default;

void LightingPanel::AddLight(const QString& name, QWidget* settingsPanel)
{
	_ui.LightSettingsContainer->addWidget(settingsPanel);
	_ui.Lights->addItem(name);
}
}
