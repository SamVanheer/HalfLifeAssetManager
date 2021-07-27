#include <QSignalBlocker>

#include "entity/HLMVStudioModelEntity.hpp"

#include "ui/StateSnapshot.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelUndoCommands.hpp"
#include "ui/assets/studiomodel/dockpanels/objects/StudioModelModelPanel.hpp"

namespace ui::assets::studiomodel
{
StudioModelModelPanel::StudioModelModelPanel(StudioModelAsset* asset, QWidget* parent)
	: QWidget(parent)
	, _asset(asset)
{
	_ui.setupUi(this);

	connect(_asset, &StudioModelAsset::ModelChanged, this, &StudioModelModelPanel::OnModelChanged);
	connect(_asset, &StudioModelAsset::LoadSnapshot, this, &StudioModelModelPanel::InitializeUI);

	connect(_ui.CenterOnWorldOrigin, &QPushButton::clicked, this, &StudioModelModelPanel::OnCenterOnWorldOrigin);
	connect(_ui.AlignOnGround, &QPushButton::clicked, this, &StudioModelModelPanel::OnAlignOnGround);

	InitializeUI();
}

void StudioModelModelPanel::InitializeUI()
{
}

void StudioModelModelPanel::OnModelChanged(const ModelChangeEvent& event)
{
}

void StudioModelModelPanel::OnCenterOnWorldOrigin()
{
	_asset->GetScene()->GetEntity()->SetOrigin({0, 0, 0});
}

void StudioModelModelPanel::OnAlignOnGround()
{
	_asset->GetScene()->AlignOnGround();
}
}
