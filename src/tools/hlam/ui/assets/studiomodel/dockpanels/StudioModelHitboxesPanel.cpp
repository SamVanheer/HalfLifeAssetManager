#include <limits>

#include <QSignalBlocker>

#include "entity/CHLMVStudioModelEntity.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelUndoCommands.hpp"
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
		spinBox->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
	}

	connect(_asset, &StudioModelAsset::ModelChanged, this, &StudioModelHitboxesPanel::OnModelChanged);

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

	const auto model = _asset->GetScene()->GetEntity()->GetModel();

	const auto header = model->GetStudioHeader();

	this->setEnabled(header->numhitboxes > 0);

	{
		const QSignalBlocker blocker{_ui.Bone};

		QStringList bones;

		bones.reserve(header->numbones);

		for (int i = 0; i < header->numbones; ++i)
		{
			bones.append(QString{"%1 (%2)"}.arg(header->GetBone(i)->name).arg(i));
		}

		_ui.Bone->addItems(bones);

		//Start off with nothing selected
		_ui.Bone->setCurrentIndex(-1);
	}

	if (header->numhitboxes > 0)
	{
		QStringList hitboxes;

		hitboxes.reserve(header->numhitboxes);

		for (int i = 0; i < header->numhitboxes; ++i)
		{
			hitboxes.append(QString{"Hitbox %1"}.arg(i + 1));
		}

		_ui.Hitboxes->addItems(hitboxes);
	}
}

StudioModelHitboxesPanel::~StudioModelHitboxesPanel() = default;

void StudioModelHitboxesPanel::OnModelChanged(const ModelChangeEvent& event)
{
	const auto model = _asset->GetScene()->GetEntity()->GetModel();
	const auto header = model->GetStudioHeader();

	switch (event.GetId())
	{
	case ModelChangeId::RenameBone:
	{
		const QSignalBlocker boneBlocker{_ui.Bone};

		const auto& listChange{static_cast<const ModelListChangeEvent&>(event)};

		const auto bone = header->GetBone(listChange.GetSourceIndex());

		_ui.Bone->setItemText(listChange.GetSourceIndex(), QString{"%1 (%2)"}.arg(bone->name).arg(listChange.GetSourceIndex()));
		UpdateQCString();
		break;
	}

	case ModelChangeId::ChangeHitboxBone:
	{
		const auto& listChange{static_cast<const ModelListChangeEvent&>(event)};

		if (listChange.GetSourceIndex() == _ui.Hitboxes->currentIndex())
		{
			const auto hitbox = header->GetHitBox(listChange.GetSourceIndex());

			const QSignalBlocker blocker{_ui.Bone};
			_ui.Bone->setCurrentIndex(hitbox->bone);

			UpdateQCString();
		}
		break;
	}

	case ModelChangeId::ChangeHitboxHitgroup:
	{
		const auto& listChange{static_cast<const ModelListChangeEvent&>(event)};

		if (listChange.GetSourceIndex() == _ui.Hitboxes->currentIndex())
		{
			const auto hitbox = header->GetHitBox(listChange.GetSourceIndex());

			const QSignalBlocker blocker{_ui.Hitgroup};
			_ui.Hitgroup->setValue(hitbox->group);

			UpdateQCString();
		}
		break;
	}

	case ModelChangeId::ChangeHitboxBounds:
	{
		const auto& listChange{static_cast<const ModelListChangeEvent&>(event)};

		if (listChange.GetSourceIndex() == _ui.Hitboxes->currentIndex())
		{
			const auto hitbox = header->GetHitBox(listChange.GetSourceIndex());

			const QSignalBlocker minimumX{_ui.MinimumX};
			const QSignalBlocker minimumY{_ui.MinimumY};
			const QSignalBlocker minimumZ{_ui.MinimumZ};

			const QSignalBlocker maximumX{_ui.MaximumX};
			const QSignalBlocker maximumY{_ui.MaximumY};
			const QSignalBlocker maximumZ{_ui.MaximumZ};

			_ui.MinimumX->setValue(hitbox->bbmin[0]);
			_ui.MinimumY->setValue(hitbox->bbmin[1]);
			_ui.MinimumZ->setValue(hitbox->bbmin[2]);

			_ui.MaximumX->setValue(hitbox->bbmax[0]);
			_ui.MaximumY->setValue(hitbox->bbmax[1]);
			_ui.MaximumZ->setValue(hitbox->bbmax[2]);

			UpdateQCString();
		}
		break;
	}
	}
}

