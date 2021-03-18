#include <limits>
#include <string_view>

#include <QSignalBlocker>

#include "entity/HLMVStudioModelEntity.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelUndoCommands.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelModelDataPanel.hpp"

using namespace qt::widgets;

namespace ui::assets::studiomodel
{
const std::string_view CheckBoxModelFlagProperty{"CheckBoxFlagProperty"};

StudioModelModelDataPanel::StudioModelModelDataPanel(StudioModelAsset* asset, QWidget* parent)
	: QWidget(parent)
	, _asset(asset)
{
	_ui.setupUi(this);

	QDoubleSpinBox* const origin[] =
	{
		_ui.OriginX,
		_ui.OriginY,
		_ui.OriginZ
	};

	for (auto originSpinner : origin)
	{
		originSpinner->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
	}

	_ui.ScaleMeshSpinner->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
	_ui.ScaleBonesSpinner->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());

	_ui.EyePosition->SetRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
	_ui.EyePosition->SetDecimals(6);

	_ui.BBoxMin->SetRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
	_ui.BBoxMin->SetDecimals(6);
	_ui.BBoxMin->SetPrefix("Min ");

	_ui.BBoxMax->SetRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
	_ui.BBoxMax->SetDecimals(6);
	_ui.BBoxMax->SetPrefix("Max ");

	_ui.CBoxMin->SetRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
	_ui.CBoxMin->SetDecimals(6);
	_ui.CBoxMin->SetPrefix("Min ");

	_ui.CBoxMax->SetRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
	_ui.CBoxMax->SetDecimals(6);
	_ui.CBoxMax->SetPrefix("Max ");

	connect(_asset, &StudioModelAsset::ModelChanged, this, &StudioModelModelDataPanel::OnModelChanged);

	connect(_ui.OriginX, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelModelDataPanel::OnOriginChanged);
	connect(_ui.OriginY, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelModelDataPanel::OnOriginChanged);
	connect(_ui.OriginZ, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelModelDataPanel::OnOriginChanged);

	connect(_ui.SetOrigin, &QPushButton::clicked, this, &StudioModelModelDataPanel::OnOriginChanged);

	connect(_ui.ScaleMesh, &QPushButton::clicked, this, &StudioModelModelDataPanel::OnScaleMesh);
	connect(_ui.ScaleBones, &QPushButton::clicked, this, &StudioModelModelDataPanel::OnScaleBones);

	connect(_ui.RocketTrail, &QCheckBox::stateChanged, this, &StudioModelModelDataPanel::OnFlagChanged);
	connect(_ui.GrenadeSmoke, &QCheckBox::stateChanged, this, &StudioModelModelDataPanel::OnFlagChanged);
	connect(_ui.GibBlood, &QCheckBox::stateChanged, this, &StudioModelModelDataPanel::OnFlagChanged);
	connect(_ui.ModelRotate, &QCheckBox::stateChanged, this, &StudioModelModelDataPanel::OnFlagChanged);

	connect(_ui.GreenTrail, &QCheckBox::stateChanged, this, &StudioModelModelDataPanel::OnFlagChanged);
	connect(_ui.ZombieBlood, &QCheckBox::stateChanged, this, &StudioModelModelDataPanel::OnFlagChanged);
	connect(_ui.OrangeTrail, &QCheckBox::stateChanged, this, &StudioModelModelDataPanel::OnFlagChanged);
	connect(_ui.PurpleTrail, &QCheckBox::stateChanged, this, &StudioModelModelDataPanel::OnFlagChanged);

	connect(_ui.NoShadeLight, &QCheckBox::stateChanged, this, &StudioModelModelDataPanel::OnFlagChanged);
	connect(_ui.HitboxCollision, &QCheckBox::stateChanged, this, &StudioModelModelDataPanel::OnFlagChanged);
	connect(_ui.ForceSkylight, &QCheckBox::stateChanged, this, &StudioModelModelDataPanel::OnFlagChanged);

