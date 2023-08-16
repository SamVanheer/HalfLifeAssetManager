#include <limits>

#include <QSignalBlocker>
#include <QToolTip>

#include "entity/HLMVStudioModelEntity.hpp"

#include "ui/StateSnapshot.hpp"

#include "plugins/halflife/studiomodel/StudioModelAsset.hpp"
#include "plugins/halflife/studiomodel/ui/StudioModelData.hpp"
#include "plugins/halflife/studiomodel/ui/StudioModelUndoCommands.hpp"
#include "plugins/halflife/studiomodel/ui/StudioModelValidators.hpp"
#include "plugins/halflife/studiomodel/ui/dockpanels/BonesPanel.hpp"

namespace studiomodel
{
//Parent indices are offset by one so -1 becomes 0, 0 becomes 1, etc
constexpr int ParentBoneOffset = 1;
constexpr int BoneControllerOffset = 1;

BonesPanel::BonesPanel(StudioModelAssetProvider* provider)
	: _provider(provider)
{
	_ui.setupUi(this);

	const auto boneNameValidator = new UniqueBoneNameValidator(MaxBoneNameBytes - 1, _provider, this);

	_ui.BoneName->setValidator(boneNameValidator);

	connect(_ui.Bones, qOverload<int>(&QComboBox::currentIndexChanged), boneNameValidator, &UniqueBoneNameValidator::SetCurrentIndex);

	connect(_provider, &StudioModelAssetProvider::AssetChanged, this, &BonesPanel::OnAssetChanged);

	connect(_ui.Bones, qOverload<int>(&QComboBox::currentIndexChanged), this, &BonesPanel::OnBoneChanged);
	connect(_ui.HighlightBone, &QCheckBox::stateChanged, this, &BonesPanel::OnHightlightBoneChanged);

	connect(_ui.BoneName, &QLineEdit::textChanged, this, &BonesPanel::OnPropsChanged);
	connect(_ui.BoneName, &QLineEdit::inputRejected, this, &BonesPanel::OnBoneNameRejected);

	connect(_ui.Position, &qt::widgets::ShortVector3Edit::ValueChanged, this, &BonesPanel::OnPropsChanged);
	connect(_ui.PositionScale, &qt::widgets::ShortVector3Edit::ValueChanged, this, &BonesPanel::OnPropsChanged);
	connect(_ui.Rotation, &qt::widgets::ShortVector3Edit::ValueChanged, this, &BonesPanel::OnPropsChanged);
	connect(_ui.RotationScale, &qt::widgets::ShortVector3Edit::ValueChanged, this, &BonesPanel::OnPropsChanged);

	OnAssetChanged(_provider->GetDummyAsset());
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
	if (_asset)
	{
		_asset->disconnect(this);
	}

	_asset = asset;

	auto modelData = _asset->GetModelData();

	connect(_asset, &StudioModelAsset::SaveSnapshot, this, &BonesPanel::OnSaveSnapshot);
	connect(_asset, &StudioModelAsset::LoadSnapshot, this, &BonesPanel::OnLoadSnapshot);

	_ui.Bones->setModel(modelData->Bones);

	this->setEnabled(_ui.Bones->count() > 0);

	UpdateRootBonesCount();

	if (_previousModelData)
	{
		_previousModelData->DisconnectFromAll(this);
	}

	_previousModelData = modelData;

	connect(modelData->Bones, &QAbstractItemModel::dataChanged, this, [this](const QModelIndex& topLeft, const QModelIndex& bottomRight)
		{
			if (topLeft.row() <= _ui.Bones->currentIndex() && _ui.Bones->currentIndex() <= bottomRight.row())
			{
				const auto model = _asset->GetEntity()->GetEditableModel();
				const QString text{QString::fromStdString(model->Bones[topLeft.row()]->Name)};

				//Avoid resetting the edit position
				if (_ui.BoneName->text() != text)
				{
					const QSignalBlocker boneName{_ui.BoneName};
					_ui.BoneName->setText(text);
				}
			}

			UpdateRootBonesCount();
		});

	connect(modelData, &StudioModelData::BoneDataChanged, this, [this](int index)
		{
			if (_ui.Bones->currentIndex() == index)
			{
				OnBoneChanged(index);
			}

			UpdateRootBonesCount();
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
	// Don't refresh the UI if this is getting called in response to a change we made.
	if (_changingBoneProperties)
	{
		return;
	}

	const auto model = _asset->GetEntity()->GetEditableModel();

	const bool isValidBone = index != -1;

	const studiomdl::StudioBone emptyBone{};

	const auto& bone = isValidBone ? *model->Bones[index] : emptyBone;

	{
		const QSignalBlocker boneName{_ui.BoneName};
		const QSignalBlocker parentBone{_ui.ParentBone};

		_ui.BoneName->setText(QString::fromStdString(bone.Name));

		if (bone.Parent)
		{
			_ui.ParentBone->setText(QString{"%1 (%2)"}
				.arg(QString::fromStdString(bone.Parent->Name))
				.arg(bone.Parent->ArrayIndex));
		}
		else
		{
			_ui.ParentBone->clear();
		}

		const QSignalBlocker position{_ui.Position};
		const QSignalBlocker positionScale{_ui.PositionScale};
		const QSignalBlocker rotation{_ui.Rotation};
		const QSignalBlocker rotationScale{_ui.RotationScale};

		_ui.Position->SetValue(glm::vec3{bone.Axes[0].Value, bone.Axes[1].Value, bone.Axes[2].Value});
		_ui.PositionScale->SetValue(glm::vec3{bone.Axes[0].Scale, bone.Axes[1].Scale, bone.Axes[2].Scale});
		_ui.Rotation->SetValue(glm::vec3{bone.Axes[3].Value, bone.Axes[4].Value, bone.Axes[5].Value});
		_ui.RotationScale->SetValue(glm::vec3{bone.Axes[3].Scale, bone.Axes[4].Scale, bone.Axes[5].Scale});
	}

	OnHightlightBoneChanged();
}

void BonesPanel::OnHightlightBoneChanged()
{
	_asset->DrawSingleBoneIndex = _ui.HighlightBone->isChecked() ? _ui.Bones->currentIndex() : -1;
}

void BonesPanel::OnBoneNameRejected()
{
	QToolTip::showText(_ui.BoneName->mapToGlobal({0, -20}), "Bone names must be unique");
}

void BonesPanel::OnPropsChanged()
{
	const auto model = _asset->GetEntity()->GetEditableModel();
	const auto& bone = *model->Bones[_ui.Bones->currentIndex()];

	const BoneProps oldProps
	{
		.Name = bone.Name,
		.Values =
		{
			glm::vec3{bone.Axes[0].Value, bone.Axes[1].Value, bone.Axes[2].Value},
			glm::vec3{bone.Axes[3].Value, bone.Axes[4].Value, bone.Axes[5].Value}
		},
		.Scales = 
		{
			glm::vec3{bone.Axes[0].Scale, bone.Axes[1].Scale, bone.Axes[2].Scale},
			glm::vec3{bone.Axes[3].Scale, bone.Axes[4].Scale, bone.Axes[5].Scale}
		}
	};

	const BoneProps newProps
	{
		.Name = _ui.BoneName->text().toStdString(),
		.Values = {_ui.Position->GetValue(), _ui.Rotation->GetValue()},
		.Scales = {_ui.PositionScale->GetValue(), _ui.RotationScale->GetValue()}
	};

	_changingBoneProperties = true;
	_asset->AddUndoCommand(new ChangeBonePropsCommand(_asset, _ui.Bones->currentIndex(), oldProps, newProps));
	_changingBoneProperties = false;
}
}
