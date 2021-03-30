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
constexpr int BoneControllerOffset = 1;

static void SyncBonePropertiesToUI(const studiomdl::Bone& bone, Ui_StudioModelBonesPanel& ui)
{
	ui.PositionX->setValue(bone.Axes[0].Value);
	ui.PositionY->setValue(bone.Axes[1].Value);
	ui.PositionZ->setValue(bone.Axes[2].Value);

	ui.PositionScaleX->setValue(bone.Axes[0].Scale);
	ui.PositionScaleY->setValue(bone.Axes[1].Scale);
	ui.PositionScaleZ->setValue(bone.Axes[2].Scale);

	ui.RotationX->setValue(bone.Axes[3].Value);
	ui.RotationY->setValue(bone.Axes[4].Value);
	ui.RotationZ->setValue(bone.Axes[5].Value);

	ui.RotationScaleX->setValue(bone.Axes[3].Scale);
	ui.RotationScaleY->setValue(bone.Axes[4].Scale);
	ui.RotationScaleZ->setValue(bone.Axes[5].Scale);
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
		spinBox->setRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
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

	connect(_ui.BoneControllerAxis, qOverload<int>(&QComboBox::currentIndexChanged), this, &StudioModelBonesPanel::OnBoneControllerAxisChanged);
	connect(_ui.BoneController, qOverload<int>(&QComboBox::currentIndexChanged), this, &StudioModelBonesPanel::OnBoneControllerChanged);

	const auto model = _asset->GetScene()->GetEntity()->GetEditableModel();

	this->setEnabled(!model->Bones.empty());

	QStringList bones;

	bones.reserve(model->Bones.size() + 1);

	bones.append("None (-1)");

	for (int i = 0; i < model->Bones.size(); ++i)
	{
		bones.append(QString{"%1 (%2)"}.arg(model->Bones[i]->Name.c_str()).arg(i));
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

	QStringList boneControllers;

	boneControllers.append("None (-1)");

	boneControllers.reserve(model->BoneControllers.size() + 1);

	for (int i = 0; i < model->BoneControllers.size(); ++i)
	{
		boneControllers.append(QString{"Controller %1"}.arg(i + 1));
	}

	{
		const QSignalBlocker blocker{_ui.BoneController};

		_ui.BoneController->addItems(boneControllers);
		_ui.BoneController->setCurrentIndex(0);
	}
}

StudioModelBonesPanel::~StudioModelBonesPanel() = default;

void StudioModelBonesPanel::OnModelChanged(const ModelChangeEvent& event)
{
	const auto model = _asset->GetScene()->GetEntity()->GetEditableModel();

	switch (event.GetId())
	{
	case ModelChangeId::RenameBone:
	{
		const QSignalBlocker bones{_ui.Bones};
		const QSignalBlocker parentBone{_ui.ParentBone};

		const auto& listChange{static_cast<const ModelListChangeEvent&>(event)};

		const auto& bone = *model->Bones[listChange.GetSourceIndex()];

		const auto newName{QString{"%1 (%2)"}.arg(bone.Name.c_str()).arg(listChange.GetSourceIndex())};

		_ui.Bones->setItemText(listChange.GetSourceIndex(), newName);
		
		if (_ui.Bones->currentIndex() == listChange.GetSourceIndex())
		{
			const QString text{bone.Name.c_str()};

			//Avoid resetting the edit position
			if (_ui.BoneName->text() != text)
			{
				const QSignalBlocker boneName{_ui.BoneName};
				_ui.BoneName->setText(bone.Name.c_str());
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

			const auto& bone = *model->Bones[listChange.GetSourceIndex()];

			_ui.ParentBone->setCurrentIndex(bone.Parent ? (bone.Parent->ArrayIndex + ParentBoneOffset) : 0);
		}
		break;
	}

	case ModelChangeId::ChangeBoneFlags:
	{
		const auto& listChange{static_cast<const ModelListChangeEvent&>(event)};

		if (_ui.Bones->currentIndex() == listChange.GetSourceIndex())
		{
			const QSignalBlocker flags{_ui.BoneFlags};

			const auto& bone = *model->Bones[listChange.GetSourceIndex()];

			_ui.BoneFlags->setValue(bone.Flags);
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

			const auto& bone = *model->Bones[listChange.GetSourceIndex()];
			
			SyncBonePropertiesToUI(bone, _ui);
		}
		break;
	}

	case ModelChangeId::ChangeBoneControllerFromBone:
	{
		const auto& listChange{static_cast<const ModelBoneControllerFromBoneChangeEvent&>(event)};

		if (_ui.Bones->currentIndex() == listChange.GetSourceIndex() && listChange.GetValue().first == _ui.BoneControllerAxis->currentIndex())
		{
			const QSignalBlocker controller{_ui.BoneController};
			_ui.BoneController->setCurrentIndex(listChange.GetValue().second + BoneControllerOffset);
		}
		break;
	}

	case ModelChangeId::ChangeBoneControllerFromController:
	{
		//Resync any changes
		OnBoneControllerAxisChanged(_ui.BoneControllerAxis->currentIndex());
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
	const auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	const auto& bone = *model->Bones[index];

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
		const QSignalBlocker controllerAxis{_ui.BoneControllerAxis};

		_ui.BoneName->setText(bone.Name.c_str());
		_ui.ParentBone->setCurrentIndex(bone.Parent ? (bone.Parent->ArrayIndex + ParentBoneOffset) : 0);
		_ui.BoneFlags->setValue(bone.Flags);

		SyncBonePropertiesToUI(bone, _ui);

		//Ensure axis initializes to index 0
		_ui.BoneControllerAxis->setCurrentIndex(0);
		OnBoneControllerAxisChanged(_ui.BoneControllerAxis->currentIndex());
	}

	OnHightlightBoneChanged();
}

void StudioModelBonesPanel::OnHightlightBoneChanged()
{
	_asset->GetScene()->DrawSingleBoneIndex = (_isActive && _ui.HighlightBone->isChecked()) ? _ui.Bones->currentIndex() : -1;
}

void StudioModelBonesPanel::OnBoneNameChanged()
{
	const auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	const auto& bone = *model->Bones[_ui.Bones->currentIndex()];

	_asset->AddUndoCommand(new BoneRenameCommand(_asset, _ui.Bones->currentIndex(), bone.Name.c_str(), _ui.BoneName->text()));
}

void StudioModelBonesPanel::OnBoneNameRejected()
{
	QToolTip::showText(_ui.BoneName->mapToGlobal({0, -20}), "Bone names must be unique");
}

void StudioModelBonesPanel::OnBoneParentChanged(int index)
{
	const auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	const auto& bone = *model->Bones[_ui.Bones->currentIndex()];

	_asset->AddUndoCommand(new ChangeBoneParentCommand(_asset, _ui.Bones->currentIndex(), bone.Parent ? bone.Parent->ArrayIndex : -1, index - ParentBoneOffset));
}

void StudioModelBonesPanel::OnBoneFlagsChanged()
{
	const auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	const auto& bone = *model->Bones[_ui.Bones->currentIndex()];

	_asset->AddUndoCommand(new ChangeBoneFlagsCommand(_asset, _ui.Bones->currentIndex(), bone.Flags, _ui.BoneFlags->value()));
}

void StudioModelBonesPanel::OnBonePropertyChanged()
{
	const auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	const auto& bone = *model->Bones[_ui.Bones->currentIndex()];

	_asset->AddUndoCommand(new ChangeBonePropertyCommand(_asset, _ui.Bones->currentIndex(),
		{
			{
				glm::vec3{bone.Axes[0].Value, bone.Axes[1].Value, bone.Axes[2].Value},
				glm::vec3{bone.Axes[3].Value, bone.Axes[4].Value, bone.Axes[5].Value}
			},
			{
				glm::vec3{bone.Axes[0].Scale, bone.Axes[1].Scale, bone.Axes[2].Scale},
				glm::vec3{bone.Axes[3].Scale, bone.Axes[4].Scale, bone.Axes[5].Scale}
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

void StudioModelBonesPanel::OnBoneControllerAxisChanged(int index)
{
	const QSignalBlocker controller{_ui.BoneController};

	const auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	const auto& bone = *model->Bones[_ui.Bones->currentIndex()];

	if (index != -1 && bone.Axes[index].Controller)
	{
		_ui.BoneController->setCurrentIndex(bone.Axes[index].Controller->ArrayIndex + BoneControllerOffset);
	}
	else
	{
		_ui.BoneController->setCurrentIndex(0);
	}
}

void StudioModelBonesPanel::OnBoneControllerChanged(int index)
{
	const auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	const auto& bone = *model->Bones[_ui.Bones->currentIndex()];

	const int axis = _ui.BoneControllerAxis->currentIndex();

	_asset->AddUndoCommand(new ChangeBoneControllerFromBoneCommand(_asset, _ui.Bones->currentIndex(), axis,
		bone.Axes[axis].Controller ? bone.Axes[axis].Controller->ArrayIndex : -1, index - BoneControllerOffset));
}
}
