#include <cmath>
#include <limits>

#include <QMessageBox>
#include <QSignalBlocker>
#include <QToolTip>

#include "entity/HLMVStudioModelEntity.hpp"

#include "ui/StateSnapshot.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelData.hpp"
#include "ui/assets/studiomodel/StudioModelUndoCommands.hpp"
#include "ui/assets/studiomodel/StudioModelValidators.hpp"
#include "ui/assets/studiomodel/dockpanels/BodyPartsPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/DockHelpers.hpp"

namespace studiomodel
{
//Parent indices are offset by one so -1 becomes 0, 0 becomes 1, etc
constexpr int BoneOffset = 1;

BodyPartsPanel::BodyPartsPanel(StudioModelAsset* asset)
	: _asset(asset)
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

	const auto modelNameValidator = new UniqueModelNameValidator(MaxModelNameBytes - 1, _asset, this);

	_ui.ModelName->setValidator(modelNameValidator);

	connect(_ui.BodyParts, qOverload<int>(&QComboBox::currentIndexChanged), modelNameValidator, &UniqueModelNameValidator::SetCurrentBodyPartIndex);
	connect(_ui.Submodels, qOverload<int>(&QComboBox::currentIndexChanged), modelNameValidator, &UniqueModelNameValidator::SetCurrentIndex);

	connect(_asset, &StudioModelAsset::AssetChanged, this, &BodyPartsPanel::OnAssetChanged);
	connect(_asset, &StudioModelAsset::SaveSnapshot, this, &BodyPartsPanel::OnSaveSnapshot);
	connect(_asset, &StudioModelAsset::LoadSnapshot, this, &BodyPartsPanel::OnLoadSnapshot);

	connect(_ui.BodyParts, qOverload<int>(&QComboBox::currentIndexChanged), this, &BodyPartsPanel::OnBodyPartChanged);
	connect(_ui.Submodels, qOverload<int>(&QComboBox::currentIndexChanged), this, &BodyPartsPanel::OnSubmodelChanged);
	connect(_ui.Skins, qOverload<int>(&QComboBox::currentIndexChanged), this, &BodyPartsPanel::OnSkinChanged);

	connect(_ui.ModelName, &QLineEdit::textChanged, this, &BodyPartsPanel::OnModelNameChanged);
	connect(_ui.ModelName, &QLineEdit::inputRejected, this, &BodyPartsPanel::OnModelNameRejected);

	connect(_ui.BoneControllers, qOverload<int>(&QComboBox::currentIndexChanged), this, &BodyPartsPanel::OnBoneControllerChanged);
	connect(_ui.BoneControllerValueSlider, &QSlider::valueChanged, this, &BodyPartsPanel::OnBoneControllerValueSliderChanged);
	connect(_ui.BoneControllerValueSpinner, qOverload<double>(&QDoubleSpinBox::valueChanged),
		this, &BodyPartsPanel::OnBoneControllerValueSpinnerChanged);

	connect(_ui.BoneControllerStart, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &BodyPartsPanel::OnBoneControllerRangeChanged);
	connect(_ui.BoneControllerEnd, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &BodyPartsPanel::OnBoneControllerRangeChanged);
	connect(_ui.BoneControllerRest, qOverload<int>(&QSpinBox::valueChanged), this, &BodyPartsPanel::OnBoneControllerRestChanged);
	connect(_ui.BoneControllerIndex, qOverload<int>(&QComboBox::currentIndexChanged), this, &BodyPartsPanel::OnBoneControllerIndexChanged);

	connect(_ui.BoneControllerBone, qOverload<int>(&QComboBox::currentIndexChanged), this, &BodyPartsPanel::OnBoneControllerBoneChanged);
	connect(_ui.BoneControllerBoneAxis, qOverload<int>(&QComboBox::currentIndexChanged), this, &BodyPartsPanel::OnBoneControllerAxisChanged);

	OnAssetChanged(_asset->GetProvider()->GetDummyAsset());
}

BodyPartsPanel::~BodyPartsPanel() = default;

void BodyPartsPanel::OnAssetChanged(StudioModelAsset* asset)
{
	const int skin = asset->GetEntity()->GetSkin();
	const int bodygroup = asset->GetEntity()->GetBodygroup();
	auto modelData = asset->GetModelData();

	{
		const QSignalBlocker blocker{_ui.BoneControllerBone};
		_ui.BoneControllerBone->setModel(modelData->BonesWithNone);
		//Start off with nothing selected
		_ui.BoneControllerBone->setCurrentIndex(-1);
	}

	_ui.BodyParts->setModel(modelData->BodyParts);
	_ui.BodyParts->setEnabled(_ui.BodyParts->count() > 0);

	_ui.BodypartsDataWidget->setVisible(_ui.BodyParts->isEnabled());

	// TODO: maybe store the list of submodels for each bodypart in user data.
	_ui.Submodels->setEnabled(_ui.BodyParts->isEnabled());

	_ui.Skins->setModel(modelData->Skins);
	_ui.Skins->setCurrentIndex(skin);
	_ui.Skins->setEnabled(_ui.Skins->count() > 0);

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

	//Should already be set but if there are no body parts and/or submodels it won't have been
	_ui.BodyValue->setText(QString::number(bodygroup));

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

	connect(modelData, &StudioModelData::SubModelNameChanged, this, [this](int bodyPartIndex, int modelIndex)
		{
			if (bodyPartIndex == _ui.BodyParts->currentIndex() && modelIndex == _ui.Submodels->currentIndex())
			{
				const auto name{QString::fromStdString(_asset->GetEditableStudioModel()->Bodyparts[bodyPartIndex]->Models[modelIndex].Name)};

				if (_ui.ModelName->text() != name)
				{
					const QSignalBlocker index{_ui.ModelName};
					_ui.ModelName->setText(name);
				}
			}
		});
}

