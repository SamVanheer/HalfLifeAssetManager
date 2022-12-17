#include <QSignalBlocker>

#include "entity/HLMVStudioModelEntity.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/dockpanels/objects/ModelPanel.hpp"

namespace studiomodel
{
ModelPanel::ModelPanel(StudioModelAsset* asset)
	: _asset(asset)
{
	_ui.setupUi(this);

	_ui.Origin->SetRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
	_ui.Origin->SetDecimals(6);

	connect(_ui.Origin, &qt::widgets::Vector3Edit::ValueChanged, this, &ModelPanel::OnOriginChanged);
	connect(_ui.CenterOnWorldOrigin, &QPushButton::clicked, this, &ModelPanel::OnCenterOnWorldOrigin);
	connect(_ui.AlignOnGround, &QPushButton::clicked, this, &ModelPanel::OnAlignOnGround);
}

void ModelPanel::OnLayoutDirectionChanged(QBoxLayout::Direction direction)
{
	_ui.MainLayout->setDirection(direction);
}

void ModelPanel::OnOriginChanged()
{
	_asset->GetEntity()->SetOrigin(_ui.Origin->GetValue());
}

void ModelPanel::OnCenterOnWorldOrigin()
{
	//TODO: need a better way to sync UI to origin
	auto entity = _asset->GetEntity();

	entity->SetOrigin({0, 0, 0});

	const QSignalBlocker blocker{_ui.Origin};
	_ui.Origin->SetValue(entity->GetOrigin());
}

void ModelPanel::OnAlignOnGround()
{
	auto entity = _asset->GetEntity();

	entity->AlignOnGround();

	const QSignalBlocker blocker{_ui.Origin};
	_ui.Origin->SetValue(entity->GetOrigin());
}
}
