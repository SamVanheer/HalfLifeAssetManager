#include <limits>

#include "entity/CHLMVStudioModelEntity.h"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
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

	auto model = _asset->GetScene()->GetEntity()->GetModel();

	for (auto rootBone : model->GetRootBones())
	{
		_rootBonePositions.emplace_back(
			RootBoneData
			{
				rootBone,
				{
					rootBone->value[0],
					rootBone->value[1],
					rootBone->value[2]
				}
			});
	}
}

StudioModelModelDataPanel::~StudioModelModelDataPanel() = default;

void StudioModelModelDataPanel::UpdateOrigin()
{
	auto entity = _asset->GetScene()->GetEntity();

	if (!_rootBonePositions.empty())
	{
		const glm::vec3 offset{_ui.OriginX->value(), _ui.OriginY->value(), _ui.OriginZ->value()};

		for (auto& data : _rootBonePositions)
		{
			const auto newPosition = data.OriginalRootBonePosition + offset;

			data.Bone->value[0] = newPosition.x;
			data.Bone->value[1] = newPosition.y;
			data.Bone->value[2] = newPosition.z;
		}

		//TODO: implement change tracking
		//m_pHLMV->GetState()->modelChanged = true;
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

	studiomdl::ScaleMeshes(entity->GetModel(), _ui.ScaleMeshSpinner->value());

	//TODO: implement change tracking
		//m_pHLMV->GetState()->modelChanged = true;
}

void StudioModelModelDataPanel::OnScaleBones()
{
	auto entity = _asset->GetScene()->GetEntity();

	studiomdl::ScaleBones(entity->GetModel(), _ui.ScaleBonesSpinner->value());

	//TODO: implement change tracking
	//m_pHLMV->GetState()->modelChanged = true;
}
}
