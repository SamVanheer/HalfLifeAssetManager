#include <limits>

#include <QSignalBlocker>

#include "entity/HLMVStudioModelEntity.hpp"

#include "ui/StateSnapshot.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelUndoCommands.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelDockHelpers.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelHitboxesPanel.hpp"

namespace ui::assets::studiomodel
{
StudioModelHitboxesPanel::StudioModelHitboxesPanel(StudioModelAsset* asset, QWidget* parent)
	: QWidget(parent)
	, _asset(asset)
{
	_ui.setupUi(this);

	_ui.Hitgroup->setRange(0, std::numeric_limits<int>::max());

	QDoubleSpinBox* const spinBoxes[] =
	{
		_ui.MinimumX,
		_ui.MinimumY,
		_ui.MinimumZ,
		_ui.MaximumX,
		_ui.MaximumY,
		_ui.MaximumZ
	};

	for (auto spinBox : spinBoxes)
	{
		spinBox->setRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
	}

	connect(_asset, &StudioModelAsset::ModelChanged, this, &StudioModelHitboxesPanel::OnModelChanged);
	connect(_asset, &StudioModelAsset::SaveSnapshot, this, &StudioModelHitboxesPanel::OnSaveSnapshot);
	connect(_asset, &StudioModelAsset::LoadSnapshot, this, &StudioModelHitboxesPanel::OnLoadSnapshot);

	connect(_ui.Hitboxes, qOverload<int>(&QComboBox::currentIndexChanged), this, &StudioModelHitboxesPanel::OnHitboxChanged);
	connect(_ui.HighlightHitbox, &QCheckBox::stateChanged, this, &StudioModelHitboxesPanel::OnHighlightHitboxChanged);

	connect(_ui.Bone, qOverload<int>(&QComboBox::currentIndexChanged), this, &StudioModelHitboxesPanel::OnBoneChanged);
	connect(_ui.Hitgroup, qOverload<int>(&QSpinBox::valueChanged), this, &StudioModelHitboxesPanel::OnHitgroupChanged);

	connect(_ui.MinimumX, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelHitboxesPanel::OnBoundsChanged);
	connect(_ui.MinimumY, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelHitboxesPanel::OnBoundsChanged);
	connect(_ui.MinimumZ, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelHitboxesPanel::OnBoundsChanged);

	connect(_ui.MaximumX, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelHitboxesPanel::OnBoundsChanged);
	connect(_ui.MaximumY, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelHitboxesPanel::OnBoundsChanged);
	connect(_ui.MaximumZ, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelHitboxesPanel::OnBoundsChanged);

	InitializeUI();
}

StudioModelHitboxesPanel::~StudioModelHitboxesPanel() = default;

void StudioModelHitboxesPanel::InitializeUI()
{
	const auto model = _asset->GetScene()->GetEntity()->GetEditableModel();

	this->setEnabled(!model->Hitboxes.empty());

	{
		const QSignalBlocker blocker{_ui.Bone};

		QStringList bones;

		bones.reserve(model->Bones.size());

		for (int i = 0; i < model->Bones.size(); ++i)
		{
			bones.append(QString{"%1 (%2)"}.arg(model->Bones[i]->Name.c_str()).arg(i));
		}

		_ui.Bone->clear();
		_ui.Bone->addItems(bones);

		//Start off with nothing selected
		_ui.Bone->setCurrentIndex(-1);
	}

	_ui.Hitboxes->clear();

	if (!model->Hitboxes.empty())
	{
		QStringList hitboxes;

		hitboxes.reserve(model->Hitboxes.size());

		for (int i = 0; i < model->Hitboxes.size(); ++i)
		{
			hitboxes.append(QString{"Hitbox %1"}.arg(i + 1));
		}

		_ui.Hitboxes->addItems(hitboxes);
	}
}

void StudioModelHitboxesPanel::OnModelChanged(const ModelChangeEvent& event)
{
	const auto model = _asset->GetScene()->GetEntity()->GetEditableModel();

	switch (event.GetId())
	{
	case ModelChangeId::RenameBone:
	{
		const QSignalBlocker boneBlocker{_ui.Bone};

		const auto& listChange{static_cast<const ModelListChangeEvent&>(event)};

		const auto& bone = *model->Bones[listChange.GetSourceIndex()];

		_ui.Bone->setItemText(listChange.GetSourceIndex(), QString{"%1 (%2)"}.arg(bone.Name.c_str()).arg(listChange.GetSourceIndex()));
		UpdateQCString();
		break;
	}

	case ModelChangeId::ChangeHitboxBone:
	{
		const auto& listChange{static_cast<const ModelListChangeEvent&>(event)};

		if (listChange.GetSourceIndex() == _ui.Hitboxes->currentIndex())
		{
			const auto& hitbox = *model->Hitboxes[listChange.GetSourceIndex()];

			const QSignalBlocker blocker{_ui.Bone};
			_ui.Bone->setCurrentIndex(hitbox.Bone->ArrayIndex);

			UpdateQCString();
		}
		break;
	}

	case ModelChangeId::ChangeHitboxHitgroup:
	{
		const auto& listChange{static_cast<const ModelListChangeEvent&>(event)};

		if (listChange.GetSourceIndex() == _ui.Hitboxes->currentIndex())
		{
			const auto& hitbox = *model->Hitboxes[listChange.GetSourceIndex()];

			const QSignalBlocker blocker{_ui.Hitgroup};
			_ui.Hitgroup->setValue(hitbox.Group);

			UpdateQCString();
		}
		break;
	}

	case ModelChangeId::ChangeHitboxBounds:
	{
		const auto& listChange{static_cast<const ModelListChangeEvent&>(event)};

		if (listChange.GetSourceIndex() == _ui.Hitboxes->currentIndex())
		{
			const auto& hitbox = *model->Hitboxes[listChange.GetSourceIndex()];

			const QSignalBlocker minimumX{_ui.MinimumX};
			const QSignalBlocker minimumY{_ui.MinimumY};
			const QSignalBlocker minimumZ{_ui.MinimumZ};

			const QSignalBlocker maximumX{_ui.MaximumX};
			const QSignalBlocker maximumY{_ui.MaximumY};
			const QSignalBlocker maximumZ{_ui.MaximumZ};

			_ui.MinimumX->setValue(hitbox.Min[0]);
			_ui.MinimumY->setValue(hitbox.Min[1]);
			_ui.MinimumZ->setValue(hitbox.Min[2]);

			_ui.MaximumX->setValue(hitbox.Max[0]);
			_ui.MaximumY->setValue(hitbox.Max[1]);
			_ui.MaximumZ->setValue(hitbox.Max[2]);

			UpdateQCString();
		}
		break;
	}
	}
}

void StudioModelHitboxesPanel::OnSaveSnapshot(StateSnapshot* snapshot)
{
	snapshot->SetValue("hitboxes.hitbox", _ui.Hitboxes->currentIndex());
}

void StudioModelHitboxesPanel::OnLoadSnapshot(StateSnapshot* snapshot)
{
	InitializeUI();

	SetRestoredModelIndex(snapshot->Value("hitboxes.hitbox").toInt(), _asset->GetScene()->GetEntity()->GetEditableModel()->Hitboxes.size(), *_ui.Hitboxes);
}

void StudioModelHitboxesPanel::UpdateQCString()
{
	const auto model = _asset->GetScene()->GetEntity()->GetEditableModel();

	const int index = _ui.Hitboxes->currentIndex();

	if (index != -1)
	{
		const auto& hitbox = *model->Hitboxes[index];

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

void StudioModelHitboxesPanel::OnHitboxChanged(int index)
{
	const auto model = _asset->GetScene()->GetEntity()->GetEditableModel();

	constexpr studiomdl::Hitbox emptyHitbox{};

	const auto& hitbox = index != -1 ? *model->Hitboxes[index] : emptyHitbox;

	{
		const QSignalBlocker bone{_ui.Bone};
		const QSignalBlocker hitgroup{_ui.Hitgroup};

		const QSignalBlocker minimumX{_ui.MinimumX};
		const QSignalBlocker minimumY{_ui.MinimumY};
		const QSignalBlocker minimumZ{_ui.MinimumZ};

		const QSignalBlocker maximumX{_ui.MaximumX};
		const QSignalBlocker maximumY{_ui.MaximumY};
		const QSignalBlocker maximumZ{_ui.MaximumZ};

		_ui.Bone->setCurrentIndex(hitbox.Bone ? hitbox.Bone->ArrayIndex : -1);

		_ui.Hitgroup->setValue(hitbox.Group);

		_ui.MinimumX->setValue(hitbox.Min[0]);
		_ui.MinimumY->setValue(hitbox.Min[1]);
		_ui.MinimumZ->setValue(hitbox.Min[2]);

		_ui.MaximumX->setValue(hitbox.Max[0]);
		_ui.MaximumY->setValue(hitbox.Max[1]);
		_ui.MaximumZ->setValue(hitbox.Max[2]);
	}

	UpdateQCString();

	OnHighlightHitboxChanged();
}

void StudioModelHitboxesPanel::OnHighlightHitboxChanged()
{
	_asset->GetScene()->DrawSingleHitboxIndex = _ui.HighlightHitbox->isChecked() ? _ui.Hitboxes->currentIndex() : -1;
}

void StudioModelHitboxesPanel::OnBoneChanged()
{
	const auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	const auto& hitbox = *model->Hitboxes[_ui.Hitboxes->currentIndex()];

	_asset->AddUndoCommand(new ChangeHitboxBoneCommand(_asset, _ui.Hitboxes->currentIndex(), hitbox.Bone->ArrayIndex, _ui.Bone->currentIndex()));
}

void StudioModelHitboxesPanel::OnHitgroupChanged()
{
	const auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	const auto& hitbox = *model->Hitboxes[_ui.Hitboxes->currentIndex()];

	_asset->AddUndoCommand(new ChangeHitboxHitgroupCommand(_asset, _ui.Hitboxes->currentIndex(), hitbox.Group, _ui.Hitgroup->value()));
}

void StudioModelHitboxesPanel::OnBoundsChanged()
{
	const auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	const auto& hitbox = *model->Hitboxes[_ui.Hitboxes->currentIndex()];

	_asset->AddUndoCommand(new ChangeHitboxBoundsCommand(_asset, _ui.Hitboxes->currentIndex(),
		std::make_pair(
			glm::vec3{hitbox.Min[0], hitbox.Min[1], hitbox.Min[2]},
			glm::vec3{hitbox.Max[0], hitbox.Max[1], hitbox.Max[2]}),
		std::make_pair(
			glm::vec3{_ui.MinimumX->value(), _ui.MinimumY->value(), _ui.MinimumZ->value()},
			glm::vec3{_ui.MaximumX->value(), _ui.MaximumY->value(), _ui.MaximumZ->value()})));
}
}
