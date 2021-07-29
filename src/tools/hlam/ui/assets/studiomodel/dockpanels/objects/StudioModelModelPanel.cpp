#include <QSignalBlocker>

#include "entity/HLMVStudioModelEntity.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/dockpanels/objects/StudioModelModelPanel.hpp"

namespace ui::assets::studiomodel
{
StudioModelModelPanel::StudioModelModelPanel(StudioModelAsset* asset, QWidget* parent)
	: QWidget(parent)
	, _asset(asset)
{
	_ui.setupUi(this);

	_ui.Origin->SetRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
	_ui.Origin->SetDecimals(6);

	connect(_ui.Origin, &qt::widgets::Vector3Edit::ValueChanged, this, &StudioModelModelPanel::OnOriginChanged);
	connect(_ui.CenterOnWorldOrigin, &QPushButton::clicked, this, &StudioModelModelPanel::OnCenterOnWorldOrigin);
	connect(_ui.AlignOnGround, &QPushButton::clicked, this, &StudioModelModelPanel::OnAlignOnGround);
}

void StudioModelModelPanel::OnOriginChanged()
{
	_asset->GetScene()->GetEntity()->SetOrigin(_ui.Origin->GetValue());
}

void StudioModelModelPanel::OnCenterOnWorldOrigin()
{
	//TODO: need a better way to sync UI to origin
	auto entity = _asset->GetScene()->GetEntity();

	entity->SetOrigin({0, 0, 0});

	const QSignalBlocker blocker{_ui.Origin};
	_ui.Origin->SetValue(entity->GetOrigin());
}

void StudioModelModelPanel::OnAlignOnGround()
{
	auto entity = _asset->GetScene()->GetEntity();

	_asset->GetScene()->AlignOnGround();

	const QSignalBlocker blocker{_ui.Origin};
	_ui.Origin->SetValue(entity->GetOrigin());
}
}
