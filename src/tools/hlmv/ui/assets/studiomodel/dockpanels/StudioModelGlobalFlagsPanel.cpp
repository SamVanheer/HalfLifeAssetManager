#include <string_view>

#include <QSignalBlocker>

#include "entity/CHLMVStudioModelEntity.h"

#include "ui/assets/studiomodel/StudioModelContext.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelGlobalFlagsPanel.hpp"

namespace ui::assets::studiomodel
{
const std::string_view CheckBoxModelFlagProperty{"CheckBoxFlagProperty"};

StudioModelGlobalFlagsPanel::StudioModelGlobalFlagsPanel(StudioModelContext* context, QWidget* parent)
	: QWidget(parent)
	, _context(context)
{
	_ui.setupUi(this);

	connect(_ui.RocketTrail, &QCheckBox::stateChanged, this, &StudioModelGlobalFlagsPanel::OnFlagChanged);
	connect(_ui.GrenadeSmoke, &QCheckBox::stateChanged, this, &StudioModelGlobalFlagsPanel::OnFlagChanged);
	connect(_ui.GibBlood, &QCheckBox::stateChanged, this, &StudioModelGlobalFlagsPanel::OnFlagChanged);
	connect(_ui.ModelRotate, &QCheckBox::stateChanged, this, &StudioModelGlobalFlagsPanel::OnFlagChanged);

	connect(_ui.GreenTrail, &QCheckBox::stateChanged, this, &StudioModelGlobalFlagsPanel::OnFlagChanged);
	connect(_ui.ZombieBlood, &QCheckBox::stateChanged, this, &StudioModelGlobalFlagsPanel::OnFlagChanged);
	connect(_ui.OrangeTrail, &QCheckBox::stateChanged, this, &StudioModelGlobalFlagsPanel::OnFlagChanged);
	connect(_ui.PurpleTrail, &QCheckBox::stateChanged, this, &StudioModelGlobalFlagsPanel::OnFlagChanged);

	connect(_ui.NoShadeLight, &QCheckBox::stateChanged, this, &StudioModelGlobalFlagsPanel::OnFlagChanged);
	connect(_ui.HitboxCollision, &QCheckBox::stateChanged, this, &StudioModelGlobalFlagsPanel::OnFlagChanged);
	connect(_ui.ForceSkylight, &QCheckBox::stateChanged, this, &StudioModelGlobalFlagsPanel::OnFlagChanged);

	QCheckBox* checkBoxes[] =
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

	const int flags = _context->GetScene()->GetEntity()->GetModel()->GetStudioHeader()->flags;

	for (auto checkBox : checkBoxes)
	{
		const auto flagValue = checkBox->property(CheckBoxModelFlagProperty.data()).toInt();

		{
			const QSignalBlocker blocker{checkBox};

			checkBox->setChecked((flags & flagValue) != 0);
		}
	}
}

StudioModelGlobalFlagsPanel::~StudioModelGlobalFlagsPanel() = default;

void StudioModelGlobalFlagsPanel::OnFlagChanged(int state)
{
	auto checkBox = sender();

	const auto flagValue = checkBox->property(CheckBoxModelFlagProperty.data()).toInt();

	auto model = _context->GetScene()->GetEntity()->GetModel()->GetStudioHeader();

	if (state == Qt::CheckState::Checked)
	{
		model->flags |= flagValue;
	}
	else
	{
		model->flags &= ~flagValue;
	}
}
}
