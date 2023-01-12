#include <limits>

#include <QSignalBlocker>

#include "entity/HLMVStudioModelEntity.hpp"

#include "ui/StateSnapshot.hpp"

#include "plugins/halflife/studiomodel/StudioModelAsset.hpp"
#include "plugins/halflife/studiomodel/ui/DockHelpers.hpp"
#include "plugins/halflife/studiomodel/ui/StudioModelData.hpp"
#include "plugins/halflife/studiomodel/ui/StudioModelUndoCommands.hpp"
#include "plugins/halflife/studiomodel/ui/dockpanels/HitboxesPanel.hpp"

namespace studiomodel
{
HitboxesPanel::HitboxesPanel(StudioModelAssetProvider* provider)
	: _provider(provider)
{
	_ui.setupUi(this);

	_ui.Hitgroup->setRange(0, std::numeric_limits<int>::max());

	connect(_provider, &StudioModelAssetProvider::AssetChanged, this, &HitboxesPanel::OnAssetChanged);

	connect(_ui.Hitboxes, qOverload<int>(&QComboBox::currentIndexChanged), this, &HitboxesPanel::OnHitboxChanged);
	connect(_ui.HighlightHitbox, &QCheckBox::stateChanged, this, &HitboxesPanel::OnHighlightHitboxChanged);

	connect(_ui.Bone, qOverload<int>(&QComboBox::currentIndexChanged), this, &HitboxesPanel::OnHitboxPropsChanged);
	connect(_ui.Hitgroup, qOverload<int>(&QSpinBox::valueChanged), this, &HitboxesPanel::OnHitboxPropsChanged);

	connect(_ui.Minimum, &qt::widgets::SimpleVector3Edit::ValueChanged, this, &HitboxesPanel::OnHitboxPropsChanged);
	connect(_ui.Maximum, &qt::widgets::SimpleVector3Edit::ValueChanged, this, &HitboxesPanel::OnHitboxPropsChanged);

	OnAssetChanged(_provider->GetDummyAsset());
}

HitboxesPanel::~HitboxesPanel() = default;

void HitboxesPanel::OnLayoutDirectionChanged(QBoxLayout::Direction direction)
{
	_ui.DockLayout->setDirection(direction);
}

void HitboxesPanel::OnAssetChanged(StudioModelAsset* asset)
{
	if (_asset)
	{
		_asset->disconnect(this);
	}

	_asset = asset;

	auto modelData = _asset->GetModelData();

	connect(_asset, &StudioModelAsset::SaveSnapshot, this, &HitboxesPanel::OnSaveSnapshot);
	connect(_asset, &StudioModelAsset::LoadSnapshot, this, &HitboxesPanel::OnLoadSnapshot);

	{
		const QSignalBlocker blocker{_ui.Bone};
		_ui.Bone->setModel(modelData->Bones);
		// Start off with nothing selected
		_ui.Bone->setCurrentIndex(-1);
	}

	_ui.Hitboxes->setModel(modelData->Hitboxes);

	this->setEnabled(_ui.Hitboxes->count() > 0);

	if (_previousModelData)
	{
		_previousModelData->DisconnectFromAll(this);
	}

	_previousModelData = modelData;

	connect(modelData->Hitboxes, &QAbstractItemModel::dataChanged, this, &HitboxesPanel::UpdateQCString);

	connect(modelData, &StudioModelData::HitboxDataChanged, this, [this](int index)
		{
			if (index == _ui.Hitboxes->currentIndex())
			{
				OnHitboxChanged(index);
			}
		});
}

void HitboxesPanel::OnSaveSnapshot(StateSnapshot* snapshot)
{
	snapshot->SetValue("hitboxes.hitbox", _ui.Hitboxes->currentIndex());
}

void HitboxesPanel::OnLoadSnapshot(StateSnapshot* snapshot)
{
	SetRestoredModelIndex(snapshot->Value("hitboxes.hitbox").toInt(), _asset->GetEntity()->GetEditableModel()->Hitboxes.size(), *_ui.Hitboxes);
}

void HitboxesPanel::UpdateQCString()
{
	const int index = _ui.Hitboxes->currentIndex();

	if (index != -1)
	{
		const auto& hitbox = *_asset->GetEditableStudioModel()->Hitboxes[index];

		_ui.QCString->setText(QString{"$hbox %1 \"%2\" %3 %4 %5 %6 %7 %8"}
			.arg(hitbox.Group)
			.arg(QString::fromStdString(hitbox.Bone->Name))
			.arg(hitbox.Min[0], 0, 'f', 6)
			.arg(hitbox.Min[1], 0, 'f', 6)
			.arg(hitbox.Min[2], 0, 'f', 6)
			.arg(hitbox.Max[0], 0, 'f', 6)
			.arg(hitbox.Max[1], 0, 'f', 6)
			.arg(hitbox.Max[2], 0, 'f', 6));

		_ui.QCString->setCursorPosition(0);
	}
	else
	{
		_ui.QCString->setText({});
	}
}

void HitboxesPanel::OnHitboxChanged(int index)
{
	// Don't refresh the UI if this is getting called in response to a change we made.
	if (_changingHitboxProperties)
	{
		return;
	}

	constexpr studiomdl::StudioHitbox emptyHitbox{};

	const auto& hitbox = index != -1 ? *_asset->GetEditableStudioModel()->Hitboxes[index] : emptyHitbox;

	{
		const QSignalBlocker bone{_ui.Bone};
		const QSignalBlocker hitgroup{_ui.Hitgroup};
		const QSignalBlocker minimum{_ui.Minimum};
		const QSignalBlocker maximum{_ui.Maximum};

		_ui.Bone->setCurrentIndex(hitbox.Bone ? hitbox.Bone->ArrayIndex : -1);
		_ui.Hitgroup->setValue(hitbox.Group);
		_ui.Minimum->SetValue(hitbox.Min);
		_ui.Maximum->SetValue(hitbox.Max);
	}

	UpdateQCString();

	OnHighlightHitboxChanged();
}

void HitboxesPanel::OnHighlightHitboxChanged()
{
	_asset->DrawSingleHitboxIndex = _ui.HighlightHitbox->isChecked() ? _ui.Hitboxes->currentIndex() : -1;
}

void HitboxesPanel::OnHitboxPropsChanged()
{
	const auto model = _asset->GetEntity()->GetEditableModel();
	const auto& hitbox = *model->Hitboxes[_ui.Hitboxes->currentIndex()];

	const HitboxProps oldProps
	{
		.Bone = hitbox.Bone->ArrayIndex,
		.Group = hitbox.Group,
		.Min = hitbox.Min,
		.Max = hitbox.Max
	};

	const HitboxProps newProps
	{
		.Bone = _ui.Bone->currentIndex(),
		.Group = _ui.Hitgroup->value(),
		.Min = _ui.Minimum->GetValue(),
		.Max = _ui.Maximum->GetValue()
	};

	_changingHitboxProperties = true;
	_asset->AddUndoCommand(new ChangeHitboxPropsCommand(_asset, _ui.Hitboxes->currentIndex(), oldProps, newProps));
	_changingHitboxProperties = false;
}
}
