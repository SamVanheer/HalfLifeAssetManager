#include <limits>

#include <QSignalBlocker>

#include "entity/HLMVStudioModelEntity.hpp"

#include "ui/StateSnapshot.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelUndoCommands.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelModelDataPanel.hpp"

using namespace qt::widgets;

namespace ui::assets::studiomodel
{
StudioModelModelDataPanel::StudioModelModelDataPanel(StudioModelAsset* asset, QWidget* parent)
	: QWidget(parent)
	, _asset(asset)
{
	_ui.setupUi(this);

	_ui.Origin->SetRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
	_ui.Origin->SetDecimals(6);

	_ui.ScaleMeshSpinner->setRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
	_ui.ScaleBonesSpinner->setRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());

	_ui.EyePosition->SetRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
	_ui.EyePosition->SetDecimals(6);

	_ui.BBoxMin->SetRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
	_ui.BBoxMin->SetDecimals(6);
	_ui.BBoxMin->SetPrefix("Min ");

	_ui.BBoxMax->SetRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
	_ui.BBoxMax->SetDecimals(6);
	_ui.BBoxMax->SetPrefix("Max ");

	_ui.CBoxMin->SetRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
	_ui.CBoxMin->SetDecimals(6);
	_ui.CBoxMin->SetPrefix("Min ");

	_ui.CBoxMax->SetRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
	_ui.CBoxMax->SetDecimals(6);
	_ui.CBoxMax->SetPrefix("Max ");

	connect(_asset, &StudioModelAsset::ModelChanged, this, &StudioModelModelDataPanel::OnModelChanged);
	connect(_asset, &StudioModelAsset::LoadSnapshot, this, &StudioModelModelDataPanel::OnLoadSnapshot);

	connect(_ui.Origin, &Vector3Edit::ValueChanged, this, &StudioModelModelDataPanel::OnOriginChanged);
	connect(_ui.SetOrigin, &QPushButton::clicked, this, &StudioModelModelDataPanel::OnOriginChanged);

	connect(_ui.ScaleMesh, &QPushButton::clicked, this, &StudioModelModelDataPanel::OnScaleMesh);
	connect(_ui.ScaleBones, &QPushButton::clicked, this, &StudioModelModelDataPanel::OnScaleBones);

	connect(_ui.EyePosition, &Vector3Edit::ValueChanged, this, &StudioModelModelDataPanel::OnEyePositionChanged);

	connect(_ui.BBoxMin, &Vector3Edit::ValueChanged, this, &StudioModelModelDataPanel::OnBBoxMinChanged);
	connect(_ui.BBoxMax, &Vector3Edit::ValueChanged, this, &StudioModelModelDataPanel::OnBBoxMaxChanged);

	connect(_ui.CBoxMin, &Vector3Edit::ValueChanged, this, &StudioModelModelDataPanel::OnCBoxMinChanged);
	connect(_ui.CBoxMax, &Vector3Edit::ValueChanged, this, &StudioModelModelDataPanel::OnCBoxMaxChanged);

	connect(_ui.FlipNormals, &QPushButton::clicked, this, &StudioModelModelDataPanel::OnFlipNormals);

	InitializeUI();

	_ui.PagesList->setCurrentRow(0);
}

StudioModelModelDataPanel::~StudioModelModelDataPanel() = default;

void StudioModelModelDataPanel::OnModelChanged(const ModelChangeEvent& event)
{
	switch (event.GetId())
	{
	case ModelChangeId::ChangeEyePosition:
	{
		const auto& change = static_cast<const ModelEyePositionChangeEvent&>(event);

		const QSignalBlocker blocker{_ui.EyePosition};

		_ui.EyePosition->SetValue(change.GetValue());
		break;
	}

	case ModelChangeId::ChangeBBox:
	{
		const auto& change = static_cast<const ModelBBoxChangeEvent&>(event);

		const QSignalBlocker min{_ui.BBoxMin};
		const QSignalBlocker max{_ui.BBoxMax};

		_ui.BBoxMin->SetValue(change.GetValue().first);
		_ui.BBoxMax->SetValue(change.GetValue().second);
		break;
	}

	case ModelChangeId::ChangeCBox:
	{
		const auto& change = static_cast<const ModelCBoxChangeEvent&>(event);

		const QSignalBlocker min{_ui.CBoxMin};
		const QSignalBlocker max{_ui.CBoxMax};

		_ui.CBoxMin->SetValue(change.GetValue().first);
		_ui.CBoxMax->SetValue(change.GetValue().second);
		break;
	}

	case ModelChangeId::ChangeModelOrigin:
	{
		const auto& originChange = static_cast<const ModelOriginChangeEvent&>(event);

		_oldOffset = originChange.GetValue();

		const QSignalBlocker origin{_ui.Origin};

		_ui.Origin->SetValue(_oldOffset);
		break;
	}

	//The mesh and bone scales aren't reset here because they're not representative of the absolute model scale compared to the initial state
	}
}

