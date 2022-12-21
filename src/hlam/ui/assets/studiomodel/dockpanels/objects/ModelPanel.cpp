#include <QSignalBlocker>

#include "entity/HLMVStudioModelEntity.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/dockpanels/objects/ModelPanel.hpp"

namespace studiomodel
{
ModelPanel::ModelPanel(StudioModelAssetProvider* provider)
	: _provider(provider)
{
	_ui.setupUi(this);

	_ui.Origin->SetRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
	_ui.Origin->SetDecimals(6);

	connect(_provider, &StudioModelAssetProvider::AssetChanged, this, &ModelPanel::OnAssetChanged);

	connect(_ui.Origin, &qt::widgets::Vector3Edit::ValueChanged, this, &ModelPanel::OnOriginChanged);
	connect(_ui.CenterOnWorldOrigin, &QPushButton::clicked, this, &ModelPanel::OnCenterOnWorldOrigin);
	connect(_ui.AlignOnGround, &QPushButton::clicked, this, &ModelPanel::OnAlignOnGround);
}

void ModelPanel::OnLayoutDirectionChanged(QBoxLayout::Direction direction)
{
	_ui.MainLayout->setDirection(direction);
}

void ModelPanel::OnAssetChanged(StudioModelAsset* asset)
{
	const QSignalBlocker originBlocker{_ui.Origin};

	auto entity = asset->GetEntity();

	_ui.Origin->SetValue(entity->GetOrigin());
}

void ModelPanel::OnOriginChanged()
{
	_provider->GetCurrentAsset()->GetEntity()->SetOrigin(_ui.Origin->GetValue());
}

void ModelPanel::OnCenterOnWorldOrigin()
{
	auto entity = _provider->GetCurrentAsset()->GetEntity();

	entity->SetOrigin({0, 0, 0});

	const QSignalBlocker blocker{_ui.Origin};
	_ui.Origin->SetValue(entity->GetOrigin());
}

void ModelPanel::OnAlignOnGround()
{
	auto entity = _provider->GetCurrentAsset()->GetEntity();

	entity->AlignOnGround();

	const QSignalBlocker blocker{_ui.Origin};
	_ui.Origin->SetValue(entity->GetOrigin());
}
}
