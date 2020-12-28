#include <limits>

#include <QSignalBlocker>
#include <QToolTip>

#include "entity/HLMVStudioModelEntity.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelUndoCommands.hpp"
#include "ui/assets/studiomodel/StudioModelValidators.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelBonesPanel.hpp"

namespace ui::assets::studiomodel
{
//Parent indices are offset by one so -1 becomes 0, 0 becomes 1, etc
constexpr int ParentBoneOffset = 1;

static void SyncBonePropertiesToUI(const mstudiobone_t& bone, Ui_StudioModelBonesPanel& ui)
{
	ui.PositionX->setValue(bone.value[0]);
	ui.PositionY->setValue(bone.value[1]);
	ui.PositionZ->setValue(bone.value[2]);

	ui.PositionScaleX->setValue(bone.scale[0]);
	ui.PositionScaleY->setValue(bone.scale[1]);
	ui.PositionScaleZ->setValue(bone.scale[2]);

	ui.RotationX->setValue(bone.value[3]);
	ui.RotationY->setValue(bone.value[4]);
	ui.RotationZ->setValue(bone.value[5]);

	ui.RotationScaleX->setValue(bone.scale[3]);
	ui.RotationScaleY->setValue(bone.scale[4]);
	ui.RotationScaleZ->setValue(bone.scale[5]);
}

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

	const auto boneNameValidator = new UniqueBoneNameValidator(MaxBoneNameBytes - 1, _asset, this);

	_ui.BoneName->setValidator(boneNameValidator);

	connect(_ui.Bones, qOverload<int>(&QComboBox::currentIndexChanged), boneNameValidator, &UniqueBoneNameValidator::SetCurrentIndex);

	connect(_asset, &StudioModelAsset::ModelChanged, this, &StudioModelBonesPanel::OnModelChanged);

	connect(_ui.Bones, qOverload<int>(&QComboBox::currentIndexChanged), this, &StudioModelBonesPanel::OnBoneChanged);
	connect(_ui.HighlightBone, &QCheckBox::stateChanged, this, &StudioModelBonesPanel::OnHightlightBoneChanged);

	connect(_ui.BonePropertyList, qOverload<int>(&QListWidget::currentRowChanged), _ui.BonePropertyStack, &QStackedWidget::setCurrentIndex);

