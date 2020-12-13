#include <limits>

#include <QSignalBlocker>

#include "entity/CHLMVStudioModelEntity.h"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelUndoCommands.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelBonesPanel.hpp"

namespace ui::assets::studiomodel
{
//Parent indices are offset by one so -1 becomes 0, 0 becomes 1, etc
constexpr int ParentBoneOffset = 1;

StudioModelBonesPanel::StudioModelBonesPanel(StudioModelAsset* asset, QWidget* parent)
	: QWidget(parent)
	, _asset(asset)
{
	_ui.setupUi(this);

	_ui.BoneFlags->setRange(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());

	QDoubleSpinBox* const spinBoxes[] =
	{
		_ui.PositionX,
		_ui.PositionY,
		_ui.PositionZ,
		_ui.PositionScaleX,
		_ui.PositionScaleY,
		_ui.PositionScaleZ,
		_ui.RotationX,
		_ui.RotationY,
		_ui.RotationZ,
		_ui.RotationScaleX,
		_ui.RotationScaleY,
		_ui.RotationScaleZ
	};

	for (auto spinBox : spinBoxes)
	{
		spinBox->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
	}

	connect(_ui.Bones, qOverload<int>(&QComboBox::currentIndexChanged), this, &StudioModelBonesPanel::OnBoneChanged);
	connect(_ui.HighlightBone, &QCheckBox::stateChanged, this, &StudioModelBonesPanel::OnHightlightBoneChanged);

	connect(_ui.BonePropertyList, qOverload<int>(&QListWidget::currentRowChanged), _ui.BonePropertyStack, &QStackedWidget::setCurrentIndex);

	connect(_ui.BoneName, &QLineEdit::textChanged, this, &StudioModelBonesPanel::OnBoneNameChanged);
	connect(_ui.ParentBone, qOverload<int>(&QComboBox::currentIndexChanged), this, &StudioModelBonesPanel::OnBoneParentChanged);
	//TODO: shouldn't be a spin box
	connect(_ui.BoneFlags, qOverload<int>(&QSpinBox::valueChanged), this, &StudioModelBonesPanel::OnBoneFlagsChanged);

	connect(_ui.PositionX, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelBonesPanel::OnBonePropertyChanged);
	connect(_ui.PositionY, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelBonesPanel::OnBonePropertyChanged);
	connect(_ui.PositionZ, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelBonesPanel::OnBonePropertyChanged);

	connect(_ui.PositionScaleX, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelBonesPanel::OnBonePropertyChanged);
	connect(_ui.PositionScaleY, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelBonesPanel::OnBonePropertyChanged);
	connect(_ui.PositionScaleZ, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelBonesPanel::OnBonePropertyChanged);

	connect(_ui.RotationX, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelBonesPanel::OnBonePropertyChanged);
	connect(_ui.RotationY, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelBonesPanel::OnBonePropertyChanged);
	connect(_ui.RotationZ, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelBonesPanel::OnBonePropertyChanged);

	connect(_ui.RotationScaleX, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelBonesPanel::OnBonePropertyChanged);
	connect(_ui.RotationScaleY, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelBonesPanel::OnBonePropertyChanged);
	connect(_ui.RotationScaleZ, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelBonesPanel::OnBonePropertyChanged);

	const auto model = _asset->GetScene()->GetEntity()->GetModel();

	const auto header = model->GetStudioHeader();

	this->setEnabled(header->numbones > 0);

	QStringList bones;

	bones.reserve(header->numbones + 1);

	bones.append("None (-1)");

	for (int i = 0; i < header->numbones; ++i)
	{
		bones.append(QString{"%1 (%2)"}.arg(header->GetBone(i)->name).arg(i));
	}

	//Set up this list first so when the first bone is selected by _ui.Bones->addItems it has everything set up properly
	{
		const QSignalBlocker blocker{_ui.ParentBone};

		_ui.ParentBone->addItems(bones);

		//Start off with nothing selected
		_ui.ParentBone->setCurrentIndex(-1);
	}

	bones.removeAt(0);

	_ui.Bones->addItems(bones);

	//Select the first property to make it clear it's the active page
	_ui.BonePropertyList->setCurrentRow(0);
}

StudioModelBonesPanel::~StudioModelBonesPanel() = default;

void StudioModelBonesPanel::UpdateQCString()
{
	const auto model = _asset->GetScene()->GetEntity()->GetModel();

	const auto header = model->GetStudioHeader();

	//TODO:
	/*
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
		*/
}

void StudioModelBonesPanel::OnDockPanelChanged(QWidget* current, QWidget* previous)
{
	_isActive = current == this;

	OnHightlightBoneChanged();
}

void StudioModelBonesPanel::OnBoneChanged(int index)
{
	const auto model = _asset->GetScene()->GetEntity()->GetModel();

	const auto header = model->GetStudioHeader();

	const auto bone = header->GetBone(index);

	{
		const QSignalBlocker boneName{_ui.BoneName};
		const QSignalBlocker parentBone{_ui.ParentBone};
		const QSignalBlocker boneFlags{_ui.BoneFlags};
		const QSignalBlocker positionX{_ui.PositionX};
		const QSignalBlocker positionY{_ui.PositionY};
		const QSignalBlocker positionZ{_ui.PositionZ};
		const QSignalBlocker positionScaleX{_ui.PositionScaleX};
		const QSignalBlocker positionScaleY{_ui.PositionScaleY};
		const QSignalBlocker positionScaleZ{_ui.PositionScaleZ};
		const QSignalBlocker rotationX{_ui.RotationX};
		const QSignalBlocker rotationY{_ui.RotationY};
		const QSignalBlocker rotationZ{_ui.RotationZ};
		const QSignalBlocker rotationScaleX{_ui.RotationScaleX};
		const QSignalBlocker rotationScaleY{_ui.RotationScaleY};
		const QSignalBlocker rotationScaleZ{_ui.RotationScaleZ};

		_ui.BoneName->setText(bone->name);
		_ui.ParentBone->setCurrentIndex(bone->parent + ParentBoneOffset);
		_ui.BoneFlags->setValue(bone->flags);

		_ui.PositionX->setValue(bone->value[0]);
		_ui.PositionY->setValue(bone->value[1]);
		_ui.PositionZ->setValue(bone->value[2]);

		_ui.PositionScaleX->setValue(bone->scale[0]);
		_ui.PositionScaleY->setValue(bone->scale[1]);
		_ui.PositionScaleZ->setValue(bone->scale[2]);

		_ui.RotationX->setValue(bone->value[3]);
		_ui.RotationY->setValue(bone->value[4]);
		_ui.RotationZ->setValue(bone->value[5]);

		_ui.RotationScaleX->setValue(bone->scale[3]);
		_ui.RotationScaleY->setValue(bone->scale[4]);
		_ui.RotationScaleZ->setValue(bone->scale[5]);
	}

	OnHightlightBoneChanged();
}

void StudioModelBonesPanel::OnHightlightBoneChanged()
{
	_asset->GetScene()->DrawSingleBoneIndex = (_isActive && _ui.HighlightBone->isChecked()) ? _ui.Bones->currentIndex() : -1;
}

void StudioModelBonesPanel::OnBoneNameChanged()
{
	const auto model = _asset->GetScene()->GetEntity()->GetModel();

	const auto header = model->GetStudioHeader();

	const auto bone = header->GetBone(_ui.Bones->currentIndex());

	_asset->AddUndoCommand(new ModelBoneRenameCommand(model, _ui.Bones->currentIndex(), bone->name, _ui.BoneName->text()));
}

void StudioModelBonesPanel::OnBoneParentChanged(int index)
{
	const auto model = _asset->GetScene()->GetEntity()->GetModel();

	const auto header = model->GetStudioHeader();

	const auto bone = header->GetBone(_ui.Bones->currentIndex());

	bone->parent = index - ParentBoneOffset;

	//TODO: mark model changed
}

void StudioModelBonesPanel::OnBoneFlagsChanged()
{
	const auto model = _asset->GetScene()->GetEntity()->GetModel();

	const auto header = model->GetStudioHeader();

	const auto bone = header->GetBone(_ui.Bones->currentIndex());

	bone->flags = _ui.BoneFlags->value();

	//TODO: mark model changed
}

void StudioModelBonesPanel::OnBonePropertyChanged()
{
	const auto model = _asset->GetScene()->GetEntity()->GetModel();

	const auto header = model->GetStudioHeader();

	const auto bone = header->GetBone(_ui.Bones->currentIndex());

	bone->value[0] = _ui.PositionX->value();
	bone->value[1] = _ui.PositionY->value();
	bone->value[2] = _ui.PositionZ->value();

	bone->scale[0] = _ui.PositionScaleX->value();
	bone->scale[1] = _ui.PositionScaleY->value();
	bone->scale[2] = _ui.PositionScaleZ->value();

	bone->value[3] = _ui.RotationX->value();
	bone->value[4] = _ui.RotationY->value();
	bone->value[5] = _ui.RotationZ->value();

	bone->scale[3] = _ui.RotationScaleX->value();
	bone->scale[4] = _ui.RotationScaleY->value();
	bone->scale[5] = _ui.RotationScaleZ->value();

	//TODO: mark model changed
}
}
