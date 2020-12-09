#include <QSignalBlocker>

#include "entity/CHLMVStudioModelEntity.h"

#include "ui/assets/studiomodel/StudioModelContext.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelBodyPartsPanel.hpp"

namespace ui::assets::studiomodel
{
StudioModelBodyPartsPanel::StudioModelBodyPartsPanel(StudioModelContext* context, QWidget* parent)
	: QWidget(parent)
	, _context(context)
{
	_ui.setupUi(this);

	connect(_ui.BodyParts, qOverload<int>(&QComboBox::currentIndexChanged), this, &StudioModelBodyPartsPanel::OnBodyPartChanged);
	connect(_ui.Submodels, qOverload<int>(&QComboBox::currentIndexChanged), this, &StudioModelBodyPartsPanel::OnSubmodelChanged);
	connect(_ui.Skins, qOverload<int>(&QComboBox::currentIndexChanged), this, &StudioModelBodyPartsPanel::OnSkinChanged);
	connect(_ui.BoneControllers, qOverload<int>(&QComboBox::currentIndexChanged), this, &StudioModelBodyPartsPanel::OnBoneControllerChanged);
	connect(_ui.BoneControllerValue, &QSlider::valueChanged, this, &StudioModelBodyPartsPanel::OnBoneControllerValueChanged);

	auto entity = _context->GetScene()->GetEntity();
	auto model = entity->GetModel()->GetStudioHeader();
	auto textureHeader = entity->GetModel()->GetTextureHeader();

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
		_ui.BoneControllerValue->setEnabled(false);
		_ui.BoneControllerValue->setRange(0, 2);
		_ui.BoneControllerValue->setValue(1);
	}

	_ui.BoneControllers->setEnabled(model->numbonecontrollers > 0);

	//Should already be set but if there are no body parts and/or submodels it won't have been
	_ui.BodyValue->setText(QString::number(entity->GetBodygroup()));
}

StudioModelBodyPartsPanel::~StudioModelBodyPartsPanel() = default;

void StudioModelBodyPartsPanel::OnBodyPartChanged(int index)
{
	auto entity = _context->GetScene()->GetEntity();

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
	auto entity = _context->GetScene()->GetEntity();

	entity->SetBodygroup(_ui.BodyParts->currentIndex(), index);

	_ui.BodyValue->setText(QString::number(entity->GetBodygroup()));
}

void StudioModelBodyPartsPanel::OnSkinChanged(int index)
{
	auto entity = _context->GetScene()->GetEntity();

	entity->SetSkin(index);
}

void StudioModelBodyPartsPanel::OnBoneControllerChanged(int index)
{
	auto entity = _context->GetScene()->GetEntity();

	auto model = entity->GetModel()->GetStudioHeader();

	const auto boneController = model->GetBoneController(index);

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
		const QSignalBlocker blocker{_ui.BoneControllerValue};

		_ui.BoneControllerValue->setRange((int)(start * _controllerSliderScale), (int)(end * _controllerSliderScale));
		_ui.BoneControllerValue->setValue(static_cast<int>(entity->GetControllerValue(index) * _controllerSliderScale));
	}
}

void StudioModelBodyPartsPanel::OnBoneControllerValueChanged(int value)
{
	const int boneControllerLogicalIndex = _ui.BoneControllers->currentIndex();

	if (boneControllerLogicalIndex != -1)
	{
		auto entity = _context->GetScene()->GetEntity();

		auto model = entity->GetModel()->GetStudioHeader();

		const auto boneController = model->GetBoneController(boneControllerLogicalIndex);

		const float adjustedValue = (static_cast<float>(value) / _controllerSliderScale);

		//TODO: support multiple mouth controllers somehow.
		if (boneController->index == STUDIO_MOUTH_CONTROLLER)
		{
			entity->SetMouth(adjustedValue);
		}
		else
		{
			entity->SetController(boneController->index, adjustedValue);
		}
	}
}
}
