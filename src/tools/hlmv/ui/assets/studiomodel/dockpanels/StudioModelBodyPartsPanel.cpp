#include <cmath>
#include <limits>

#include <QMessageBox>
#include <QSignalBlocker>

#include "entity/CHLMVStudioModelEntity.h"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelUndoCommands.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelBodyPartsPanel.hpp"

namespace ui::assets::studiomodel
{
StudioModelBodyPartsPanel::StudioModelBodyPartsPanel(StudioModelAsset* asset, QWidget* parent)
	: QWidget(parent)
	, _asset(asset)
{
	_ui.setupUi(this);

	_ui.BoneControllerRest->setRange(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());

	QDoubleSpinBox* const spinBoxes[] =
	{
		_ui.BoneControllerStart,
		_ui.BoneControllerEnd
	};

	for (auto spinBox : spinBoxes)
	{
		spinBox->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
	}

	connect(_asset, &StudioModelAsset::ModelChanged, this, &StudioModelBodyPartsPanel::OnModelChanged);

	connect(_ui.BodyParts, qOverload<int>(&QComboBox::currentIndexChanged), this, &StudioModelBodyPartsPanel::OnBodyPartChanged);
	connect(_ui.Submodels, qOverload<int>(&QComboBox::currentIndexChanged), this, &StudioModelBodyPartsPanel::OnSubmodelChanged);
	connect(_ui.Skins, qOverload<int>(&QComboBox::currentIndexChanged), this, &StudioModelBodyPartsPanel::OnSkinChanged);
	connect(_ui.BoneControllers, qOverload<int>(&QComboBox::currentIndexChanged), this, &StudioModelBodyPartsPanel::OnBoneControllerChanged);
	connect(_ui.BoneControllerValueSlider, &QSlider::valueChanged, this, &StudioModelBodyPartsPanel::OnBoneControllerValueSliderChanged);
	connect(_ui.BoneControllerValueSpinner, qOverload<double>(&QDoubleSpinBox::valueChanged),
		this, &StudioModelBodyPartsPanel::OnBoneControllerValueSpinnerChanged);

	connect(_ui.BoneControllerBone, qOverload<int>(&QComboBox::currentIndexChanged), this, &StudioModelBodyPartsPanel::OnBoneControllerBoneChanged);
	connect(_ui.BoneControllerStart, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelBodyPartsPanel::OnBoneControllerRangeChanged);
	connect(_ui.BoneControllerEnd, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelBodyPartsPanel::OnBoneControllerRangeChanged);
	connect(_ui.BoneControllerRest, qOverload<int>(&QSpinBox::valueChanged), this, &StudioModelBodyPartsPanel::OnBoneControllerRestChanged);
	connect(_ui.BoneControllerIndex, qOverload<int>(&QComboBox::currentIndexChanged), this, &StudioModelBodyPartsPanel::OnBoneControllerIndexChanged);
	connect(_ui.BoneControllerType, qOverload<int>(&QComboBox::currentIndexChanged), this, &StudioModelBodyPartsPanel::OnBoneControllerTypeChanged);

	auto entity = _asset->GetScene()->GetEntity();
	auto model = entity->GetModel()->GetStudioHeader();
	auto textureHeader = entity->GetModel()->GetTextureHeader();

	{
		const QSignalBlocker blocker{_ui.BoneControllerBone};

		QStringList bones;

		bones.reserve(model->numbones);

		for (int i = 0; i < model->numbones; ++i)
		{
			bones.append(QString{"%1 (%2)"}.arg(model->GetBone(i)->name).arg(i));
		}

		_ui.BoneControllerBone->addItems(bones);

		//Start off with nothing selected
		_ui.BoneControllerBone->setCurrentIndex(-1);
	}

	if (model->numbodyparts > 0)
	{
		QStringList bodyParts;

		bodyParts.reserve(model->numbodyparts);

		for (int i = 0; i < model->numbodyparts; ++i)
		{
			bodyParts.append(model->GetBodypart(i)->name);
		}

		_ui.BodyParts->addItems(bodyParts);
	}
	else
	{
		_ui.BodyParts->setEnabled(false);
		_ui.Submodels->setEnabled(false);
	}

	QStringList skins;

	for (int i = 0; i < textureHeader->numskinfamilies; ++i)
	{
		skins.append(QString{"Skin %1"}.arg(i + 1));
	}

	_ui.Skins->addItems(skins);

	_ui.Skins->setEnabled(textureHeader->numskinfamilies > 0);

	if (model->numbonecontrollers > 0)
	{
		QStringList boneControllers;

		for (int i = 0; i < model->numbonecontrollers; ++i)
		{
			const auto boneController = model->GetBoneController(i);

			if (boneController->index == STUDIO_MOUTH_CONTROLLER)
			{
				boneControllers.append("Mouth");
			}
			else
			{
				boneControllers.append(QString{"Controller %1"}.arg(boneController->index));
			}
		}

		_ui.BoneControllers->addItems(boneControllers);
	}
	else
	{
		//Disable and center it
		_ui.BoneControllerValueSlider->setEnabled(false);
		_ui.BoneControllerValueSlider->setRange(0, 2);
		_ui.BoneControllerValueSlider->setValue(1);

		_ui.BoneControllerValueSpinner->setEnabled(false);
		_ui.BoneControllerValueSpinner->setRange(0, 1);
		_ui.BoneControllerValueSpinner->setValue(0);
	}

	_ui.BoneControllers->setEnabled(model->numbonecontrollers > 0);

	//Should already be set but if there are no body parts and/or submodels it won't have been
	_ui.BodyValue->setText(QString::number(entity->GetBodygroup()));
}

StudioModelBodyPartsPanel::~StudioModelBodyPartsPanel() = default;

void StudioModelBodyPartsPanel::OnModelChanged(const ModelChangeEvent& event)
{
	const auto model = _asset->GetScene()->GetEntity()->GetModel();

	const auto header = model->GetStudioHeader();

	switch (event.GetId())
	{
	case ModelChangeId::BoneRename:
	{
		const QSignalBlocker boneControllerBone{_ui.BoneControllerBone};

		const auto& listChange{static_cast<const ModelListChangeEvent&>(event)};

		const auto bone = header->GetBone(listChange.GetSourceIndex());

		_ui.BoneControllerBone->setItemText(listChange.GetSourceIndex(), QString{"%1 (%2)"}.arg(bone->name).arg(listChange.GetSourceIndex()));
		break;
	}
	}
}

void StudioModelBodyPartsPanel::UpdateControllerRange()
{
	auto entity = _asset->GetScene()->GetEntity();

	auto model = entity->GetModel()->GetStudioHeader();

	const auto boneController = model->GetBoneController(_ui.BoneControllers->currentIndex());

	float start, end;

	//Swap values if the range is inverted
	if (boneController->end < boneController->start)
	{
		start = boneController->end;
		end = boneController->start;
	}
	else
	{
		start = boneController->start;
		end = boneController->end;
	}

	//Should probably scale as needed so the range is sufficiently large
	//This prevents ranges that cover less than a whole integer from not doing anything
	if ((end - start) < 1.0f)
	{
		_controllerSliderScale = 100.0f;
	}
	else
	{
		_controllerSliderScale = 1.0f;
	}

	{
		//Don't let the changes ripple back to change the current setting, because this will result in a loss of accuracy due to casting to integer
		const QSignalBlocker slider{_ui.BoneControllerValueSlider};
		const QSignalBlocker spinner{_ui.BoneControllerValueSpinner};

		const double value = entity->GetControllerValue(_ui.BoneControllers->currentIndex());

		_ui.BoneControllerValueSlider->setRange((int)(start * _controllerSliderScale), (int)(end * _controllerSliderScale));
		_ui.BoneControllerValueSlider->setValue(static_cast<int>(value * _controllerSliderScale));

		_ui.BoneControllerValueSpinner->setRange(start, end);
		_ui.BoneControllerValueSpinner->setValue(value);
	}
}

void StudioModelBodyPartsPanel::OnBodyPartChanged(int index)
{
	auto entity = _asset->GetScene()->GetEntity();

	auto model = entity->GetModel()->GetStudioHeader();

	const auto bodyPart = model->GetBodypart(index);

	const bool hasSubmodels = bodyPart->nummodels > 0;

	{
		const QSignalBlocker blocker{_ui.Submodels};

		_ui.Submodels->clear();

		if (hasSubmodels)
		{
			QStringList submodels;

			for (int i = 0; i < bodyPart->nummodels; ++i)
			{
				submodels.append(QString{"Submodel %1"}.arg(i + 1));
			}

			_ui.Submodels->addItems(submodels);
		}
	}

	_ui.Submodels->setEnabled(hasSubmodels);

	if (hasSubmodels)
	{
		_ui.Submodels->setCurrentIndex(entity->GetBodyValueForGroup(index));
	}
}

void StudioModelBodyPartsPanel::OnSubmodelChanged(int index)
{
	auto entity = _asset->GetScene()->GetEntity();

	entity->SetBodygroup(_ui.BodyParts->currentIndex(), index);

	_ui.BodyValue->setText(QString::number(entity->GetBodygroup()));
}

void StudioModelBodyPartsPanel::OnSkinChanged(int index)
{
	auto entity = _asset->GetScene()->GetEntity();

	entity->SetSkin(index);
}

void StudioModelBodyPartsPanel::OnBoneControllerChanged(int index)
{
	auto entity = _asset->GetScene()->GetEntity();

	auto model = entity->GetModel()->GetStudioHeader();

	const auto boneController = model->GetBoneController(index);

	UpdateControllerRange();

	{
		const QSignalBlocker boneName{_ui.BoneControllerBone};
		const QSignalBlocker start{_ui.BoneControllerStart};
		const QSignalBlocker end{_ui.BoneControllerEnd};
		const QSignalBlocker rest{_ui.BoneControllerRest};
		const QSignalBlocker index{_ui.BoneControllerIndex};

		const QSignalBlocker controllerType{_ui.BoneControllerType};

		_ui.BoneControllerBone->setCurrentIndex(boneController->bone);
		_ui.BoneControllerStart->setValue(boneController->start);
		_ui.BoneControllerEnd->setValue(boneController->end);
		_ui.BoneControllerRest->setValue(boneController->rest);
		_ui.BoneControllerIndex->setCurrentIndex(boneController->index);

		const int type = boneController->type & STUDIO_BONECONTROLLER_TYPES;

		const int typeIndex = static_cast<int>(std::log2(type));

		_ui.BoneControllerType->setCurrentIndex(typeIndex);
	}
}

void StudioModelBodyPartsPanel::OnBoneControllerValueSliderChanged(int value)
{
	OnBoneControllerValueSpinnerChanged(value / _controllerSliderScale);
}

void StudioModelBodyPartsPanel::OnBoneControllerValueSpinnerChanged(double value)
{
	const int boneControllerLogicalIndex = _ui.BoneControllers->currentIndex();

	if (boneControllerLogicalIndex != -1)
	{
		auto entity = _asset->GetScene()->GetEntity();

		auto model = entity->GetModel()->GetStudioHeader();

		const auto boneController = model->GetBoneController(boneControllerLogicalIndex);

		//TODO: support multiple mouth controllers somehow.
		if (boneController->index == STUDIO_MOUTH_CONTROLLER)
		{
			entity->SetMouth(value);
		}
		else
		{
			entity->SetController(boneController->index, value);
		}
	}

	{
		const QSignalBlocker slider{_ui.BoneControllerValueSlider};
		const QSignalBlocker spinner{_ui.BoneControllerValueSpinner};

		_ui.BoneControllerValueSlider->setValue(static_cast<int>(value * _controllerSliderScale));
		_ui.BoneControllerValueSpinner->setValue(value);
	}
}

void StudioModelBodyPartsPanel::OnBoneControllerBoneChanged(int index)
{
	const int boneControllerLogicalIndex = _ui.BoneControllers->currentIndex();

	const auto entity = _asset->GetScene()->GetEntity();

	const auto model = entity->GetModel()->GetStudioHeader();

	const auto boneController = model->GetBoneController(boneControllerLogicalIndex);

	const int type = boneController->type & STUDIO_BONECONTROLLER_TYPES;

	const int typeIndex = static_cast<int>(std::log2(type));

	const auto newBone = model->GetBone(_ui.BoneControllerBone->currentIndex());

	if (newBone->bonecontroller[typeIndex] != -1)
	{
		_ui.BoneControllerBone->setCurrentIndex(boneController->bone);

		QMessageBox::critical(this, "Error",
			QString{"Bone \"%1\" already has a bone controller attached on type \"%2\""}.arg(newBone->name).arg(_ui.BoneControllerType->itemText(typeIndex)));
		return;
	}

	//Remove the reference to this controller from the old bone
	const auto oldBone = model->GetBone(boneController->bone);

	oldBone->bonecontroller[typeIndex] = -1;

	boneController->bone = _ui.BoneControllerBone->currentIndex();

	//Patch up the new bone reference
	newBone->bonecontroller[typeIndex] = _ui.BoneControllers->currentIndex();

	//TODO: pass undo command
	//_context->GetAsset()->EmitAssetDataChanged(AssetDataChangeEvent{AssetDataChangeType::Modification});
}

void StudioModelBodyPartsPanel::OnBoneControllerRangeChanged()
{
	const int boneControllerLogicalIndex = _ui.BoneControllers->currentIndex();

	auto entity = _asset->GetScene()->GetEntity();

	auto model = entity->GetModel()->GetStudioHeader();

	const auto boneController = model->GetBoneController(boneControllerLogicalIndex);

	boneController->start = _ui.BoneControllerStart->value();
	boneController->end = _ui.BoneControllerEnd->value();

	UpdateControllerRange();

	//Reset the value back to 0
	OnBoneControllerValueSpinnerChanged(0);

	//TODO: mark model changed
}

void StudioModelBodyPartsPanel::OnBoneControllerRestChanged()
{
	const int boneControllerLogicalIndex = _ui.BoneControllers->currentIndex();

	auto entity = _asset->GetScene()->GetEntity();

	auto model = entity->GetModel()->GetStudioHeader();

	const auto boneController = model->GetBoneController(boneControllerLogicalIndex);

	boneController->rest = _ui.BoneControllerRest->value();

	//TODO: mark model changed
}

void StudioModelBodyPartsPanel::OnBoneControllerIndexChanged()
{
	const int boneControllerLogicalIndex = _ui.BoneControllers->currentIndex();

	auto entity = _asset->GetScene()->GetEntity();

	auto model = entity->GetModel()->GetStudioHeader();

	const auto boneController = model->GetBoneController(boneControllerLogicalIndex);

	boneController->index = _ui.BoneControllerIndex->currentIndex();

	//TODO: mark model changed
}

void StudioModelBodyPartsPanel::OnBoneControllerTypeChanged(int index)
{
	const int boneControllerLogicalIndex = _ui.BoneControllers->currentIndex();

	auto entity = _asset->GetScene()->GetEntity();

	auto model = entity->GetModel()->GetStudioHeader();

	const auto boneController = model->GetBoneController(boneControllerLogicalIndex);

	const auto bone = model->GetBone(boneController->bone);

	const int oldType = boneController->type & STUDIO_BONECONTROLLER_TYPES;
	const int newType = 1 << index;

	const int oldTypeIndex = static_cast<int>(std::log2(oldType));
	const int newTypeIndex = static_cast<int>(std::log2(newType));

	if (bone->bonecontroller[newTypeIndex] != -1)
	{
		_ui.BoneControllerType->setCurrentIndex(oldTypeIndex);

		QMessageBox::critical(this, "Error",
			QString{"Bone \"%1\" already has a controller attached on type \"%2\""}.arg(bone->name).arg(_ui.BoneControllerType->itemText(newTypeIndex)));
		return;
	}

	bone->bonecontroller[oldTypeIndex] = -1;

	boneController->type &= ~oldType;

	boneController->type |= newType;

	bone->bonecontroller[newTypeIndex] = _ui.BoneControllers->currentIndex();

	//TODO: mark model changed
}
}