	connect(_ui.BoneName, &QLineEdit::textChanged, this, &StudioModelBonesPanel::OnBoneNameChanged);
	connect(_ui.BoneName, &QLineEdit::inputRejected, this, &StudioModelBonesPanel::OnBoneNameRejected);

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

void StudioModelBonesPanel::OnModelChanged(const ModelChangeEvent& event)
{
	const auto model = _asset->GetScene()->GetEntity()->GetModel();

	const auto header = model->GetStudioHeader();

	switch (event.GetId())
	{
	case ModelChangeId::RenameBone:
	{
		const QSignalBlocker bones{_ui.Bones};
		const QSignalBlocker parentBone{_ui.ParentBone};

		const auto& listChange{static_cast<const ModelListChangeEvent&>(event)};

		const auto bone = header->GetBone(listChange.GetSourceIndex());

		const auto newName{QString{"%1 (%2)"}.arg(bone->name).arg(listChange.GetSourceIndex())};

		_ui.Bones->setItemText(listChange.GetSourceIndex(), newName);
		
		if (_ui.Bones->currentIndex() == listChange.GetSourceIndex())
		{
			const QString text{bone->name};

			//Avoid resetting the edit position
			if (_ui.BoneName->text() != text)
			{
				const QSignalBlocker boneName{_ui.BoneName};
				_ui.BoneName->setText(bone->name);
			}
		}

		_ui.ParentBone->setItemText(listChange.GetSourceIndex() + ParentBoneOffset, newName);
		break;
	}

	case ModelChangeId::ChangeBoneParent:
	{
		const auto& listChange{static_cast<const ModelListChangeEvent&>(event)};

		if (_ui.Bones->currentIndex() == listChange.GetSourceIndex())
		{
			const QSignalBlocker parentBone{_ui.ParentBone};

			const auto bone = header->GetBone(listChange.GetSourceIndex());

			_ui.ParentBone->setCurrentIndex(bone->parent + ParentBoneOffset);
		}
		break;
	}

	case ModelChangeId::ChangeBoneFlags:
	{
		const auto& listChange{static_cast<const ModelListChangeEvent&>(event)};

		if (_ui.Bones->currentIndex() == listChange.GetSourceIndex())
		{
			const QSignalBlocker flags{_ui.BoneFlags};

			const auto bone = header->GetBone(listChange.GetSourceIndex());

			_ui.BoneFlags->setValue(bone->flags);
		}
		break;
	}

	case ModelChangeId::ChangeBoneProperty:
	{
		const auto& listChange{static_cast<const ModelListChangeEvent&>(event)};

		if (_ui.Bones->currentIndex() == listChange.GetSourceIndex())
		{
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

			const auto bone = header->GetBone(listChange.GetSourceIndex());
			
			SyncBonePropertiesToUI(*bone, _ui);
		}
		break;
	}
	}
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

		SyncBonePropertiesToUI(*bone, _ui);
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

	_asset->AddUndoCommand(new BoneRenameCommand(_asset, _ui.Bones->currentIndex(), bone->name, _ui.BoneName->text()));
}

void StudioModelBonesPanel::OnBoneNameRejected()
{
	QToolTip::showText(_ui.BoneName->mapToGlobal({0, -20}), "Bone names must be unique");
}

void StudioModelBonesPanel::OnBoneParentChanged(int index)
{
	const auto model = _asset->GetScene()->GetEntity()->GetModel();

	const auto header = model->GetStudioHeader();

	const auto bone = header->GetBone(_ui.Bones->currentIndex());

	_asset->AddUndoCommand(new ChangeBoneParentCommand(_asset, _ui.Bones->currentIndex(), bone->parent, index - ParentBoneOffset));
}

void StudioModelBonesPanel::OnBoneFlagsChanged()
{
	const auto model = _asset->GetScene()->GetEntity()->GetModel();

	const auto header = model->GetStudioHeader();

	const auto bone = header->GetBone(_ui.Bones->currentIndex());

	_asset->AddUndoCommand(new ChangeBoneFlagsCommand(_asset, _ui.Bones->currentIndex(), bone->flags, _ui.BoneFlags->value()));
}

void StudioModelBonesPanel::OnBonePropertyChanged()
{
	const auto model = _asset->GetScene()->GetEntity()->GetModel();

	const auto header = model->GetStudioHeader();

	const auto bone = header->GetBone(_ui.Bones->currentIndex());

	_asset->AddUndoCommand(new ChangeBonePropertyCommand(_asset, _ui.Bones->currentIndex(),
		{
			{
				glm::vec3{bone->value[0], bone->value[1], bone->value[2]},
				glm::vec3{bone->value[3], bone->value[4], bone->value[5]}
			},
			{
				glm::vec3{bone->scale[0], bone->scale[1], bone->scale[2]},
				glm::vec3{bone->scale[3], bone->scale[4], bone->scale[5]}
			}
		},
		{
			{
				glm::vec3{_ui.PositionX->value(), _ui.PositionY->value(), _ui.PositionZ->value()},
				glm::vec3{_ui.RotationX->value(), _ui.RotationY->value(), _ui.RotationZ->value()}
			},
			{
				glm::vec3{_ui.PositionScaleX->value(), _ui.PositionScaleY->value(), _ui.PositionScaleZ->value()},
				glm::vec3{_ui.RotationScaleX->value(), _ui.RotationScaleY->value(), _ui.RotationScaleZ->value()}
			}
		}));
}
}
