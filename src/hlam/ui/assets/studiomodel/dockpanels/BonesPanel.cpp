#include <limits>

#include <QSignalBlocker>
#include <QToolTip>

#include "entity/HLMVStudioModelEntity.hpp"

#include "ui/StateSnapshot.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelData.hpp"
#include "ui/assets/studiomodel/StudioModelUndoCommands.hpp"
#include "ui/assets/studiomodel/StudioModelValidators.hpp"
#include "ui/assets/studiomodel/dockpanels/BonesPanel.hpp"

#include "ui/settings/StudioModelSettings.hpp"

namespace ui::assets::studiomodel
{
//Parent indices are offset by one so -1 becomes 0, 0 becomes 1, etc
constexpr int ParentBoneOffset = 1;
constexpr int BoneControllerOffset = 1;

static void SyncBonePropertiesToUI(const studiomdl::Bone& bone, Ui_BonesPanel& ui)
{
	const QSignalBlocker positionX{ui.PositionX};
	const QSignalBlocker positionY{ui.PositionY};
	const QSignalBlocker positionZ{ui.PositionZ};
	const QSignalBlocker positionScaleX{ui.PositionScaleX};
	const QSignalBlocker positionScaleY{ui.PositionScaleY};
	const QSignalBlocker positionScaleZ{ui.PositionScaleZ};
	const QSignalBlocker rotationX{ui.RotationX};
	const QSignalBlocker rotationY{ui.RotationY};
	const QSignalBlocker rotationZ{ui.RotationZ};
	const QSignalBlocker rotationScaleX{ui.RotationScaleX};
	const QSignalBlocker rotationScaleY{ui.RotationScaleY};
	const QSignalBlocker rotationScaleZ{ui.RotationScaleZ};

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

BonesPanel::BonesPanel(StudioModelAsset* asset)
	: _asset(asset)
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

	connect(_asset, &StudioModelAsset::AssetChanged, this, &BonesPanel::OnAssetChanged);
	connect(_asset, &StudioModelAsset::SaveSnapshot, this, &BonesPanel::OnSaveSnapshot);
	connect(_asset, &StudioModelAsset::LoadSnapshot, this, &BonesPanel::OnLoadSnapshot);

	connect(_ui.Bones, qOverload<int>(&QComboBox::currentIndexChanged), this, &BonesPanel::OnBoneChanged);
	connect(_ui.HighlightBone, &QCheckBox::stateChanged, this, &BonesPanel::OnHightlightBoneChanged);

	connect(_ui.BonePropertyList, qOverload<int>(&QListWidget::currentRowChanged), _ui.BonePropertyStack, &QStackedWidget::setCurrentIndex);

	connect(_ui.BoneName, &QLineEdit::textChanged, this, &BonesPanel::OnBoneNameChanged);
	connect(_ui.BoneName, &QLineEdit::inputRejected, this, &BonesPanel::OnBoneNameRejected);

	connect(_ui.ParentBone, qOverload<int>(&QComboBox::currentIndexChanged), this, &BonesPanel::OnBoneParentChanged);
	connect(_ui.BoneFlags, qOverload<int>(&QSpinBox::valueChanged), this, &BonesPanel::OnBoneFlagsChanged);

	connect(_ui.PositionX, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &BonesPanel::OnBonePropertyChanged);
	connect(_ui.PositionY, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &BonesPanel::OnBonePropertyChanged);
	connect(_ui.PositionZ, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &BonesPanel::OnBonePropertyChanged);

	connect(_ui.PositionScaleX, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &BonesPanel::OnBonePropertyChanged);
	connect(_ui.PositionScaleY, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &BonesPanel::OnBonePropertyChanged);
	connect(_ui.PositionScaleZ, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &BonesPanel::OnBonePropertyChanged);

	connect(_ui.RotationX, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &BonesPanel::OnBonePropertyChanged);
	connect(_ui.RotationY, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &BonesPanel::OnBonePropertyChanged);
	connect(_ui.RotationZ, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &BonesPanel::OnBonePropertyChanged);

	connect(_ui.RotationScaleX, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &BonesPanel::OnBonePropertyChanged);
	connect(_ui.RotationScaleY, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &BonesPanel::OnBonePropertyChanged);
	connect(_ui.RotationScaleZ, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &BonesPanel::OnBonePropertyChanged);

	connect(_ui.BoneControllerAxis, qOverload<int>(&QComboBox::currentIndexChanged), this, &BonesPanel::OnBoneControllerAxisChanged);
	connect(_ui.BoneController, qOverload<int>(&QComboBox::currentIndexChanged), this, &BonesPanel::OnBoneControllerChanged);

	OnAssetChanged(nullptr);
}

BonesPanel::~BonesPanel() = default;

void BonesPanel::UpdateRootBonesCount()
{
	const auto model = _asset->GetEntity()->GetEditableModel();

	int count = 0;

	for (const auto& bone : model->Bones)
	{
		if (!bone->Parent)
		{
			++count;
		}
	}

	_ui.RootBonesCount->setText(QString::number(count));
}

void BonesPanel::OnAssetChanged(StudioModelAsset* asset)
{
	auto modelData = asset ? asset->GetModelData() : StudioModelData::GetEmptyModel();

	//Set up this list first so when the first bone is selected by _ui.Bones->setModel it has everything set up properly
	{
		const QSignalBlocker blocker{_ui.ParentBone};
		_ui.ParentBone->setModel(modelData->BonesWithNone);
		//Start off with nothing selected
		_ui.ParentBone->setCurrentIndex(-1);
	}

	_ui.Bones->setModel(modelData->Bones);

	//Select the first property to make it clear it's the active page
	_ui.BonePropertyList->setCurrentRow(0);

	{
		const QSignalBlocker blocker{_ui.BoneController};
		_ui.BoneController->setModel(modelData->BoneControllersWithNone);
		_ui.BoneController->setCurrentIndex(0);
	}

	this->setEnabled(_ui.Bones->count() > 0);

	UpdateRootBonesCount();

	if (_previousModelData)
	{
		_previousModelData->DisconnectFromAll(this);
	}

	_previousModelData = modelData;

	connect(modelData->Bones, &QAbstractItemModel::dataChanged, this, &BonesPanel::SyncBoneName);

	connect(modelData, &StudioModelData::BoneParentChanged, this, [this](int index)
		{
			if (_ui.Bones->currentIndex() == index)
			{
				const QSignalBlocker parentBone{_ui.ParentBone};
				const auto& bone = *_asset->GetEditableStudioModel()->Bones[index];
				_ui.ParentBone->setCurrentIndex(bone.Parent ? (bone.Parent->ArrayIndex + ParentBoneOffset) : 0);
			}

			UpdateRootBonesCount();
		});

	connect(modelData, &StudioModelData::BoneFlagsChanged, this, [this](int index)
		{
			if (_ui.Bones->currentIndex() == index)
			{
				const QSignalBlocker flags{_ui.BoneFlags};
				_ui.BoneFlags->setValue(_asset->GetEditableStudioModel()->Bones[index]->Flags);
			}
		});

	connect(modelData, &StudioModelData::BonePropertiesChanged, this, [this](int index)
		{
			if (_ui.Bones->currentIndex() == index)
			{
				SyncBonePropertiesToUI(*_asset->GetEditableStudioModel()->Bones[index], _ui);
			}
		});

	connect(modelData, &StudioModelData::BoneControllerChangedFromBone, this, [this](int controllerIndex, int boneIndex)
		{
			if (_ui.Bones->currentIndex() == controllerIndex && boneIndex == _ui.BoneControllerAxis->currentIndex())
			{
				const QSignalBlocker controller{_ui.BoneController};
				_ui.BoneController->setCurrentIndex(boneIndex + BoneControllerOffset);
			}
		});

	connect(modelData, &StudioModelData::BoneControllerChangedFromController, this, [this]()
		{
			//Resync any changes
			OnBoneControllerAxisChanged(_ui.BoneControllerAxis->currentIndex());
		});

	const auto genericChangeHandler = [this]()
	{
		OnBoneChanged(_ui.Bones->currentIndex());
	};

	connect(modelData, &StudioModelData::ModelOriginChanged, this, genericChangeHandler);
	connect(modelData, &StudioModelData::ModelScaleChanged, this, genericChangeHandler);
	connect(modelData, &StudioModelData::ModelRotationChanged, this, genericChangeHandler);
}

void BonesPanel::OnSaveSnapshot(StateSnapshot* snapshot)
{
	if (const int index = _ui.Bones->currentIndex(); index != -1)
	{
		const auto model = _asset->GetEntity()->GetEditableModel();

		snapshot->SetValue("bones.bone", QString::fromStdString(model->Bones[index]->Name));
	}
}

void BonesPanel::OnLoadSnapshot(StateSnapshot* snapshot)
{
	if (auto bone = snapshot->Value("bones.bone"); bone.isValid())
	{
		const auto boneName = bone.toString().toStdString();

		const auto model = _asset->GetEntity()->GetEditableModel();

		if (auto it = std::find_if(model->Bones.begin(), model->Bones.end(), [&](const auto& bone)
			{
				return bone->Name == boneName;
			}); it != model->Bones.end())
		{
			const auto index = it - model->Bones.begin();

			_ui.Bones->setCurrentIndex(index);
		}
	}
}

void BonesPanel::OnBoneChanged(int index)
{
	const auto model = _asset->GetEntity()->GetEditableModel();

	const bool isValidBone = index != -1;

	const studiomdl::Bone emptyBone{};

	const auto& bone = isValidBone ? *model->Bones[index] : emptyBone;

	{
		const QSignalBlocker boneName{_ui.BoneName};
		const QSignalBlocker parentBone{_ui.ParentBone};
		const QSignalBlocker boneFlags{_ui.BoneFlags};
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

void BonesPanel::OnHightlightBoneChanged()
{
	_asset->GetProvider()->GetStudioModelSettings()->DrawSingleBoneIndex = _ui.HighlightBone->isChecked() ? _ui.Bones->currentIndex() : -1;
}

void BonesPanel::SyncBoneName(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
	if (topLeft.row() <= _ui.Bones->currentIndex() && _ui.Bones->currentIndex() <= bottomRight.row())
	{
		const auto model = _asset->GetEntity()->GetEditableModel();
		const QString text{model->Bones[topLeft.row()]->Name.c_str()};

		//Avoid resetting the edit position
		if (_ui.BoneName->text() != text)
		{
			const QSignalBlocker boneName{_ui.BoneName};
			_ui.BoneName->setText(text);
		}
	}
}

void BonesPanel::OnBoneNameChanged()
{
	const auto model = _asset->GetEntity()->GetEditableModel();
	const auto& bone = *model->Bones[_ui.Bones->currentIndex()];

	_asset->AddUndoCommand(new BoneRenameCommand(_asset, _ui.Bones->currentIndex(), bone.Name.c_str(), _ui.BoneName->text()));
}

void BonesPanel::OnBoneNameRejected()
{
	QToolTip::showText(_ui.BoneName->mapToGlobal({0, -20}), "Bone names must be unique");
}

void BonesPanel::OnBoneParentChanged(int index)
{
	// TODO: prevent setting self as parent
	const auto model = _asset->GetEntity()->GetEditableModel();
	const auto& bone = *model->Bones[_ui.Bones->currentIndex()];

	_asset->AddUndoCommand(new ChangeBoneParentCommand(_asset, _ui.Bones->currentIndex(), bone.Parent ? bone.Parent->ArrayIndex : -1, index - ParentBoneOffset));
}

void BonesPanel::OnBoneFlagsChanged()
{
	const auto model = _asset->GetEntity()->GetEditableModel();
	const auto& bone = *model->Bones[_ui.Bones->currentIndex()];

	_asset->AddUndoCommand(new ChangeBoneFlagsCommand(_asset, _ui.Bones->currentIndex(), bone.Flags, _ui.BoneFlags->value()));
}

void BonesPanel::OnBonePropertyChanged()
{
	const auto model = _asset->GetEntity()->GetEditableModel();
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

void BonesPanel::OnBoneControllerAxisChanged(int index)
{
	const QSignalBlocker controller{_ui.BoneController};

	const int boneIndex = _ui.Bones->currentIndex();

	const bool isValidBone = boneIndex != -1;

	const auto model = _asset->GetEntity()->GetEditableModel();

	const studiomdl::Bone emptyBone{};

	const auto& bone = isValidBone  ? *model->Bones[boneIndex] : emptyBone;

	if (isValidBone && index != -1 && bone.Axes[index].Controller)
	{
		_ui.BoneController->setCurrentIndex(bone.Axes[index].Controller->ArrayIndex + BoneControllerOffset);
	}
	else
	{
		_ui.BoneController->setCurrentIndex(0);
	}
}

void BonesPanel::OnBoneControllerChanged(int index)
{
	const auto model = _asset->GetEntity()->GetEditableModel();
	const auto& bone = *model->Bones[_ui.Bones->currentIndex()];

	const int axis = _ui.BoneControllerAxis->currentIndex();

	_asset->AddUndoCommand(new ChangeBoneControllerFromBoneCommand(_asset, _ui.Bones->currentIndex(), axis,
		bone.Axes[axis].Controller ? bone.Axes[axis].Controller->ArrayIndex : -1, index - BoneControllerOffset));
}
}
