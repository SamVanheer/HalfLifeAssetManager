#include <limits>

#include <QSignalBlocker>
#include <QToolTip>

#include "entity/HLMVStudioModelEntity.hpp"

#include "ui/StateSnapshot.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelData.hpp"
#include "ui/assets/studiomodel/StudioModelUndoCommands.hpp"
#include "ui/assets/studiomodel/StudioModelValidators.hpp"
#include "ui/assets/studiomodel/dockpanels/AttachmentsPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/DockHelpers.hpp"

#include "ui/settings/StudioModelSettings.hpp"

namespace ui::assets::studiomodel
{
AttachmentsPanel::AttachmentsPanel(StudioModelAsset* asset)
	: _asset(asset)
{
	_ui.setupUi(this);

	_ui.Type->setRange(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());

	QDoubleSpinBox* const spinBoxes[] =
	{
		_ui.OriginX,
		_ui.OriginY,
		_ui.OriginZ
	};

	for (auto spinBox : spinBoxes)
	{
		spinBox->setRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
	}

	const auto attachmentNameValidator = new UniqueAttachmentNameValidator(MaxAttachmentNameBytes - 1, _asset, this);

	_ui.Name->setValidator(attachmentNameValidator);

	connect(_ui.Attachments, qOverload<int>(&QComboBox::currentIndexChanged), attachmentNameValidator, &UniqueAttachmentNameValidator::SetCurrentIndex);

	connect(_asset, &StudioModelAsset::ModelChanged, this, &AttachmentsPanel::OnModelChanged);
	connect(_asset, &StudioModelAsset::AssetChanged, this, &AttachmentsPanel::OnAssetChanged);
	connect(_asset, &StudioModelAsset::SaveSnapshot, this, &AttachmentsPanel::OnSaveSnapshot);
	connect(_asset, &StudioModelAsset::LoadSnapshot, this, &AttachmentsPanel::OnLoadSnapshot);

	connect(_ui.Attachments, qOverload<int>(&QComboBox::currentIndexChanged), this, &AttachmentsPanel::OnAttachmentChanged);
	connect(_ui.HighlightAttachment, &QCheckBox::stateChanged, this, &AttachmentsPanel::OnHighlightAttachmentChanged);

	connect(_ui.Name, &QLineEdit::textChanged, this, &AttachmentsPanel::OnNameChanged);
	connect(_ui.Name, &QLineEdit::inputRejected, this, &AttachmentsPanel::OnNameRejected);

	connect(_ui.Type, qOverload<int>(&QSpinBox::valueChanged), this, &AttachmentsPanel::OnTypeChanged);
	connect(_ui.Bone, qOverload<int>(&QComboBox::currentIndexChanged), this, &AttachmentsPanel::OnBoneChanged);

	connect(_ui.OriginX, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &AttachmentsPanel::OnOriginChanged);
	connect(_ui.OriginY, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &AttachmentsPanel::OnOriginChanged);
	connect(_ui.OriginZ, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &AttachmentsPanel::OnOriginChanged);

	OnAssetChanged(nullptr);
}

AttachmentsPanel::~AttachmentsPanel() = default;

void AttachmentsPanel::OnModelChanged(const ModelChangeEvent& event)
{
	const auto model = _asset->GetEntity()->GetEditableModel();

	switch (event.GetId())
	{
	case ModelChangeId::RenameBone:
	{
		const QSignalBlocker boneBlocker{_ui.Bone};

		const auto& listChange{static_cast<const ModelListChangeEvent&>(event)};

		const auto& bone = *model->Bones[listChange.GetSourceIndex()];

		_ui.Bone->setItemText(listChange.GetSourceIndex(), QString{"%1 (%2)"}.arg(bone.Name.c_str()).arg(listChange.GetSourceIndex()));
		UpdateQCString();
		break;
	}

	case ModelChangeId::ChangeAttachmentName:
	{
		const auto& listChange{static_cast<const ModelListChangeEvent&>(event)};

		if (listChange.GetSourceIndex() == _ui.Attachments->currentIndex())
		{
			const auto& attachment = *model->Attachments[listChange.GetSourceIndex()];

			const QString text{attachment.Name.c_str()};

			//Avoid resetting the edit position
			if (_ui.Name->text() != text)
			{
				const QSignalBlocker name{_ui.Name};
				_ui.Name->setText(text);
				UpdateQCString();
			}
		}
		break;
	}

	case ModelChangeId::ChangeAttachmentType:
	{
		const auto& listChange{static_cast<const ModelListChangeEvent&>(event)};

		if (listChange.GetSourceIndex() == _ui.Attachments->currentIndex())
		{
			const auto& attachment = *model->Attachments[listChange.GetSourceIndex()];

			const QSignalBlocker type{_ui.Type};

			_ui.Type->setValue(attachment.Type);
			UpdateQCString();
		}
		break;
	}

	case ModelChangeId::ChangeAttachmentBone:
	{
		const auto& listChange{static_cast<const ModelListChangeEvent&>(event)};

		if (listChange.GetSourceIndex() == _ui.Attachments->currentIndex())
		{
			const auto& attachment = *model->Attachments[listChange.GetSourceIndex()];

			const QSignalBlocker bone{_ui.Bone};

			_ui.Bone->setCurrentIndex(attachment.Bone->ArrayIndex);
			UpdateQCString();
		}
		break;
	}

	case ModelChangeId::ChangeAttachmentOrigin:
	{
		const auto& listChange{static_cast<const ModelListChangeEvent&>(event)};

		if (listChange.GetSourceIndex() == _ui.Attachments->currentIndex())
		{
			const auto& attachment = *model->Attachments[listChange.GetSourceIndex()];

			const QSignalBlocker originX{_ui.OriginX};
			const QSignalBlocker originY{_ui.OriginY};
			const QSignalBlocker originZ{_ui.OriginZ};

			_ui.OriginX->setValue(attachment.Origin[0]);
			_ui.OriginY->setValue(attachment.Origin[1]);
			_ui.OriginZ->setValue(attachment.Origin[2]);
			UpdateQCString();
		}
		break;
	}
	}
}

void AttachmentsPanel::OnAssetChanged(StudioModelAsset* asset)
{
	auto modelData = asset ? asset->GetModelData() : StudioModelData::GetEmptyModel();

	{
		const QSignalBlocker blocker{_ui.Bone};
		_ui.Bone->setModel(modelData->Bones);

		//Start off with nothing selected
		_ui.Bone->setCurrentIndex(-1);
	}

	_ui.Attachments->setModel(modelData->Attachments);

	this->setEnabled(_ui.Attachments->count() > 0);
}

void AttachmentsPanel::OnSaveSnapshot(StateSnapshot* snapshot)
{
	snapshot->SetValue("attachments.attachment", _ui.Attachments->currentIndex());
}

void AttachmentsPanel::OnLoadSnapshot(StateSnapshot* snapshot)
{
	SetRestoredModelIndex(
		snapshot->Value("attachments.attachment").toInt(), _asset->GetEntity()->GetEditableModel()->Attachments.size(), *_ui.Attachments);
}

void AttachmentsPanel::UpdateQCString()
{
	const auto model = _asset->GetEntity()->GetEditableModel();

	const int index = _ui.Attachments->currentIndex();

	if (index != -1)
	{
		const auto& attachment = *model->Attachments[index];

		_ui.QCString->setText(QString{"$attachment %1 \"%2\" %3 %4 %5"}
			.arg(_ui.Attachments->currentIndex())
			.arg(attachment.Bone->Name.c_str())
			.arg(attachment.Origin[0], 0, 'f', 6)
			.arg(attachment.Origin[1], 0, 'f', 6)
			.arg(attachment.Origin[2], 0, 'f', 6));

		_ui.QCString->setCursorPosition(0);
	}
	else
	{
		_ui.QCString->setText({});
	}
}

void AttachmentsPanel::OnAttachmentChanged(int index)
{
	const auto model = _asset->GetEntity()->GetEditableModel();

	const studiomdl::Attachment emptyAttachment{};

	const auto& attachment = index != -1 ? *model->Attachments[index] : emptyAttachment;

	{
		const QSignalBlocker name{_ui.Name};
		const QSignalBlocker type{_ui.Type};
		const QSignalBlocker bone{_ui.Bone};
		const QSignalBlocker originX{_ui.OriginX};
		const QSignalBlocker originY{_ui.OriginY};
		const QSignalBlocker originZ{_ui.OriginZ};

		_ui.Name->setText(attachment.Name.c_str());
		_ui.Type->setValue(attachment.Type);
		_ui.Bone->setCurrentIndex(attachment.Bone ? attachment.Bone->ArrayIndex : -1);

		_ui.OriginX->setValue(attachment.Origin[0]);
		_ui.OriginY->setValue(attachment.Origin[1]);
		_ui.OriginZ->setValue(attachment.Origin[2]);
	}

	UpdateQCString();

	OnHighlightAttachmentChanged();
}

void AttachmentsPanel::OnHighlightAttachmentChanged()
{
	_asset->GetProvider()->GetStudioModelSettings()->DrawSingleAttachmentIndex = _ui.HighlightAttachment->isChecked() ? _ui.Attachments->currentIndex() : -1;
}

void AttachmentsPanel::OnNameChanged()
{
	const auto model = _asset->GetEntity()->GetEditableModel();
	const auto& attachment = *model->Attachments[_ui.Attachments->currentIndex()];

	_asset->AddUndoCommand(new ChangeAttachmentNameCommand(_asset, _ui.Attachments->currentIndex(), attachment.Name.c_str(), _ui.Name->text()));
}

void AttachmentsPanel::OnNameRejected()
{
	QToolTip::showText(_ui.Name->mapToGlobal({0, -20}), "Attachment names must be unique");
}

void AttachmentsPanel::OnTypeChanged()
{
	const auto model = _asset->GetEntity()->GetEditableModel();
	const auto& attachment = *model->Attachments[_ui.Attachments->currentIndex()];

	_asset->AddUndoCommand(new ChangeAttachmentTypeCommand(_asset, _ui.Attachments->currentIndex(), attachment.Type, _ui.Type->value()));
}

void AttachmentsPanel::OnBoneChanged()
{
	const auto model = _asset->GetEntity()->GetEditableModel();
	const auto& attachment = *model->Attachments[_ui.Attachments->currentIndex()];

	_asset->AddUndoCommand(new ChangeAttachmentBoneCommand(_asset, _ui.Attachments->currentIndex(), attachment.Bone->ArrayIndex, _ui.Bone->currentIndex()));
}

void AttachmentsPanel::OnOriginChanged()
{
	const auto model = _asset->GetEntity()->GetEditableModel();
	const auto& attachment = *model->Attachments[_ui.Attachments->currentIndex()];

	_asset->AddUndoCommand(new ChangeAttachmentOriginCommand(_asset, _ui.Attachments->currentIndex(),
		{attachment.Origin[0], attachment.Origin[1], attachment.Origin[2]},
		{_ui.OriginX->value(), _ui.OriginY->value(), _ui.OriginZ->value()}));
}
}
