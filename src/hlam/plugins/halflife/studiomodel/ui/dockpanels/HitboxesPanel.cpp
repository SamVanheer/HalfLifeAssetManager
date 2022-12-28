#include <limits>

#include <QSignalBlocker>

#include "entity/HLMVStudioModelEntity.hpp"

#include "ui/StateSnapshot.hpp"

#include "plugins/halflife/studiomodel/StudioModelAsset.hpp"
#include "plugins/halflife/studiomodel/ui/StudioModelData.hpp"
#include "plugins/halflife/studiomodel/ui/StudioModelUndoCommands.hpp"
#include "plugins/halflife/studiomodel/ui/dockpanels/DockHelpers.hpp"
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

	connect(_ui.Bone, qOverload<int>(&QComboBox::currentIndexChanged), this, &HitboxesPanel::OnBoneChanged);
	connect(_ui.Hitgroup, qOverload<int>(&QSpinBox::valueChanged), this, &HitboxesPanel::OnHitgroupChanged);

	connect(_ui.Minimum, &qt::widgets::SimpleVector3Edit::ValueChanged, this, &HitboxesPanel::OnBoundsChanged);
	connect(_ui.Maximum, &qt::widgets::SimpleVector3Edit::ValueChanged, this, &HitboxesPanel::OnBoundsChanged);

	OnAssetChanged(_provider->GetDummyAsset());
}

HitboxesPanel::~HitboxesPanel() = default;

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

	connect(modelData->Bones, &QAbstractItemModel::dataChanged, this, &HitboxesPanel::UpdateQCString);

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
			.arg(hitbox.Bone->Name.c_str())
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

	constexpr studiomdl::Hitbox emptyHitbox{};

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

void HitboxesPanel::OnBoneChanged()
{
	const auto model = _asset->GetEntity()->GetEditableModel();
	const auto& hitbox = *model->Hitboxes[_ui.Hitboxes->currentIndex()];

	_asset->AddUndoCommand(new ChangeHitboxBoneCommand(_asset, _ui.Hitboxes->currentIndex(), hitbox.Bone->ArrayIndex, _ui.Bone->currentIndex()));
}

void HitboxesPanel::OnHitgroupChanged()
{
	const auto model = _asset->GetEntity()->GetEditableModel();
	const auto& hitbox = *model->Hitboxes[_ui.Hitboxes->currentIndex()];

	_asset->AddUndoCommand(new ChangeHitboxHitgroupCommand(_asset, _ui.Hitboxes->currentIndex(), hitbox.Group, _ui.Hitgroup->value()));
}

void HitboxesPanel::OnBoundsChanged()
{
	const auto model = _asset->GetEntity()->GetEditableModel();
	const auto& hitbox = *model->Hitboxes[_ui.Hitboxes->currentIndex()];

	_changingHitboxProperties = true;

	_asset->AddUndoCommand(new ChangeHitboxBoundsCommand(_asset, _ui.Hitboxes->currentIndex(),
		std::make_pair(hitbox.Min, hitbox.Max), std::make_pair(_ui.Minimum->GetValue(), _ui.Maximum->GetValue())));

	_changingHitboxProperties = false;
}
}
