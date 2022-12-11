#include <limits>

#include <QSignalBlocker>

#include "entity/HLMVStudioModelEntity.hpp"

#include "ui/StateSnapshot.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
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

	connect(_asset, &StudioModelAsset::ModelChanged, this, &ModelDataPanel::OnModelChanged);
	connect(_asset, &StudioModelAsset::LoadSnapshot, this, &ModelDataPanel::InitializeUI);

	connect(_ui.EyePosition, &Vector3Edit::ValueChanged, this, &ModelDataPanel::OnEyePositionChanged);

	connect(_ui.BBoxMin, &Vector3Edit::ValueChanged, this, &ModelDataPanel::OnBBoxMinChanged);
	connect(_ui.BBoxMax, &Vector3Edit::ValueChanged, this, &ModelDataPanel::OnBBoxMaxChanged);

	connect(_ui.CBoxMin, &Vector3Edit::ValueChanged, this, &ModelDataPanel::OnCBoxMinChanged);
	connect(_ui.CBoxMax, &Vector3Edit::ValueChanged, this, &ModelDataPanel::OnCBoxMaxChanged);

	InitializeUI();
}

ModelDataPanel::~ModelDataPanel() = default;

void ModelDataPanel::OnLayoutDirectionChanged(QBoxLayout::Direction direction)
{
	DockableWidget::OnLayoutDirectionChanged(direction);

	_ui.BoundingBoxLayout->setDirection(direction);
	_ui.ClippingBoxLayout->setDirection(direction);
}

void ModelDataPanel::OnModelChanged(const ModelChangeEvent& event)
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
	}
}

void ModelDataPanel::InitializeUI()
{
	auto model = _asset->GetEntity()->GetEditableModel();

	const QSignalBlocker blocker{_ui.EyePosition};
	const QSignalBlocker bmin{_ui.BBoxMin};
	const QSignalBlocker bmax{_ui.BBoxMax};
	const QSignalBlocker cmin{_ui.CBoxMin};
	const QSignalBlocker cmax{_ui.CBoxMax};

	_ui.EyePosition->SetValue(model->EyePosition);
	_ui.BBoxMin->SetValue(model->BoundingMin);
	_ui.BBoxMax->SetValue(model->BoundingMax);
	_ui.CBoxMin->SetValue(model->ClippingMin);
	_ui.CBoxMax->SetValue(model->ClippingMax);
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
