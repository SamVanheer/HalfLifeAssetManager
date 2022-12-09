#include <QSpinBox>
#include <QGridLayout>
#include <QLabel>

#include "entity/HLMVStudioModelEntity.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/dockpanels/SkyLightPanel.hpp"

#include "utility/mathlib.hpp"

namespace ui::assets::studiomodel
{
SkyLightPanel::SkyLightPanel(StudioModelAsset* asset)
	: _asset(asset)
{
	_ui.setupUi(this);

	const glm::vec3 lightVector{_asset->GetStudioModelRenderer()->GetLightVector()};

	const glm::vec3 angles{VectorToAngles(lightVector)};

	_ui.XAngle->setValue(angles.x);
	_ui.YAngle->setValue(angles.y);

	connect(_ui.XAngle, &QDial::valueChanged, this, &SkyLightPanel::OnAnglesChanged);
	connect(_ui.YAngle, &QDial::valueChanged, this, &SkyLightPanel::OnAnglesChanged);
}

SkyLightPanel::~SkyLightPanel() = default;

void SkyLightPanel::OnAnglesChanged()
{
	const glm::vec3 lightVector{AnglesToAimVector({_ui.XAngle->value(), _ui.YAngle->value(), 0})};

	_asset->GetStudioModelRenderer()->SetLightVector(lightVector);
}
}
