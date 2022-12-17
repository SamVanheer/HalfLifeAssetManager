#include <QAction>

#include "entity/HLMVStudioModelEntity.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelUndoCommands.hpp"
#include "ui/assets/studiomodel/dockpanels/TransformPanel.hpp"

namespace studiomodel
{
constexpr int RotateId = 0;
constexpr int ScaleId = 1;
constexpr int MoveId = 2;

TransformPanel::TransformPanel(StudioModelAsset* asset)
	: _asset(asset)
{
	_ui.setupUi(this);

	_ui.ModeGroup->setId(_ui.Rotate, RotateId);
	_ui.ModeGroup->setId(_ui.Scale, ScaleId);
	_ui.ModeGroup->setId(_ui.Move, MoveId);

	_ui.RotateValues->SetRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
	_ui.RotateValues->SetDecimals(6);

	_ui.MoveValues->SetRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
	_ui.MoveValues->SetDecimals(6);

	_ui.ScaleValue->setRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());

	connect(_ui.ModeGroup, &QButtonGroup::idClicked, _ui.ValuesStack, &QStackedWidget::setCurrentIndex);
	connect(_ui.ApplyButton, &QPushButton::clicked, this, &TransformPanel::OnApply);

	ResetValues();
}

void TransformPanel::ResetValues()
{
	_ui.RotateValues->SetValue({});
	_ui.ScaleValue->setValue(1);
	_ui.MoveValues->SetValue({});

	//Leave scale flags as-is
}

void TransformPanel::OnApply()
{
	switch (_ui.ModeGroup->checkedId())
	{
	case RotateId:
	{
		auto entity = _asset->GetEntity();

		auto data{studiomdl::CalculateRotatedData(*entity->GetEditableModel(), _ui.RotateValues->GetValue())};

		_asset->AddUndoCommand(new ChangeModelRotationCommand(_asset, std::move(data.first), std::move(data.second)));
		break;
	}

	case ScaleId:
	{
		const double scale = _ui.ScaleValue->value();

		int flags = studiomdl::ScaleFlags::None;

		if (_ui.ScaleMeshes->isChecked())
		{
			flags |= studiomdl::ScaleFlags::ScaleMeshes;
		}

		if (_ui.ScaleHitboxes->isChecked())
		{
			flags |= studiomdl::ScaleFlags::ScaleHitboxes;
		}

		if (_ui.ScaleSequenceBBoxes->isChecked())
		{
			flags |= studiomdl::ScaleFlags::ScaleSequenceBBoxes;
		}

		if (_ui.ScaleBones->isChecked())
		{
			flags |= studiomdl::ScaleFlags::ScaleBones;
		}

		if (_ui.ScaleEyePosition->isChecked())
		{
			flags |= studiomdl::ScaleFlags::ScaleEyePosition;
		}

		if (_ui.ScaleAttachments->isChecked())
		{
			flags |= studiomdl::ScaleFlags::ScaleAttachments;
		}

		auto entity = _asset->GetEntity();

		auto data{studiomdl::CalculateScaleData(*entity->GetEditableModel(), scale, flags)};

		_asset->AddUndoCommand(new ChangeModelScaleCommand(_asset, std::move(data.first), std::move(data.second)));

		break;
	}

	case MoveId:
	{
		auto moveData = studiomdl::CalculateMoveData(*_asset->GetEntity()->GetEditableModel(), _ui.MoveValues->GetValue());

		if (!moveData.first.BoneData.empty())
		{
			_asset->AddUndoCommand(new ChangeModelOriginCommand(_asset, std::move(moveData.first), std::move(moveData.second)));
		}
		break;
	}
	}

	if (_ui.ResetOnApply->isChecked())
	{
		ResetValues();
	}
}
}
