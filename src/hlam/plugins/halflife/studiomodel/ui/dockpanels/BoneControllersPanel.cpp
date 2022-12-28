#include <cmath>
#include <limits>

#include <QSignalBlocker>

#include "entity/HLMVStudioModelEntity.hpp"

#include "ui/StateSnapshot.hpp"

#include "plugins/halflife/studiomodel/StudioModelAsset.hpp"
#include "plugins/halflife/studiomodel/ui/StudioModelData.hpp"
#include "plugins/halflife/studiomodel/ui/StudioModelUndoCommands.hpp"
#include "plugins/halflife/studiomodel/ui/dockpanels/BoneControllersPanel.hpp"
#include "plugins/halflife/studiomodel/ui/dockpanels/DockHelpers.hpp"

namespace studiomodel
{
//Parent indices are offset by one so -1 becomes 0, 0 becomes 1, etc
constexpr int BoneOffset = 1;

BoneControllersPanel::BoneControllersPanel(StudioModelAssetProvider* provider)
	: _provider(provider)
{
	_ui.setupUi(this);

	_ui.BoneControllerRest->setRange(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());

	QDoubleSpinBox* const spinBoxes[] =
	{
		_ui.BoneControllerStart,
		_ui.BoneControllerEnd
	};

	for (auto spinBox : spinBoxes)
	{
		spinBox->setRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
	}

	connect(_provider, &StudioModelAssetProvider::AssetChanged, this, &BoneControllersPanel::OnAssetChanged);

	connect(_ui.BoneControllers, qOverload<int>(&QComboBox::currentIndexChanged),
		this, &BoneControllersPanel::OnBoneControllerChanged);
	connect(_ui.BoneControllerValueSlider, &QSlider::valueChanged,
		this, &BoneControllersPanel::OnBoneControllerValueSliderChanged);
	connect(_ui.BoneControllerValueSpinner, qOverload<double>(&QDoubleSpinBox::valueChanged),
		this, &BoneControllersPanel::OnBoneControllerValueSpinnerChanged);

	connect(_ui.BoneControllerStart, qOverload<double>(&QDoubleSpinBox::valueChanged),
		this, &BoneControllersPanel::OnBoneControllerRangeChanged);
	connect(_ui.BoneControllerEnd, qOverload<double>(&QDoubleSpinBox::valueChanged),
		this, &BoneControllersPanel::OnBoneControllerRangeChanged);
	connect(_ui.BoneControllerRest, qOverload<int>(&QSpinBox::valueChanged),
		this, &BoneControllersPanel::OnBoneControllerRestChanged);
	connect(_ui.BoneControllerIndex, qOverload<int>(&QComboBox::currentIndexChanged),
		this, &BoneControllersPanel::OnBoneControllerIndexChanged);

	connect(_ui.BoneControllerBone, qOverload<int>(&QComboBox::currentIndexChanged),
		this, &BoneControllersPanel::OnBoneControllerBoneChanged);
	connect(_ui.BoneControllerBoneAxis, qOverload<int>(&QComboBox::currentIndexChanged),
		this, &BoneControllersPanel::OnBoneControllerAxisChanged);

	OnAssetChanged(_provider->GetDummyAsset());
}

BoneControllersPanel::~BoneControllersPanel() = default;

void BoneControllersPanel::OnAssetChanged(StudioModelAsset* asset)
{
	if (_asset)
	{
		_asset->disconnect(this);
	}

	_asset = asset;

	auto modelData = _asset->GetModelData();

	connect(_asset, &StudioModelAsset::SaveSnapshot, this, &BoneControllersPanel::OnSaveSnapshot);
	connect(_asset, &StudioModelAsset::LoadSnapshot, this, &BoneControllersPanel::OnLoadSnapshot);

	{
		const QSignalBlocker blocker{_ui.BoneControllerBone};
		_ui.BoneControllerBone->setModel(modelData->BonesWithNone);
		//Start off with nothing selected
		_ui.BoneControllerBone->setCurrentIndex(-1);
	}

	_ui.BoneControllers->setModel(modelData->BoneControllers);
	_ui.BoneControllers->setEnabled(_ui.BoneControllers->count() > 0);

	_ui.ControllerDataWidget->setVisible(_ui.BoneControllers->isEnabled());

	_ui.BoneControllerValueSlider->setEnabled(_ui.BoneControllers->count() > 0);
	_ui.BoneControllerValueSpinner->setEnabled(_ui.BoneControllers->count() > 0);

	if (!_ui.BoneControllers->isEnabled())
	{
		//Disable and center it
		_ui.BoneControllerValueSlider->setRange(0, 2);
		_ui.BoneControllerValueSlider->setValue(1);

		_ui.BoneControllerValueSpinner->setRange(0, 1);
		_ui.BoneControllerValueSpinner->setValue(0);
	}

	if (_previousModelData)
	{
		_previousModelData->DisconnectFromAll(this);
	}

	_previousModelData = modelData;

	connect(modelData, &StudioModelData::BoneControllerDataChanged, this, [this](int index)
		{
			if (index == _ui.BoneControllers->currentIndex())
			{
				OnBoneControllerChanged(index);
			}
		});
}

void BoneControllersPanel::OnSaveSnapshot(StateSnapshot* snapshot)
{
	// Just save bone controllers by list index, no real unique id for those
	// (multiple controllers can have the same index)
	snapshot->SetValue("bonecontrollers.bonecontroller", _ui.BoneControllers->currentIndex());
}

void BoneControllersPanel::OnLoadSnapshot(StateSnapshot* snapshot)
{
	const auto model = _asset->GetEntity()->GetEditableModel();

	SetRestoredModelIndex(
		snapshot->Value("bonecontrollers.bonecontroller").toInt(), model->BoneControllers.size(), *_ui.BoneControllers);
}

void BoneControllersPanel::UpdateControllerRange(const studiomdl::BoneController& boneController)
{
	const auto entity = _asset->GetEntity();

	float start, end;

	//Swap values if the range is inverted
	if (boneController.End < boneController.Start)
	{
		start = boneController.End;
		end = boneController.Start;
	}
	else
	{
		start = boneController.Start;
		end = boneController.End;
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
		// Don't let the changes ripple back to change the current setting,
		// because this will result in a loss of accuracy due to casting to integer
		const QSignalBlocker slider{_ui.BoneControllerValueSlider};
		const QSignalBlocker spinner{_ui.BoneControllerValueSpinner};

		const double value = entity->GetControllerValue(_ui.BoneControllers->currentIndex());

		_ui.BoneControllerValueSlider->setRange(
			(int)(start * _controllerSliderScale), (int)(end * _controllerSliderScale));
		_ui.BoneControllerValueSlider->setValue(static_cast<int>(value * _controllerSliderScale));

		_ui.BoneControllerValueSpinner->setRange(start, end);
		_ui.BoneControllerValueSpinner->setValue(value);
	}
}

void BoneControllersPanel::OnLayoutDirectionChanged(QBoxLayout::Direction direction)
{
	DockableWidget::OnLayoutDirectionChanged(direction);

	_ui.ControllerDataLayout->setDirection(direction);
}

void BoneControllersPanel::OnBoneControllerChanged(int index)
{
	// Don't refresh the UI if this is getting called in response to a change we made.
	if (_changingBoneControllerProperties)
	{
		return;
	}

	const auto model = _asset->GetEntity()->GetEditableModel();

	static constexpr studiomdl::BoneController EmptyController{};

	const auto& boneController = index != -1 ? *model->BoneControllers[index] : EmptyController;

	UpdateControllerRange(boneController);

	const QSignalBlocker boneName{_ui.BoneControllerBone};
	const QSignalBlocker axis{_ui.BoneControllerBoneAxis};
	const QSignalBlocker start{_ui.BoneControllerStart};
	const QSignalBlocker end{_ui.BoneControllerEnd};
	const QSignalBlocker rest{_ui.BoneControllerRest};
	const QSignalBlocker controllerIndex{_ui.BoneControllerIndex};

	const auto connection = model->FindBoneControllerIsAttachedTo(boneController.ArrayIndex)
		.value_or(std::pair<int, int>(-1, -1));

	_ui.BoneControllerBone->setCurrentIndex(connection.first + BoneOffset);
	_ui.BoneControllerBoneAxis->setCurrentIndex(connection.second);
	_ui.BoneControllerStart->setValue(boneController.Start);
	_ui.BoneControllerEnd->setValue(boneController.End);
	_ui.BoneControllerRest->setValue(boneController.Rest);
	_ui.BoneControllerIndex->setCurrentIndex(boneController.Index);
}

void BoneControllersPanel::OnBoneControllerValueSliderChanged(int value)
{
	OnBoneControllerValueSpinnerChanged(value / _controllerSliderScale);
}

void BoneControllersPanel::OnBoneControllerValueSpinnerChanged(double value)
{
	const int boneControllerLogicalIndex = _ui.BoneControllers->currentIndex();

	if (boneControllerLogicalIndex != -1)
	{
		const auto entity = _asset->GetEntity();
		const auto model = entity->GetEditableModel();
		const auto& boneController = *model->BoneControllers[boneControllerLogicalIndex];

		//TODO: support multiple mouth controllers somehow.
		if (boneController.Index == STUDIO_MOUTH_CONTROLLER)
		{
			entity->SetMouth(value);
		}
		else
		{
			entity->SetController(boneController.Index, value);
		}
	}

	{
		const QSignalBlocker slider{_ui.BoneControllerValueSlider};
		const QSignalBlocker spinner{_ui.BoneControllerValueSpinner};

		_ui.BoneControllerValueSlider->setValue(static_cast<int>(value * _controllerSliderScale));
		_ui.BoneControllerValueSpinner->setValue(value);
	}
}

void BoneControllersPanel::OnBoneControllerRangeChanged()
{
	const int boneControllerLogicalIndex = _ui.BoneControllers->currentIndex();

	const auto model = _asset->GetEntity()->GetEditableModel();
	const auto& boneController = *model->BoneControllers[boneControllerLogicalIndex];

	_changingBoneControllerProperties = true;

	_asset->AddUndoCommand(new ChangeBoneControllerRangeCommand(_asset, boneControllerLogicalIndex,
		{boneController.Start, boneController.End},
		{static_cast<float>(_ui.BoneControllerStart->value()), static_cast<float>(_ui.BoneControllerEnd->value())}));

	_changingBoneControllerProperties = false;
}

void BoneControllersPanel::OnBoneControllerRestChanged()
{
	const int boneControllerLogicalIndex = _ui.BoneControllers->currentIndex();

	const auto model = _asset->GetEntity()->GetEditableModel();
	const auto& boneController = *model->BoneControllers[boneControllerLogicalIndex];

	_asset->AddUndoCommand(new ChangeBoneControllerRestCommand(_asset, boneControllerLogicalIndex,
		boneController.Rest, _ui.BoneControllerRest->value()));
}

void BoneControllersPanel::OnBoneControllerIndexChanged()
{
	const int boneControllerLogicalIndex = _ui.BoneControllers->currentIndex();

	const auto model = _asset->GetEntity()->GetEditableModel();
	const auto& boneController = *model->BoneControllers[boneControllerLogicalIndex];

	_asset->AddUndoCommand(new ChangeBoneControllerIndexCommand(_asset, boneControllerLogicalIndex,
		boneController.Index, _ui.BoneControllerIndex->currentIndex()));
}

void BoneControllersPanel::OnBoneControllerBoneChanged(int index)
{
	const int boneControllerLogicalIndex = _ui.BoneControllers->currentIndex();

	const auto model = _asset->GetEntity()->GetEditableModel();
	const auto& boneController = *model->BoneControllers[boneControllerLogicalIndex];

	const auto oldConnection = model->FindBoneControllerIsAttachedTo(boneControllerLogicalIndex);

	//Will default to 0 if not currently attached to anything
	const int axis = static_cast<int>(std::log2(boneController.Type & STUDIO_BONECONTROLLER_TYPES));

	_asset->AddUndoCommand(new ChangeBoneControllerFromControllerCommand(_asset, boneControllerLogicalIndex,
		oldConnection.value_or(std::pair<int, int>{-1, -1}), {index - BoneOffset, axis}));
}

void BoneControllersPanel::OnBoneControllerAxisChanged(int index)
{
	const int boneControllerLogicalIndex = _ui.BoneControllers->currentIndex();

	const auto model = _asset->GetEntity()->GetEditableModel();
	const auto& boneController = *model->BoneControllers[boneControllerLogicalIndex];

	//Will default to 0 if not currently attached to anything
	const int axis = static_cast<int>(std::log2(boneController.Type & STUDIO_BONECONTROLLER_TYPES));

	_asset->AddUndoCommand(new ChangeBoneControllerFromControllerCommand(_asset, boneControllerLogicalIndex,
		{_ui.BoneControllerBone->currentIndex() - BoneOffset, axis},
		{_ui.BoneControllerBone->currentIndex() - BoneOffset, index}));
}
}
