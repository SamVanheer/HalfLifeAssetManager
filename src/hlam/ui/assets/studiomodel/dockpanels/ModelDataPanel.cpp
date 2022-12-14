#include <limits>

#include <QSignalBlocker>

#include "entity/HLMVStudioModelEntity.hpp"

#include "ui/StateSnapshot.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelData.hpp"
#include "ui/assets/studiomodel/StudioModelUndoCommands.hpp"
#include "ui/assets/studiomodel/dockpanels/ModelDataPanel.hpp"

using namespace qt::widgets;

namespace ui::assets::studiomodel
{
ModelDataPanel::ModelDataPanel(StudioModelAsset* asset)
	: _asset(asset)
{
	_ui.setupUi(this);

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

	connect(_asset, &StudioModelAsset::AssetChanged, this, &ModelDataPanel::OnAssetChanged);

	connect(_ui.EyePosition, &Vector3Edit::ValueChanged, this, &ModelDataPanel::OnEyePositionChanged);

	connect(_ui.BBoxMin, &Vector3Edit::ValueChanged, this, &ModelDataPanel::OnBBoxMinChanged);
	connect(_ui.BBoxMax, &Vector3Edit::ValueChanged, this, &ModelDataPanel::OnBBoxMaxChanged);

	connect(_ui.CBoxMin, &Vector3Edit::ValueChanged, this, &ModelDataPanel::OnCBoxMinChanged);
	connect(_ui.CBoxMax, &Vector3Edit::ValueChanged, this, &ModelDataPanel::OnCBoxMaxChanged);

	OnAssetChanged(nullptr);
}

ModelDataPanel::~ModelDataPanel() = default;

void ModelDataPanel::OnLayoutDirectionChanged(QBoxLayout::Direction direction)
{
	DockableWidget::OnLayoutDirectionChanged(direction);

	_ui.BoundingBoxLayout->setDirection(direction);
	_ui.ClippingBoxLayout->setDirection(direction);
}

void ModelDataPanel::OnAssetChanged(StudioModelAsset* asset)
{
	auto modelData = asset ? asset->GetModelData() : StudioModelData::GetEmptyModel();

	const QSignalBlocker blocker{_ui.EyePosition};
	const QSignalBlocker bmin{_ui.BBoxMin};
	const QSignalBlocker bmax{_ui.BBoxMax};
	const QSignalBlocker cmin{_ui.CBoxMin};
	const QSignalBlocker cmax{_ui.CBoxMax};

	if (asset)
	{
		auto model = asset->GetEntity()->GetEditableModel();

		_ui.EyePosition->SetValue(model->EyePosition);
		_ui.BBoxMin->SetValue(model->BoundingMin);
		_ui.BBoxMax->SetValue(model->BoundingMax);
		_ui.CBoxMin->SetValue(model->ClippingMin);
		_ui.CBoxMax->SetValue(model->ClippingMax);
	}
	else
	{
		_ui.EyePosition->SetValue(glm::vec3{0});
		_ui.BBoxMin->SetValue(glm::vec3{0});
		_ui.BBoxMax->SetValue(glm::vec3{0});
		_ui.CBoxMin->SetValue(glm::vec3{0});
		_ui.CBoxMax->SetValue(glm::vec3{0});
	}

	if (_previousModelData)
	{
		_previousModelData->DisconnectFromAll(this);
	}

	_previousModelData = modelData;

	connect(modelData, &StudioModelData::EyePositionChanged, this, [this]()
		{
			const QSignalBlocker blocker{_ui.EyePosition};
			_ui.EyePosition->SetValue(_asset->GetEditableStudioModel()->EyePosition);
		});

	connect(modelData, &StudioModelData::ModelBBoxChanged, this, [this]()
		{
			auto model = _asset->GetEditableStudioModel();
			const QSignalBlocker min{_ui.BBoxMin};
			const QSignalBlocker max{_ui.BBoxMax};
			_ui.BBoxMin->SetValue(model->BoundingMin);
			_ui.BBoxMax->SetValue(model->BoundingMax);
		});

	connect(modelData, &StudioModelData::ModelCBoxChanged, this, [this]()
		{
			auto model = _asset->GetEditableStudioModel();
			const QSignalBlocker min{_ui.CBoxMin};
			const QSignalBlocker max{_ui.CBoxMax};
			_ui.CBoxMin->SetValue(model->ClippingMin);
			_ui.CBoxMax->SetValue(model->ClippingMax);
		});
}

void ModelDataPanel::OnEyePositionChanged(const glm::vec3& value)
{
	_asset->AddUndoCommand(new ChangeEyePositionCommand(_asset, _asset->GetEntity()->GetEditableModel()->EyePosition, value));
}

void ModelDataPanel::OnBBoxMinChanged(const glm::vec3& value)
{
	auto model = _asset->GetEntity()->GetEditableModel();
	_asset->AddUndoCommand(new ChangeBBoxCommand(_asset, {model->BoundingMin, model->BoundingMax}, {value, model->BoundingMax}));
}

void ModelDataPanel::OnBBoxMaxChanged(const glm::vec3& value)
{
	auto model = _asset->GetEntity()->GetEditableModel();
	_asset->AddUndoCommand(new ChangeBBoxCommand(_asset, {model->BoundingMin, model->BoundingMax}, {model->BoundingMin, value}));
}

void ModelDataPanel::OnCBoxMinChanged(const glm::vec3& value)
{
	auto model = _asset->GetEntity()->GetEditableModel();
	_asset->AddUndoCommand(new ChangeCBoxCommand(_asset, {model->ClippingMin, model->ClippingMax}, {value, model->ClippingMax}));
}

void ModelDataPanel::OnCBoxMaxChanged(const glm::vec3& value)
{
	auto model = _asset->GetEntity()->GetEditableModel();
	_asset->AddUndoCommand(new ChangeCBoxCommand(_asset, {model->ClippingMin, model->ClippingMax}, {model->ClippingMin, value}));
}
}