	connect(_ui.EyePosition, &Vector3Edit::ValueChanged, this, &StudioModelModelDataPanel::OnEyePositionChanged);

	connect(_ui.BBoxMin, &Vector3Edit::ValueChanged, this, &StudioModelModelDataPanel::OnBBoxMinChanged);
	connect(_ui.BBoxMax, &Vector3Edit::ValueChanged, this, &StudioModelModelDataPanel::OnBBoxMaxChanged);

	connect(_ui.CBoxMin, &Vector3Edit::ValueChanged, this, &StudioModelModelDataPanel::OnCBoxMinChanged);
	connect(_ui.CBoxMax, &Vector3Edit::ValueChanged, this, &StudioModelModelDataPanel::OnCBoxMaxChanged);

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

	auto model = _asset->GetScene()->GetEntity()->GetEditableModel();

	SetFlags(model->Flags);

	{
		const QSignalBlocker blocker{_ui.EyePosition};
		_ui.EyePosition->SetValue(model->EyePosition);
	}

	{
		const QSignalBlocker min{_ui.BBoxMin};
		const QSignalBlocker max{_ui.BBoxMax};

		_ui.BBoxMin->SetValue(model->BoundingMin);
		_ui.BBoxMax->SetValue(model->BoundingMax);
	}

	{
		const QSignalBlocker min{_ui.CBoxMin};
		const QSignalBlocker max{_ui.CBoxMax};

		_ui.CBoxMin->SetValue(model->ClippingMin);
		_ui.CBoxMax->SetValue(model->ClippingMax);
	}
}

StudioModelModelDataPanel::~StudioModelModelDataPanel() = default;

void StudioModelModelDataPanel::OnModelChanged(const ModelChangeEvent& event)
{
	switch (event.GetId())
	{
	case ModelChangeId::ChangeEyePosition:
	{
		const auto& change = static_cast<const ModelEyePositionChangeEvent&>(event);

		const QSignalBlocker blocker{_ui.EyePosition};

		_ui.EyePosition->SetValue(change.GetPosition());
		break;
	}

	case ModelChangeId::ChangeBBox:
	{
		const auto& change = static_cast<const ModelBBoxChangeEvent&>(event);

		const QSignalBlocker min{_ui.BBoxMin};
		const QSignalBlocker max{_ui.BBoxMax};

		_ui.BBoxMin->SetValue(change.GetValue().first);
		_ui.BBoxMax->SetValue(change.GetValue().second);
		break;
	}

	case ModelChangeId::ChangeCBox:
	{
		const auto& change = static_cast<const ModelCBoxChangeEvent&>(event);

		const QSignalBlocker min{_ui.CBoxMin};
		const QSignalBlocker max{_ui.CBoxMax};

		_ui.CBoxMin->SetValue(change.GetValue().first);
		_ui.CBoxMax->SetValue(change.GetValue().second);
		break;
	}

	case ModelChangeId::ChangeModelOrigin:
	{
		const auto& originChange = static_cast<const ModelOriginChangeEvent&>(event);

		_oldOffset = originChange.GetOffset();

		const QSignalBlocker originX{_ui.OriginX};
		const QSignalBlocker originY{_ui.OriginY};
		const QSignalBlocker originZ{_ui.OriginZ};

		_ui.OriginX->setValue(_oldOffset.x);
		_ui.OriginY->setValue(_oldOffset.y);
		_ui.OriginZ->setValue(_oldOffset.z);
		break;
	}

	//The mesh and bone scales aren't reset here because they're not representative of the absolute model scale compared to the initial state

	case ModelChangeId::ChangeModelFlags:
	{
		SetFlags(_asset->GetScene()->GetEntity()->GetEditableModel()->Flags);
		break;
	}
	}
}

void StudioModelModelDataPanel::SetFlags(int flags)
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

		{
			const QSignalBlocker blocker{checkBox};

			checkBox->setChecked((flags & flagValue) != 0);
		}
	}
}

