#include <limits>

#include "entity/CHLMVStudioModelEntity.h"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelUndoCommands.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelModelDataPanel.hpp"

namespace ui::assets::studiomodel
{
StudioModelModelDataPanel::StudioModelModelDataPanel(StudioModelAsset* asset, QWidget* parent)
	: QWidget(parent)
	, _asset(asset)
{
	_ui.setupUi(this);

	QDoubleSpinBox* const origin[] =
	{
		_ui.OriginX,
		_ui.OriginY,
		_ui.OriginZ
	};

	for (auto originSpinner : origin)
	{
		originSpinner->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
	}

	_ui.ScaleMeshSpinner->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
	_ui.ScaleBonesSpinner->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());

	connect(_ui.OriginX, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelModelDataPanel::OnOriginChanged);
	connect(_ui.OriginY, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelModelDataPanel::OnOriginChanged);
	connect(_ui.OriginZ, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelModelDataPanel::OnOriginChanged);

	connect(_ui.SetOrigin, &QPushButton::clicked, this, &StudioModelModelDataPanel::OnSetOrigin);

	connect(_ui.ScaleMesh, &QPushButton::clicked, this, &StudioModelModelDataPanel::OnScaleMesh);
	connect(_ui.ScaleBones, &QPushButton::clicked, this, &StudioModelModelDataPanel::OnScaleBones);
}

StudioModelModelDataPanel::~StudioModelModelDataPanel() = default;

void StudioModelModelDataPanel::UpdateOrigin()
{
	const auto model = _asset->GetScene()->GetEntity()->GetModel();
	const auto header = model->GetStudioHeader();

	const glm::vec3 offset{_ui.OriginX->value(), _ui.OriginY->value(), _ui.OriginZ->value()};

	const auto rootBoneIndices{model->GetRootBoneIndices()};

	std::vector<RootBoneData> oldRootBonePositions;
	std::vector<RootBoneData> newRootBonePositions;

	oldRootBonePositions.reserve(rootBoneIndices.size());
	newRootBonePositions.reserve(rootBoneIndices.size());

	for (auto rootBoneIndex : rootBoneIndices)
	{
		const auto rootBone = header->GetBone(rootBoneIndex);

		oldRootBonePositions.emplace_back(
			RootBoneData
			{
				rootBoneIndex,
				{rootBone->value[0], rootBone->value[1], rootBone->value[2]}
			}
		);

		newRootBonePositions.emplace_back(
			RootBoneData
			{
				rootBoneIndex,
				{
					rootBone->value[0] + offset[0],
					rootBone->value[1] + offset[1],
					rootBone->value[2] + offset[2]
				}
			});
	}

	if (!newRootBonePositions.empty())
	{
		_asset->AddUndoCommand(new ChangeModelOriginCommand(_asset, std::move(oldRootBonePositions), std::move(newRootBonePositions)));
	}
}

void StudioModelModelDataPanel::OnOriginChanged()
{
	UpdateOrigin();
}

void StudioModelModelDataPanel::OnSetOrigin()
{
	UpdateOrigin();
}

void StudioModelModelDataPanel::OnScaleMesh()
{
	auto entity = _asset->GetScene()->GetEntity();

	auto data{studiomdl::CalculateScaledMeshesData(*entity->GetModel(), _ui.ScaleMeshSpinner->value())};

	//TODO: reset scale value to 1

	_asset->AddUndoCommand(new ChangeModelMeshesScaleCommand(_asset, std::move(data.first), std::move(data.second)));
}

void StudioModelModelDataPanel::OnScaleBones()
{
	auto entity = _asset->GetScene()->GetEntity();

	auto data{studiomdl::CalculateScaledBonesData(*entity->GetModel(), _ui.ScaleBonesSpinner->value())};

	//TODO: reset scale value to 1

	_asset->AddUndoCommand(new ChangeModelBonesScaleCommand(_asset, std::move(data.first), std::move(data.second)));
}
}