void StudioModelModelDataPanel::OnLoadSnapshot(StateSnapshot* snapshot)
{
	InitializeUI();
}

void StudioModelModelDataPanel::InitializeUI()
{
	auto model = _asset->GetScene()->GetEntity()->GetEditableModel();

	{
		const QSignalBlocker blocker{_ui.EyePosition};
		_ui.EyePosition->SetValue(model->EyePosition);
	}

	{
		const QSignalBlocker min{_ui.BBoxMin};
		const QSignalBlocker max{_ui.BBoxMax};

		_ui.BBoxMin->SetValue(model->BoundingMin);
		_ui.BBoxMax->SetValue(model->BoundingMax);
	}

	{
		const QSignalBlocker min{_ui.CBoxMin};
		const QSignalBlocker max{_ui.CBoxMax};

		_ui.CBoxMin->SetValue(model->ClippingMin);
		_ui.CBoxMax->SetValue(model->ClippingMax);
	}
}

void StudioModelModelDataPanel::OnOriginChanged()
{
	const auto model = _asset->GetScene()->GetEntity()->GetEditableModel();

	const glm::vec3 absoluteOffset{_ui.Origin->GetValue()};
	const auto relativeOffset{absoluteOffset - _oldOffset};

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
					rootBone.Axes[0].Value + relativeOffset[0],
					rootBone.Axes[1].Value + relativeOffset[1],
					rootBone.Axes[2].Value + relativeOffset[2]
				}
			});
	}

	if (!newRootBonePositions.empty())
	{
		_asset->AddUndoCommand(
			new ChangeModelOriginCommand(_asset, {std::move(oldRootBonePositions), _oldOffset}, {std::move(newRootBonePositions), absoluteOffset}));
	}
}

void StudioModelModelDataPanel::OnScaleMesh()
{
	auto entity = _asset->GetScene()->GetEntity();

	auto data{studiomdl::CalculateScaledMeshesData(*entity->GetEditableModel(), _ui.ScaleMeshSpinner->value())};

	_asset->AddUndoCommand(new ChangeModelMeshesScaleCommand(_asset, std::move(data.first), std::move(data.second)));
}

void StudioModelModelDataPanel::OnScaleBones()
{
	auto entity = _asset->GetScene()->GetEntity();

	auto data{studiomdl::CalculateScaledBonesData(*entity->GetEditableModel(), _ui.ScaleBonesSpinner->value())};

	_asset->AddUndoCommand(new ChangeModelBonesScaleCommand(_asset, std::move(data.first), std::move(data.second)));
}

void StudioModelModelDataPanel::OnEyePositionChanged(const glm::vec3& value)
{
	_asset->AddUndoCommand(new ChangeEyePositionCommand(_asset, _asset->GetScene()->GetEntity()->GetEditableModel()->EyePosition, value));
}

void StudioModelModelDataPanel::OnBBoxMinChanged(const glm::vec3& value)
{
	auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	_asset->AddUndoCommand(new ChangeBBoxCommand(_asset, {model->BoundingMin, model->BoundingMax}, {value, model->BoundingMax}));
}

void StudioModelModelDataPanel::OnBBoxMaxChanged(const glm::vec3& value)
{
	auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	_asset->AddUndoCommand(new ChangeBBoxCommand(_asset, {model->BoundingMin, model->BoundingMax}, {model->BoundingMin, value}));
}

void StudioModelModelDataPanel::OnCBoxMinChanged(const glm::vec3& value)
{
	auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	_asset->AddUndoCommand(new ChangeCBoxCommand(_asset, {model->ClippingMin, model->ClippingMax}, {value, model->ClippingMax}));
}

void StudioModelModelDataPanel::OnCBoxMaxChanged(const glm::vec3& value)
{
	auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	_asset->AddUndoCommand(new ChangeCBoxCommand(_asset, {model->ClippingMin, model->ClippingMax}, {model->ClippingMin, value}));
}

void StudioModelModelDataPanel::OnFlipNormals()
{
	std::vector<glm::vec3> oldNormals;
	std::vector<glm::vec3> newNormals;

	auto model = _asset->GetScene()->GetEntity()->GetEditableModel();

	for (auto& bodypart : model->Bodyparts)
	{
		for (auto& model : bodypart->Models)
		{
			oldNormals.reserve(oldNormals.size() + model.Normals.size());
			newNormals.reserve(newNormals.size() + model.Normals.size());

			for (auto& normal : model.Normals)
			{
				oldNormals.push_back(normal.Vertex);
				newNormals.push_back(-normal.Vertex);
			}
		}
	}

	_asset->AddUndoCommand(new FlipNormalsCommand(_asset, std::move(oldNormals), std::move(newNormals)));
}
}