void StudioModelModelDataPanel::OnOriginChanged()
{
	const auto model = _asset->GetScene()->GetEntity()->GetEditableModel();

	const glm::vec3 absoluteOffset{_ui.OriginX->value(), _ui.OriginY->value(), _ui.OriginZ->value()};
	const auto relativeOffset{absoluteOffset - _oldOffset};

	const auto rootBoneIndices{model->GetRootBoneIndices()};

	std::vector<RootBoneData> oldRootBonePositions;
	std::vector<RootBoneData> newRootBonePositions;

	oldRootBonePositions.reserve(rootBoneIndices.size());
	newRootBonePositions.reserve(rootBoneIndices.size());

	for (auto rootBoneIndex : rootBoneIndices)
	{
		const auto& rootBone = *model->Bones[rootBoneIndex];

		oldRootBonePositions.emplace_back(
			RootBoneData
			{
				rootBoneIndex,
				{rootBone.Axes[0].Value, rootBone.Axes[1].Value, rootBone.Axes[2].Value}
			}
		);

		newRootBonePositions.emplace_back(
			RootBoneData
			{
				rootBoneIndex,
				{
					rootBone.Axes[0].Value + relativeOffset[0],
					rootBone.Axes[1].Value + relativeOffset[1],
					rootBone.Axes[2].Value + relativeOffset[2]
				}
			});
	}

	if (!newRootBonePositions.empty())
	{
		_asset->AddUndoCommand(
			new ChangeModelOriginCommand(_asset, {std::move(oldRootBonePositions), _oldOffset}, {std::move(newRootBonePositions), absoluteOffset}));
	}
}

void StudioModelModelDataPanel::OnScaleMesh()
{
	auto entity = _asset->GetScene()->GetEntity();

	auto data{studiomdl::CalculateScaledMeshesData(*entity->GetEditableModel(), _ui.ScaleMeshSpinner->value())};

	_asset->AddUndoCommand(new ChangeModelMeshesScaleCommand(_asset, std::move(data.first), std::move(data.second)));
}

void StudioModelModelDataPanel::OnScaleBones()
{
	auto entity = _asset->GetScene()->GetEntity();

	auto data{studiomdl::CalculateScaledBonesData(*entity->GetEditableModel(), _ui.ScaleBonesSpinner->value())};

	_asset->AddUndoCommand(new ChangeModelBonesScaleCommand(_asset, std::move(data.first), std::move(data.second)));
}

void StudioModelModelDataPanel::OnFlagChanged(int state)
{
	const auto checkBox = sender();

	const auto flagValue = checkBox->property(CheckBoxModelFlagProperty.data()).toInt();

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

void StudioModelModelDataPanel::OnEyePositionChanged(const glm::vec3& value)
{
	_asset->AddUndoCommand(new ChangeEyePositionCommand(_asset, _asset->GetScene()->GetEntity()->GetEditableModel()->EyePosition, value));
}

void StudioModelModelDataPanel::OnBBoxMinChanged(const glm::vec3& value)
{
	auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	_asset->AddUndoCommand(new ChangeBBoxCommand(_asset, {model->BoundingMin, model->BoundingMax}, {value, model->BoundingMax}));
}

void StudioModelModelDataPanel::OnBBoxMaxChanged(const glm::vec3& value)
{
	auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	_asset->AddUndoCommand(new ChangeBBoxCommand(_asset, {model->BoundingMin, model->BoundingMax}, {model->BoundingMin, value}));
}

void StudioModelModelDataPanel::OnCBoxMinChanged(const glm::vec3& value)
{
	auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	_asset->AddUndoCommand(new ChangeCBoxCommand(_asset, {model->ClippingMin, model->ClippingMax}, {value, model->ClippingMax}));
}

void StudioModelModelDataPanel::OnCBoxMaxChanged(const glm::vec3& value)
{
	auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	_asset->AddUndoCommand(new ChangeCBoxCommand(_asset, {model->ClippingMin, model->ClippingMax}, {model->ClippingMin, value}));
}
}
