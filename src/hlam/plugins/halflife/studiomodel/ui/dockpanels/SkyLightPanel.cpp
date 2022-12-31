#include <QColorDialog>
#include <QSignalBlocker>

#include "graphics/Scene.hpp"

#include "settings/ColorSettings.hpp"

#include "plugins/halflife/studiomodel/StudioModelAsset.hpp"
#include "plugins/halflife/studiomodel/ui/StudioModelData.hpp"
#include "plugins/halflife/studiomodel/ui/dockpanels/SkyLightPanel.hpp"

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
	connect(_ui.Ambient, qOverload<int>(&QSpinBox::valueChanged), this, &SkyLightPanel::OnAmbientChanged);
	connect(_ui.Shade, qOverload<int>(&QSpinBox::valueChanged), this, &SkyLightPanel::OnShadeChanged);

	OnAssetChanged(_provider->GetDummyAsset());
}

SkyLightPanel::~SkyLightPanel() = default;

void SkyLightPanel::SetButtonColor(const QColor& color)
{
	_ui.Color->setStyleSheet(QString{"border: 1px solid black; background-color: %1"}.arg(color.name()));
}

void SkyLightPanel::OnLayoutDirectionChanged(QBoxLayout::Direction direction)
{
	_ui.MainLayout->setDirection(direction);
}

void SkyLightPanel::OnAssetChanged(StudioModelAsset* asset)
{
	if (_modelData)
	{
		_modelData->DisconnectFromAll(this);
	}

	_modelData = asset->GetModelData();

	const auto lambda = [this]()
	{
		const QSignalBlocker xAngle{_ui.XAngle};
		const QSignalBlocker yAngle{_ui.YAngle};
		const QSignalBlocker ambient{_ui.Ambient};
		const QSignalBlocker shade{_ui.Shade};

		auto scene = _provider->GetCurrentAsset()->GetScene();

		const glm::vec3 angles{VectorToAngles(scene->SkyLight.Direction)};

		_ui.XAngle->setValue(angles.x);
		_ui.YAngle->setValue(angles.y);

		SetButtonColor(VectorToColor(scene->SkyLight.Color));

		_ui.Ambient->setValue(scene->SkyLight.Ambient);
		_ui.Shade->setValue(scene->SkyLight.Shade);
	};

	lambda();

	connect(_modelData, &StudioModelData::SkyLightChanged, this, lambda);
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

void SkyLightPanel::OnAmbientChanged(int value)
{
	_provider->GetCurrentAsset()->GetScene()->SkyLight.Ambient = value;
}

void SkyLightPanel::OnShadeChanged(int value)
{
	_provider->GetCurrentAsset()->GetScene()->SkyLight.Shade = value;
}
}