void BodyPartsPanel::OnSaveSnapshot(StateSnapshot* snapshot)
{
	if (const int index = _ui.BodyParts->currentIndex(); index != -1)
	{
		const auto model = _asset->GetEntity()->GetEditableModel();

		snapshot->SetValue("bodyparts.bodypart", QString::fromStdString(model->Bodyparts[index]->Name));
	}

	//Just save bone controllers by list index, no real unique id for those (multiple controllers can have the same index)
	snapshot->SetValue("bodyparts.bonecontroller", _ui.BoneControllers->currentIndex());
}

void BodyPartsPanel::OnLoadSnapshot(StateSnapshot* snapshot)
{
	const auto model = _asset->GetEntity()->GetEditableModel();

	if (auto bodypart = snapshot->Value("bodyparts.bodypart"); bodypart.isValid())
	{
		const auto bodypartName = bodypart.toString().toStdString();

		if (auto it = std::find_if(model->Bodyparts.begin(), model->Bodyparts.end(), [&](const auto& bodypart)
			{
				return bodypart->Name == bodypartName;
			}); it != model->Bodyparts.end())
		{
			const auto index = it - model->Bodyparts.begin();

			_ui.BodyParts->setCurrentIndex(index);
		}
	}

	SetRestoredModelIndex(snapshot->Value("bodyparts.bonecontroller").toInt(), model->BoneControllers.size(), *_ui.BoneControllers);
}

void BodyPartsPanel::UpdateControllerRange(const studiomdl::BoneController& boneController)
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
		//Don't let the changes ripple back to change the current setting, because this will result in a loss of accuracy due to casting to integer
		const QSignalBlocker slider{_ui.BoneControllerValueSlider};
		const QSignalBlocker spinner{_ui.BoneControllerValueSpinner};

		const double value = entity->GetControllerValue(_ui.BoneControllers->currentIndex());

		_ui.BoneControllerValueSlider->setRange((int)(start * _controllerSliderScale), (int)(end * _controllerSliderScale));
		_ui.BoneControllerValueSlider->setValue(static_cast<int>(value * _controllerSliderScale));

		_ui.BoneControllerValueSpinner->setRange(start, end);
		_ui.BoneControllerValueSpinner->setValue(value);
	}
}

void BodyPartsPanel::OnLayoutDirectionChanged(QBoxLayout::Direction direction)
{
	DockableWidget::OnLayoutDirectionChanged(direction);

	_ui.ControllerDataLayout->setDirection(direction);
}

void BodyPartsPanel::OnBodyPartChanged(int index)
{
	const auto entity = _asset->GetEntity();
	const auto model = entity->GetEditableModel();

	const studiomdl::Bodypart emptyBodypart{};

	const auto& bodyPart = index != -1 ? *model->Bodyparts[index] : emptyBodypart;

	const bool hasSubmodels = !bodyPart.Models.empty();

	{
		const QSignalBlocker blocker{_ui.Submodels};

		_ui.Submodels->clear();

		if (hasSubmodels)
		{
			QStringList submodels;

			for (int i = 0; i < bodyPart.Models.size(); ++i)
			{
				submodels.append(QString{"Submodel %1"}.arg(i + 1));
			}

			_ui.Submodels->addItems(submodels);
		}

		_ui.Submodels->setCurrentIndex(entity->GetBodyValueForGroup(index));
	}

	_ui.Submodels->setEnabled(hasSubmodels);

	OnSubmodelChanged(_ui.Submodels->currentIndex());
}

