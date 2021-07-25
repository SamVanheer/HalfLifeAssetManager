#include <string_view>

#include <QSignalBlocker>

#include "entity/HLMVStudioModelEntity.hpp"

#include "ui/StateSnapshot.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelUndoCommands.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelFlagsPanel.hpp"

namespace ui::assets::studiomodel
{
const std::string_view CheckBoxModelFlagProperty{"CheckBoxFlagProperty"};

StudioModelFlagsPanel::StudioModelFlagsPanel(StudioModelAsset* asset, QWidget* parent)
	: QWidget(parent)
	, _asset(asset)
{
	_ui.setupUi(this);

	connect(_asset, &StudioModelAsset::ModelChanged, this, &StudioModelFlagsPanel::OnModelChanged);
	connect(_asset, &StudioModelAsset::LoadSnapshot, this, &StudioModelFlagsPanel::InitializeUI);

	connect(_ui.RocketTrail, &QCheckBox::stateChanged, this, &StudioModelFlagsPanel::OnFlagChanged);
	connect(_ui.GrenadeSmoke, &QCheckBox::stateChanged, this, &StudioModelFlagsPanel::OnFlagChanged);
	connect(_ui.GibBlood, &QCheckBox::stateChanged, this, &StudioModelFlagsPanel::OnFlagChanged);
	connect(_ui.ModelRotate, &QCheckBox::stateChanged, this, &StudioModelFlagsPanel::OnFlagChanged);

	connect(_ui.GreenTrail, &QCheckBox::stateChanged, this, &StudioModelFlagsPanel::OnFlagChanged);
	connect(_ui.ZombieBlood, &QCheckBox::stateChanged, this, &StudioModelFlagsPanel::OnFlagChanged);
	connect(_ui.OrangeTrail, &QCheckBox::stateChanged, this, &StudioModelFlagsPanel::OnFlagChanged);
	connect(_ui.PurpleTrail, &QCheckBox::stateChanged, this, &StudioModelFlagsPanel::OnFlagChanged);

	connect(_ui.NoShadeLight, &QCheckBox::stateChanged, this, &StudioModelFlagsPanel::OnFlagChanged);
	connect(_ui.HitboxCollision, &QCheckBox::stateChanged, this, &StudioModelFlagsPanel::OnFlagChanged);
	connect(_ui.ForceSkylight, &QCheckBox::stateChanged, this, &StudioModelFlagsPanel::OnFlagChanged);

	_ui.RocketTrail->setProperty(CheckBoxModelFlagProperty.data(), EF_ROCKET);
	_ui.GrenadeSmoke->setProperty(CheckBoxModelFlagProperty.data(), EF_GRENADE);
	_ui.GibBlood->setProperty(CheckBoxModelFlagProperty.data(), EF_GIB);
	_ui.ModelRotate->setProperty(CheckBoxModelFlagProperty.data(), EF_ROTATE);

	_ui.GreenTrail->setProperty(CheckBoxModelFlagProperty.data(), EF_TRACER);
	_ui.ZombieBlood->setProperty(CheckBoxModelFlagProperty.data(), EF_ZOMGIB);
	_ui.OrangeTrail->setProperty(CheckBoxModelFlagProperty.data(), EF_TRACER2);
	_ui.PurpleTrail->setProperty(CheckBoxModelFlagProperty.data(), EF_TRACER3);

	_ui.NoShadeLight->setProperty(CheckBoxModelFlagProperty.data(), EF_NOSHADELIGHT);
	_ui.HitboxCollision->setProperty(CheckBoxModelFlagProperty.data(), EF_HITBOXCOLLISIONS);
	_ui.ForceSkylight->setProperty(CheckBoxModelFlagProperty.data(), EF_FORCESKYLIGHT);

	InitializeUI();
}

void StudioModelFlagsPanel::InitializeUI()
{
	SetFlags(_asset->GetScene()->GetEntity()->GetEditableModel()->Flags);
}

void StudioModelFlagsPanel::OnModelChanged(const ModelChangeEvent& event)
{
	if (event.GetId() == ModelChangeId::ChangeModelFlags)
	{
		SetFlags(_asset->GetScene()->GetEntity()->GetEditableModel()->Flags);
	}
}

void StudioModelFlagsPanel::SetFlags(int flags)
{
	QCheckBox* const checkBoxes[] =
	{
		_ui.RocketTrail,
		_ui.GrenadeSmoke,
		_ui.GibBlood,
		_ui.ModelRotate,
		_ui.GreenTrail,
		_ui.ZombieBlood,
		_ui.OrangeTrail,
		_ui.PurpleTrail,
		_ui.NoShadeLight,
		_ui.HitboxCollision,
		_ui.ForceSkylight
	};

	for (auto checkBox : checkBoxes)
	{
		const auto flagValue = checkBox->property(CheckBoxModelFlagProperty.data()).toInt();

		const QSignalBlocker blocker{checkBox};
		checkBox->setChecked((flags & flagValue) != 0);
	}
}

void StudioModelFlagsPanel::OnFlagChanged(int state)
{
	const auto flagValue = sender()->property(CheckBoxModelFlagProperty.data()).toInt();

	const auto model = _asset->GetScene()->GetEntity()->GetEditableModel();

	int newFlags = model->Flags;

	if (state == Qt::CheckState::Checked)
	{
		newFlags |= flagValue;
	}
	else
	{
		newFlags &= ~flagValue;
	}

	_asset->AddUndoCommand(new ChangeModelFlagsCommand(_asset, model->Flags, newFlags));
}
}
