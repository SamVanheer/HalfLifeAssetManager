#include <cmath>

#include <QSignalBlocker>
#include <QToolTip>

#include "entity/HLMVStudioModelEntity.hpp"

#include "ui/StateSnapshot.hpp"

#include "plugins/halflife/studiomodel/StudioModelAsset.hpp"
#include "plugins/halflife/studiomodel/ui/StudioModelData.hpp"
#include "plugins/halflife/studiomodel/ui/StudioModelUndoCommands.hpp"
#include "plugins/halflife/studiomodel/ui/StudioModelValidators.hpp"
#include "plugins/halflife/studiomodel/ui/dockpanels/BodyPartsPanel.hpp"
#include "plugins/halflife/studiomodel/ui/dockpanels/DockHelpers.hpp"

namespace studiomodel
{
BodyPartsPanel::BodyPartsPanel(StudioModelAssetProvider* provider)
	: _provider(provider)
{
	_ui.setupUi(this);

	const auto modelNameValidator = new UniqueModelNameValidator(MaxModelNameBytes - 1, _provider, this);

	_ui.ModelName->setValidator(modelNameValidator);

	connect(_ui.BodyParts, qOverload<int>(&QComboBox::currentIndexChanged),
		modelNameValidator, &UniqueModelNameValidator::SetCurrentBodyPartIndex);
	connect(_ui.Submodels, qOverload<int>(&QComboBox::currentIndexChanged),
		modelNameValidator, &UniqueModelNameValidator::SetCurrentIndex);

	connect(_provider, &StudioModelAssetProvider::AssetChanged, this, &BodyPartsPanel::OnAssetChanged);

	connect(_ui.BodyParts, qOverload<int>(&QComboBox::currentIndexChanged), this, &BodyPartsPanel::OnBodyPartChanged);
	connect(_ui.Submodels, qOverload<int>(&QComboBox::currentIndexChanged), this, &BodyPartsPanel::OnSubmodelChanged);
	connect(_ui.Skins, qOverload<int>(&QComboBox::currentIndexChanged), this, &BodyPartsPanel::OnSkinChanged);

	connect(_ui.ModelName, &QLineEdit::textChanged, this, &BodyPartsPanel::OnModelNameChanged);
	connect(_ui.ModelName, &QLineEdit::inputRejected, this, &BodyPartsPanel::OnModelNameRejected);

	OnAssetChanged(_provider->GetDummyAsset());
}

BodyPartsPanel::~BodyPartsPanel() = default;

void BodyPartsPanel::OnAssetChanged(StudioModelAsset* asset)
{
	if (_asset)
	{
		_asset->disconnect(this);
	}

	_asset = asset;

	auto modelData = _asset->GetModelData();

	connect(_asset, &StudioModelAsset::SaveSnapshot, this, &BodyPartsPanel::OnSaveSnapshot);
	connect(_asset, &StudioModelAsset::LoadSnapshot, this, &BodyPartsPanel::OnLoadSnapshot);

	_ui.BodyParts->setModel(modelData->BodyParts);
	_ui.BodyParts->setEnabled(_ui.BodyParts->count() > 0);

	_ui.BodypartsDataWidget->setVisible(_ui.BodyParts->isEnabled());

	// TODO: maybe store the list of submodels for each bodypart in user data.
	_ui.Submodels->setEnabled(_ui.BodyParts->isEnabled());

	const int skin = _asset->GetEntity()->GetSkin();
	{
		const QSignalBlocker skinBlocker{_ui.Skins};
		_ui.Skins->setModel(modelData->Skins);
	}
	_ui.Skins->setEnabled(_ui.Skins->count() > 0);

	//Should already be set but if there are no body parts and/or submodels it won't have been
	_ui.BodyValue->setText(QString::number(_asset->GetEntity()->GetBodygroup()));

	if (_previousModelData)
	{
		_previousModelData->DisconnectFromAll(this);
	}

	_previousModelData = modelData;

	connect(modelData, &StudioModelData::SubModelNameChanged, this, [this](int bodyPartIndex, int modelIndex)
		{
			if (bodyPartIndex == _ui.BodyParts->currentIndex() && modelIndex == _ui.Submodels->currentIndex())
			{
				const auto name{QString::fromStdString(
					_asset->GetEditableStudioModel()->Bodyparts[bodyPartIndex]->Models[modelIndex].Name)};

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
}

void BodyPartsPanel::OnLayoutDirectionChanged(QBoxLayout::Direction direction)
{
	DockableWidget::OnLayoutDirectionChanged(direction);
}

void BodyPartsPanel::OnBodyPartChanged(int index)
{
	const auto entity = _asset->GetEntity();
	const auto model = entity->GetEditableModel();

	const studiomdl::StudioBodypart emptyBodypart{};

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

	_asset->AddUndoCommand(new ChangeModelNameCommand(
		_asset, _ui.BodyParts->currentIndex(), _ui.Submodels->currentIndex(),
		subModel.Name.c_str(), _ui.ModelName->text()));
}

void BodyPartsPanel::OnModelNameRejected()
{
	QToolTip::showText(_ui.ModelName->mapToGlobal({0, -20}), "Model names must be unique");
}
}