void BodyPartsPanel::OnSubmodelChanged(int index)
{
	const int bodyPartIndex{_ui.BodyParts->currentIndex()};

	const auto entity = _asset->GetEntity();

	//Treat no submodels as having the first selected
	entity->SetBodygroup(bodyPartIndex, index != -1 ? index : 0);

	_ui.BodyValue->setText(QString::number(entity->GetBodygroup()));

	const auto model = entity->GetEditableModel();

	QString modelName;
	int meshCount{0};
	int vertexCount{0};
	int normalCount{0};

	bool success = false;

	if (bodyPartIndex >= 0 && bodyPartIndex < model->Bodyparts.size())
	{
		const auto& bodyPart = *model->Bodyparts[bodyPartIndex];

		if (index >= 0 && index < bodyPart.Models.size())
		{
			const auto subModel = model->GetModelByBodyPart(entity->GetBodygroup(), bodyPartIndex);

			modelName = subModel->Name.c_str();
			meshCount = subModel->Meshes.size();
			vertexCount = subModel->Vertices.size();
			normalCount = subModel->Normals.size();

			success = true;
		}
	}

	{
		const QSignalBlocker nameBlocker{_ui.ModelName};
		const QSignalBlocker meshCountBlocker{_ui.MeshCountLabel};
		const QSignalBlocker vertexCountBlocker{_ui.VertexCountLabel};
		const QSignalBlocker normalCountBlocker{_ui.NormalCountLabel};

		_ui.ModelName->setText(modelName);
		_ui.MeshCountLabel->setText(QString::number(meshCount));
		_ui.VertexCountLabel->setText(QString::number(vertexCount));
		_ui.NormalCountLabel->setText(QString::number(normalCount));
	}

	_ui.ModelName->setEnabled(success);
}

void BodyPartsPanel::OnSkinChanged(int index)
{
	const auto entity = _asset->GetEntity();

	entity->SetSkin(index);
}

void BodyPartsPanel::OnModelNameChanged()
{
	const auto model = _asset->GetEntity()->GetEditableModel();
	const auto& bodyPart = *model->Bodyparts[_ui.BodyParts->currentIndex()];
	const auto& subModel = bodyPart.Models[_ui.Submodels->currentIndex()];

	_asset->AddUndoCommand(new ChangeModelNameCommand(_asset, _ui.BodyParts->currentIndex(), _ui.Submodels->currentIndex(),
		subModel.Name.c_str(), _ui.ModelName->text()));
}

void BodyPartsPanel::OnModelNameRejected()
{
	QToolTip::showText(_ui.ModelName->mapToGlobal({0, -20}), "Model names must be unique");
}

void BodyPartsPanel::OnBoneControllerChanged(int index)
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

	const auto connection = model->FindBoneControllerIsAttachedTo(boneController.ArrayIndex).value_or(std::pair<int, int>(-1, -1));

	_ui.BoneControllerBone->setCurrentIndex(connection.first + BoneOffset);
	_ui.BoneControllerBoneAxis->setCurrentIndex(connection.second);
	_ui.BoneControllerStart->setValue(boneController.Start);
	_ui.BoneControllerEnd->setValue(boneController.End);
	_ui.BoneControllerRest->setValue(boneController.Rest);
	_ui.BoneControllerIndex->setCurrentIndex(boneController.Index);
}

void BodyPartsPanel::OnBoneControllerValueSliderChanged(int value)
{
	OnBoneControllerValueSpinnerChanged(value / _controllerSliderScale);
}

void BodyPartsPanel::OnBoneControllerValueSpinnerChanged(double value)
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

void BodyPartsPanel::OnBoneControllerRangeChanged()
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

void BodyPartsPanel::OnBoneControllerRestChanged()
{
	const int boneControllerLogicalIndex = _ui.BoneControllers->currentIndex();

	const auto model = _asset->GetEntity()->GetEditableModel();
	const auto& boneController = *model->BoneControllers[boneControllerLogicalIndex];

	_asset->AddUndoCommand(new ChangeBoneControllerRestCommand(_asset, boneControllerLogicalIndex,
		boneController.Rest, _ui.BoneControllerRest->value()));
}

void BodyPartsPanel::OnBoneControllerIndexChanged()
{
	const int boneControllerLogicalIndex = _ui.BoneControllers->currentIndex();

	const auto model = _asset->GetEntity()->GetEditableModel();
	const auto& boneController = *model->BoneControllers[boneControllerLogicalIndex];

	_asset->AddUndoCommand(new ChangeBoneControllerIndexCommand(_asset, boneControllerLogicalIndex,
		boneController.Index, _ui.BoneControllerIndex->currentIndex()));
}

void BodyPartsPanel::OnBoneControllerBoneChanged(int index)
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

void BodyPartsPanel::OnBoneControllerAxisChanged(int index)
{
	const int boneControllerLogicalIndex = _ui.BoneControllers->currentIndex();

	const auto model = _asset->GetEntity()->GetEditableModel();
	const auto& boneController = *model->BoneControllers[boneControllerLogicalIndex];

	//Will default to 0 if not currently attached to anything
	const int axis = static_cast<int>(std::log2(boneController.Type & STUDIO_BONECONTROLLER_TYPES));

	_asset->AddUndoCommand(new ChangeBoneControllerFromControllerCommand(_asset, boneControllerLogicalIndex,
		{_ui.BoneControllerBone->currentIndex() - BoneOffset, axis}, {_ui.BoneControllerBone->currentIndex() - BoneOffset, index}));
}
}
