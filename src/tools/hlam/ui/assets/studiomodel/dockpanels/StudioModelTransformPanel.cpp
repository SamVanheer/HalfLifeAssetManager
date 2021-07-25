#include <QAction>

#include "entity/HLMVStudioModelEntity.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelUndoCommands.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelTransformPanel.hpp"

namespace ui::assets::studiomodel
{
constexpr int MoveId = 0;
constexpr int ScaleId = 1;

StudioModelTransformPanel::StudioModelTransformPanel(StudioModelAsset* asset, QWidget* parent)
	: QWidget(parent)
	, _asset(asset)
{
	_ui.setupUi(this);

	_ui.ModeGroup->setId(_ui.Move, MoveId);
	_ui.ModeGroup->setId(_ui.Scale, ScaleId);

	_ui.MoveValues->SetRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
	_ui.MoveValues->SetDecimals(6);

	_ui.ScaleValue->setRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());

	connect(_ui.ModeGroup, &QButtonGroup::idClicked, _ui.ValuesStack, &QStackedWidget::setCurrentIndex);
	connect(_ui.ApplyButton, &QPushButton::clicked, this, &StudioModelTransformPanel::OnApply);

	ResetValues();
}

void StudioModelTransformPanel::ResetValues()
{
	_ui.MoveValues->SetValue({});
	_ui.ScaleValue->setValue(1);

	//Leave scale flags as-is
}

void StudioModelTransformPanel::OnApply()
{
	switch (_ui.ModeGroup->checkedId())
	{
	case MoveId:
	{
		const auto model = _asset->GetScene()->GetEntity()->GetEditableModel();

		const glm::vec3 offset{_ui.MoveValues->GetValue()};

		const auto rootBoneIndices{model->GetRootBoneIndices()};

		std::vector<RootBoneData> oldRootBonePositions;
		std::vector<RootBoneData> newRootBonePositions;

		oldRootBonePositions.reserve(rootBoneIndices.size());
		newRootBonePositions.reserve(rootBoneIndices.size());

		for (auto rootBoneIndex : rootBoneIndices)
		{
			const auto& rootBone = *model->Bones[rootBoneIndex];

			oldRootBonePositions.emplace_back(
				RootBoneData
				{
					rootBoneIndex,
					{rootBone.Axes[0].Value, rootBone.Axes[1].Value, rootBone.Axes[2].Value}
				}
			);

			newRootBonePositions.emplace_back(
				RootBoneData
				{
					rootBoneIndex,
					{
						rootBone.Axes[0].Value + offset[0],
						rootBone.Axes[1].Value + offset[1],
						rootBone.Axes[2].Value + offset[2]
					}
				});
		}

		if (!newRootBonePositions.empty())
		{
			_asset->AddUndoCommand(new ChangeModelOriginCommand(_asset, {std::move(oldRootBonePositions)}, {std::move(newRootBonePositions)}));
		}
		break;
	}

	case ScaleId:
	{
		const double scale = _ui.ScaleValue->value();

		int flags = studiomdl::ScaleFlags::None;

		if (_ui.ScaleMesh->isChecked())
		{
			flags |= studiomdl::ScaleFlags::ScaleMeshes;
		}

		if (_ui.ScaleBones->isChecked())
		{
			flags |= studiomdl::ScaleFlags::ScaleBones;
		}

		auto entity = _asset->GetScene()->GetEntity();

		auto data{studiomdl::CalculateScaleData(*entity->GetEditableModel(), scale, flags)};

		_asset->AddUndoCommand(new ChangeModelScaleCommand(_asset, std::move(data.first), std::move(data.second)));

		break;
	}
	}

	if (_ui.ResetOnApply->isChecked())
	{
		ResetValues();
	}
}
}
