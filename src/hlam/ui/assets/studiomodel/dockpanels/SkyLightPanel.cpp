#include <QSignalBlocker>

#include "graphics/Scene.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/dockpanels/SkyLightPanel.hpp"

#include "utility/mathlib.hpp"

namespace studiomodel
{
SkyLightPanel::SkyLightPanel(StudioModelAssetProvider* provider)
	: _provider(provider)
{
	_ui.setupUi(this);

	connect(_provider, &StudioModelAssetProvider::AssetChanged, this, &SkyLightPanel::OnAssetChanged);

	connect(_ui.XAngle, &QDial::valueChanged, this, &SkyLightPanel::OnAnglesChanged);
	connect(_ui.YAngle, &QDial::valueChanged, this, &SkyLightPanel::OnAnglesChanged);

	OnAssetChanged(_provider->GetDummyAsset());
}

SkyLightPanel::~SkyLightPanel() = default;

void SkyLightPanel::OnAssetChanged(StudioModelAsset* asset)
{
	const QSignalBlocker xAngle{_ui.XAngle};
	const QSignalBlocker yAngle{_ui.YAngle};

	const glm::vec3 angles{VectorToAngles(asset->GetScene()->SkyLight.Direction)};

	_ui.XAngle->setValue(angles.x);
	_ui.YAngle->setValue(angles.y);
}

void SkyLightPanel::OnAnglesChanged()
{
	const auto scene = _provider->GetCurrentAsset()->GetScene();
	scene->SkyLight.Direction = AnglesToAimVector({_ui.XAngle->value(), _ui.YAngle->value(), 0});
}
}
