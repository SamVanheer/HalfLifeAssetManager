#include <QColorDialog>
#include <QSignalBlocker>

#include "graphics/Scene.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/dockpanels/SkyLightPanel.hpp"
#include "ui/settings/ColorSettings.hpp"

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
	connect(_ui.Color, &QPushButton::clicked, this, &SkyLightPanel::OnSelectColor);

	OnAssetChanged(_provider->GetDummyAsset());
}

SkyLightPanel::~SkyLightPanel() = default;

void SkyLightPanel::SetButtonColor(const QColor& color)
{
	_ui.Color->setStyleSheet(QString{"border: 1px solid black; background-color: %1"}.arg(color.name()));
}

void SkyLightPanel::OnAssetChanged(StudioModelAsset* asset)
{
	const QSignalBlocker xAngle{_ui.XAngle};
	const QSignalBlocker yAngle{_ui.YAngle};

	const glm::vec3 angles{VectorToAngles(asset->GetScene()->SkyLight.Direction)};

	_ui.XAngle->setValue(angles.x);
	_ui.YAngle->setValue(angles.y);

	SetButtonColor(VectorToColor(asset->GetScene()->SkyLight.Color));
}

void SkyLightPanel::OnAnglesChanged()
{
	const auto scene = _provider->GetCurrentAsset()->GetScene();
	scene->SkyLight.Direction = AnglesToAimVector({_ui.XAngle->value(), _ui.YAngle->value(), 0});
}

void SkyLightPanel::OnSelectColor()
{
	const auto asset = _provider->GetCurrentAsset();

	const auto color = QColorDialog::getColor(VectorToColor(asset->GetScene()->SkyLight.Color), this);

	if (color.isValid())
	{
		asset->GetScene()->SkyLight.Color = ColorToVector(color);
		SetButtonColor(color);
	}
}
}
