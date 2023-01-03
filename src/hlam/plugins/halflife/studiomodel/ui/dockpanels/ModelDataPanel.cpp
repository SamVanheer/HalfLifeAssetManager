#include <limits>
#include <string_view>

#include <QSignalBlocker>

#include "entity/HLMVStudioModelEntity.hpp"

#include "plugins/halflife/studiomodel/StudioModelAsset.hpp"
#include "plugins/halflife/studiomodel/ui/StudioModelData.hpp"
#include "plugins/halflife/studiomodel/ui/StudioModelUndoCommands.hpp"
#include "plugins/halflife/studiomodel/ui/dockpanels/ModelDataPanel.hpp"

using namespace qt::widgets;

namespace studiomodel
{
const std::string_view CheckBoxModelFlagProperty{"CheckBoxFlagProperty"};

ModelDataPanel::ModelDataPanel(StudioModelAssetProvider* provider)
	: _provider(provider)
{
	_ui.setupUi(this);

	connect(_provider, &StudioModelAssetProvider::AssetChanged, this, &ModelDataPanel::OnAssetChanged);

	connect(_ui.EyePosition, &SimpleVector3Edit::ValueChanged, this, &ModelDataPanel::OnEyePositionChanged);

	connect(_ui.BBoxMin, &SimpleVector3Edit::ValueChanged, this, &ModelDataPanel::OnBBoxChanged);
	connect(_ui.BBoxMax, &SimpleVector3Edit::ValueChanged, this, &ModelDataPanel::OnBBoxChanged);

	connect(_ui.CBoxMin, &SimpleVector3Edit::ValueChanged, this, &ModelDataPanel::OnCBoxChanged);
	connect(_ui.CBoxMax, &SimpleVector3Edit::ValueChanged, this, &ModelDataPanel::OnCBoxChanged);

	connect(_ui.RocketTrail, &QCheckBox::stateChanged, this, &ModelDataPanel::OnFlagChanged);
	connect(_ui.GrenadeSmoke, &QCheckBox::stateChanged, this, &ModelDataPanel::OnFlagChanged);
	connect(_ui.GibBlood, &QCheckBox::stateChanged, this, &ModelDataPanel::OnFlagChanged);
	connect(_ui.ModelRotate, &QCheckBox::stateChanged, this, &ModelDataPanel::OnFlagChanged);

	connect(_ui.GreenTrail, &QCheckBox::stateChanged, this, &ModelDataPanel::OnFlagChanged);
	connect(_ui.ZombieBlood, &QCheckBox::stateChanged, this, &ModelDataPanel::OnFlagChanged);
	connect(_ui.OrangeTrail, &QCheckBox::stateChanged, this, &ModelDataPanel::OnFlagChanged);
	connect(_ui.PurpleTrail, &QCheckBox::stateChanged, this, &ModelDataPanel::OnFlagChanged);

	connect(_ui.NoShadeLight, &QCheckBox::stateChanged, this, &ModelDataPanel::OnFlagChanged);
	connect(_ui.HitboxCollision, &QCheckBox::stateChanged, this, &ModelDataPanel::OnFlagChanged);
	connect(_ui.ForceSkylight, &QCheckBox::stateChanged, this, &ModelDataPanel::OnFlagChanged);

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

	OnAssetChanged(_provider->GetDummyAsset());
}

ModelDataPanel::~ModelDataPanel() = default;

void ModelDataPanel::SetFlags(int flags)
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

void ModelDataPanel::OnAssetChanged(StudioModelAsset* asset)
{
	if (_asset)
	{
		_asset->disconnect(this);
	}

	_asset = asset;

	auto modelData = _asset->GetModelData();

	const QSignalBlocker blocker{_ui.EyePosition};
	const QSignalBlocker bmin{_ui.BBoxMin};
	const QSignalBlocker bmax{_ui.BBoxMax};
	const QSignalBlocker cmin{_ui.CBoxMin};
	const QSignalBlocker cmax{_ui.CBoxMax};

	auto model = _asset->GetEntity()->GetEditableModel();

	_ui.EyePosition->SetValue(model->EyePosition);
	_ui.BBoxMin->SetValue(model->BoundingMin);
	_ui.BBoxMax->SetValue(model->BoundingMax);
	_ui.CBoxMin->SetValue(model->ClippingMin);
	_ui.CBoxMax->SetValue(model->ClippingMax);

	SetFlags(_asset->GetEntity()->GetEditableModel()->Flags);

	if (_previousModelData)
	{
		_previousModelData->DisconnectFromAll(this);
	}

	_previousModelData = modelData;

	connect(modelData, &StudioModelData::EyePositionChanged, this, [this]()
		{
			// Don't refresh the UI if this is getting called in response to a change we made.
			if (_changingDataProperties)
			{
				return;
			}

			const QSignalBlocker blocker{_ui.EyePosition};
			_ui.EyePosition->SetValue(_asset->GetEditableStudioModel()->EyePosition);
		});

	connect(modelData, &StudioModelData::ModelBBoxChanged, this, [this]()
		{
			// Don't refresh the UI if this is getting called in response to a change we made.
			if (_changingDataProperties)
			{
				return;
			}

			auto model = _asset->GetEditableStudioModel();
			const QSignalBlocker min{_ui.BBoxMin};
			const QSignalBlocker max{_ui.BBoxMax};
			_ui.BBoxMin->SetValue(model->BoundingMin);
			_ui.BBoxMax->SetValue(model->BoundingMax);
		});

	connect(modelData, &StudioModelData::ModelCBoxChanged, this, [this]()
		{
			// Don't refresh the UI if this is getting called in response to a change we made.
			if (_changingDataProperties)
			{
				return;
			}

			auto model = _asset->GetEditableStudioModel();
			const QSignalBlocker min{_ui.CBoxMin};
			const QSignalBlocker max{_ui.CBoxMax};
			_ui.CBoxMin->SetValue(model->ClippingMin);
			_ui.CBoxMax->SetValue(model->ClippingMax);
		});

	connect(modelData, &StudioModelData::ModelFlagsChanged, this, [this]()
		{
			SetFlags(_asset->GetEditableStudioModel()->Flags);
		});
}

void ModelDataPanel::OnEyePositionChanged()
{
	_changingDataProperties = true;
	_asset->AddUndoCommand(
		new ChangeEyePositionCommand(_asset, _asset->GetEntity()->GetEditableModel()->EyePosition, _ui.EyePosition->GetValue()));
	_changingDataProperties = false;
}

void ModelDataPanel::OnBBoxChanged()
{
	auto model = _asset->GetEntity()->GetEditableModel();
	_changingDataProperties = true;
	_asset->AddUndoCommand(
		new ChangeBBoxCommand(_asset, {model->BoundingMin, model->BoundingMax}, {_ui.BBoxMin->GetValue(), _ui.BBoxMax->GetValue()}));
	_changingDataProperties = false;
}

void ModelDataPanel::OnCBoxChanged()
{
	auto model = _asset->GetEntity()->GetEditableModel();
	_changingDataProperties = true;
	_asset->AddUndoCommand(
		new ChangeCBoxCommand(_asset, {model->ClippingMin, model->ClippingMax}, {_ui.CBoxMin->GetValue(), _ui.CBoxMax->GetValue()}));
	_changingDataProperties = false;
}



void ModelDataPanel::OnFlagChanged(int state)
{
	const auto flagValue = sender()->property(CheckBoxModelFlagProperty.data()).toInt();

	const auto model = _asset->GetEntity()->GetEditableModel();

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
