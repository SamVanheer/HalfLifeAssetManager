#include <limits>

#include <QSignalBlocker>

#include "entity/HLMVStudioModelEntity.hpp"

#include "ui/StateSnapshot.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelData.hpp"
#include "ui/assets/studiomodel/StudioModelUndoCommands.hpp"
#include "ui/assets/studiomodel/dockpanels/ModelDataPanel.hpp"

using namespace qt::widgets;

namespace studiomodel
{
ModelDataPanel::ModelDataPanel(StudioModelAssetProvider* provider)
	: _provider(provider)
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

	connect(_provider, &StudioModelAssetProvider::AssetChanged, this, &ModelDataPanel::OnAssetChanged);

	connect(_ui.EyePosition, &Vector3Edit::ValueChanged, this, &ModelDataPanel::OnEyePositionChanged);

	connect(_ui.BBoxMin, &Vector3Edit::ValueChanged, this, &ModelDataPanel::OnBBoxChanged);
	connect(_ui.BBoxMax, &Vector3Edit::ValueChanged, this, &ModelDataPanel::OnBBoxChanged);

	connect(_ui.CBoxMin, &Vector3Edit::ValueChanged, this, &ModelDataPanel::OnCBoxChanged);
	connect(_ui.CBoxMax, &Vector3Edit::ValueChanged, this, &ModelDataPanel::OnCBoxChanged);

	OnAssetChanged(_provider->GetDummyAsset());
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
	if (_asset)
	{
		_asset->disconnect(this);
	}

	_asset = asset;

	auto modelData = _asset->GetModelData();

	const QSignalBlocker blocker{_ui.EyePosition};
	const QSignalBlocker bmin{_ui.BBoxMin};
	const QSignalBlocker bmax{_ui.BBoxMax};
	const QSignalBlocker cmin{_ui.CBoxMin};
	const QSignalBlocker cmax{_ui.CBoxMax};

	auto model = asset->GetEntity()->GetEditableModel();

	_ui.EyePosition->SetValue(model->EyePosition);
	_ui.BBoxMin->SetValue(model->BoundingMin);
	_ui.BBoxMax->SetValue(model->BoundingMax);
	_ui.CBoxMin->SetValue(model->ClippingMin);
	_ui.CBoxMax->SetValue(model->ClippingMax);

	if (_previousModelData)
	{
		_previousModelData->DisconnectFromAll(this);
	}

	_previousModelData = modelData;

	connect(modelData, &StudioModelData::EyePositionChanged, this, [this]()
		{
			// Don't refresh the UI if this is getting called in response to a change we made.
			if (_changingDataProperties)
			{
				return;
			}

			const QSignalBlocker blocker{_ui.EyePosition};
			_ui.EyePosition->SetValue(_asset->GetEditableStudioModel()->EyePosition);
		});

	connect(modelData, &StudioModelData::ModelBBoxChanged, this, [this]()
		{
			// Don't refresh the UI if this is getting called in response to a change we made.
			if (_changingDataProperties)
			{
				return;
			}

			auto model = _asset->GetEditableStudioModel();
			const QSignalBlocker min{_ui.BBoxMin};
			const QSignalBlocker max{_ui.BBoxMax};
			_ui.BBoxMin->SetValue(model->BoundingMin);
			_ui.BBoxMax->SetValue(model->BoundingMax);
		});

	connect(modelData, &StudioModelData::ModelCBoxChanged, this, [this]()
		{
			// Don't refresh the UI if this is getting called in response to a change we made.
			if (_changingDataProperties)
			{
				return;
			}

			auto model = _asset->GetEditableStudioModel();
			const QSignalBlocker min{_ui.CBoxMin};
			const QSignalBlocker max{_ui.CBoxMax};
			_ui.CBoxMin->SetValue(model->ClippingMin);
			_ui.CBoxMax->SetValue(model->ClippingMax);
		});
}

void ModelDataPanel::OnEyePositionChanged()
{
	_changingDataProperties = true;
	_asset->AddUndoCommand(
		new ChangeEyePositionCommand(_asset, _asset->GetEntity()->GetEditableModel()->EyePosition, _ui.EyePosition->GetValue()));
	_changingDataProperties = false;
}

void ModelDataPanel::OnBBoxChanged()
{
	auto model = _asset->GetEntity()->GetEditableModel();
	_changingDataProperties = true;
	_asset->AddUndoCommand(
		new ChangeBBoxCommand(_asset, {model->BoundingMin, model->BoundingMax}, {_ui.BBoxMin->GetValue(), _ui.BBoxMax->GetValue()}));
	_changingDataProperties = false;
}

void ModelDataPanel::OnCBoxChanged()
{
	auto model = _asset->GetEntity()->GetEditableModel();
	_changingDataProperties = true;
	_asset->AddUndoCommand(
		new ChangeCBoxCommand(_asset, {model->ClippingMin, model->ClippingMax}, {_ui.CBoxMin->GetValue(), _ui.CBoxMax->GetValue()}));
	_changingDataProperties = false;
}
}