void StudioModelHitboxesPanel::UpdateQCString()
{
	const auto model = _asset->GetScene()->GetEntity()->GetModel();
	const auto header = model->GetStudioHeader();
	const auto hitbox = header->GetHitBox(_ui.Hitboxes->currentIndex());
	const auto bone = header->GetBone(hitbox->bone);

	_ui.QCString->setText(QString{"$hbox %1 \"%2\" %3 %4 %5 %6 %7 %8"}
		.arg(hitbox->group)
		.arg(bone->name)
		.arg(hitbox->bbmin[0], 0, 'f', 6)
		.arg(hitbox->bbmin[1], 0, 'f', 6)
		.arg(hitbox->bbmin[2], 0, 'f', 6)
		.arg(hitbox->bbmax[0], 0, 'f', 6)
		.arg(hitbox->bbmax[1], 0, 'f', 6)
		.arg(hitbox->bbmax[2], 0, 'f', 6));
}

void StudioModelHitboxesPanel::OnDockPanelChanged(QWidget* current, QWidget* previous)
{
	_isActive = current == this;

	OnHighlightHitboxChanged();
}

void StudioModelHitboxesPanel::OnHitboxChanged(int index)
{
	const auto model = _asset->GetScene()->GetEntity()->GetModel();
	const auto header = model->GetStudioHeader();
	const auto hitbox = header->GetHitBox(index);

	{
		const QSignalBlocker bone{_ui.Bone};
		const QSignalBlocker hitgroup{_ui.Hitgroup};

		const QSignalBlocker minimumX{_ui.MinimumX};
		const QSignalBlocker minimumY{_ui.MinimumY};
		const QSignalBlocker minimumZ{_ui.MinimumZ};

		const QSignalBlocker maximumX{_ui.MaximumX};
		const QSignalBlocker maximumY{_ui.MaximumY};
		const QSignalBlocker maximumZ{_ui.MaximumZ};

		_ui.Bone->setCurrentIndex(hitbox->bone);

		_ui.Hitgroup->setValue(hitbox->group);

		_ui.MinimumX->setValue(hitbox->bbmin[0]);
		_ui.MinimumY->setValue(hitbox->bbmin[1]);
		_ui.MinimumZ->setValue(hitbox->bbmin[2]);

		_ui.MaximumX->setValue(hitbox->bbmax[0]);
		_ui.MaximumY->setValue(hitbox->bbmax[1]);
		_ui.MaximumZ->setValue(hitbox->bbmax[2]);
	}

	UpdateQCString();

	OnHighlightHitboxChanged();
}

void StudioModelHitboxesPanel::OnHighlightHitboxChanged()
{
	_asset->GetScene()->DrawSingleHitboxIndex = (_isActive && _ui.HighlightHitbox->isChecked()) ? _ui.Hitboxes->currentIndex() : -1;
}

void StudioModelHitboxesPanel::OnBoneChanged()
{
	const auto model = _asset->GetScene()->GetEntity()->GetModel();
	const auto header = model->GetStudioHeader();
	const auto hitbox = header->GetHitBox(_ui.Hitboxes->currentIndex());

	_asset->AddUndoCommand(new ChangeHitboxBoneCommand(_asset, _ui.Hitboxes->currentIndex(), hitbox->bone, _ui.Bone->currentIndex()));
}

void StudioModelHitboxesPanel::OnHitgroupChanged()
{
	const auto model = _asset->GetScene()->GetEntity()->GetModel();
	const auto header = model->GetStudioHeader();
	const auto hitbox = header->GetHitBox(_ui.Hitboxes->currentIndex());

	_asset->AddUndoCommand(new ChangeHitboxHitgroupCommand(_asset, _ui.Hitboxes->currentIndex(), hitbox->group, _ui.Hitgroup->value()));
}

void StudioModelHitboxesPanel::OnBoundsChanged()
{
	const auto model = _asset->GetScene()->GetEntity()->GetModel();
	const auto header = model->GetStudioHeader();
	const auto hitbox = header->GetHitBox(_ui.Hitboxes->currentIndex());

	_asset->AddUndoCommand(new ChangeHitboxBoundsCommand(_asset, _ui.Hitboxes->currentIndex(),
		std::make_pair(
			glm::vec3{hitbox->bbmin[0], hitbox->bbmin[1], hitbox->bbmin[2]},
			glm::vec3{hitbox->bbmax[0], hitbox->bbmax[1], hitbox->bbmax[2]}),
		std::make_pair(
			glm::vec3{_ui.MinimumX->value(), _ui.MinimumY->value(), _ui.MinimumZ->value()},
			glm::vec3{_ui.MaximumX->value(), _ui.MaximumY->value(), _ui.MaximumZ->value()})));
}
}
