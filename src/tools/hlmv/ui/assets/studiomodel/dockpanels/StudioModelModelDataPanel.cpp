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

	connect(_asset, &StudioModelAsset::ModelChanged, this, &StudioModelModelDataPanel::OnModelChanged);

	connect(_ui.OriginX, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelModelDataPanel::OnOriginChanged);
	connect(_ui.OriginY, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelModelDataPanel::OnOriginChanged);
	connect(_ui.OriginZ, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelModelDataPanel::OnOriginChanged);

	connect(_ui.SetOrigin, &QPushButton::clicked, this, &StudioModelModelDataPanel::OnSetOrigin);

	connect(_ui.ScaleMesh, &QPushButton::clicked, this, &StudioModelModelDataPanel::OnScaleMesh);
	connect(_ui.ScaleBones, &QPushButton::clicked, this, &StudioModelModelDataPanel::OnScaleBones);
}

StudioModelModelDataPanel::~StudioModelModelDataPanel() = default;

void StudioModelModelDataPanel::OnModelChanged(const ModelChangeEvent& event)
{
	switch (event.GetId())
	{
	case ModelChangeId::ChangeModelOrigin:
	{
		const auto& originChange = static_cast<const ModelOriginChangeEvent&>(event);

		_oldOffset = originChange.GetOffset();

		const QSignalBlocker originX{_ui.OriginX};
		const QSignalBlocker originY{_ui.OriginY};
		const QSignalBlocker originZ{_ui.OriginZ};

		_ui.OriginX->setValue(_oldOffset.x);
		_ui.OriginY->setValue(_oldOffset.y);
		_ui.OriginZ->setValue(_oldOffset.z);
		break;
	}

	//The mesh and bone scales aren't reset here because they're not representative of the absolute model scale compared to the initial state
	}
}

void StudioModelModelDataPanel::UpdateOrigin()
{
	const auto model = _asset->GetScene()->GetEntity()->GetModel();
	const auto header = model->GetStudioHeader();

	const glm::vec3 absoluteOffset{_ui.OriginX->value(), _ui.OriginY->value(), _ui.OriginZ->value()};
	const auto relativeOffset{absoluteOffset - _oldOffset};

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
					rootBone->value[0] + relativeOffset[0],
					rootBone->value[1] + relativeOffset[1],
					rootBone->value[2] + relativeOffset[2]
				}
			});
	}

	if (!newRootBonePositions.empty())
	{
		_asset->AddUndoCommand(
			new ChangeModelOriginCommand(_asset, {std::move(oldRootBonePositions), _oldOffset}, {std::move(newRootBonePositions), absoluteOffset}));
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

	_asset->AddUndoCommand(new ChangeModelMeshesScaleCommand(_asset, std::move(data.first), std::move(data.second)));
}

void StudioModelModelDataPanel::OnScaleBones()
{
	auto entity = _asset->GetScene()->GetEntity();

	auto data{studiomdl::CalculateScaledBonesData(*entity->GetModel(), _ui.ScaleBonesSpinner->value())};

	_asset->AddUndoCommand(new ChangeModelBonesScaleCommand(_asset, std::move(data.first), std::move(data.second)));
}
}
