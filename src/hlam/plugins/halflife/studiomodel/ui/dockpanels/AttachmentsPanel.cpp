#include <limits>

#include <QSignalBlocker>
#include <QToolTip>

#include "entity/HLMVStudioModelEntity.hpp"

#include "ui/StateSnapshot.hpp"

#include "plugins/halflife/studiomodel/StudioModelAsset.hpp"
#include "plugins/halflife/studiomodel/ui/DockHelpers.hpp"
#include "plugins/halflife/studiomodel/ui/StudioModelData.hpp"
#include "plugins/halflife/studiomodel/ui/StudioModelUndoCommands.hpp"
#include "plugins/halflife/studiomodel/ui/StudioModelValidators.hpp"
#include "plugins/halflife/studiomodel/ui/dockpanels/AttachmentsPanel.hpp"

namespace studiomodel
{
AttachmentsPanel::AttachmentsPanel(StudioModelAssetProvider* provider)
	: _provider(provider)
{
	_ui.setupUi(this);

	const auto attachmentNameValidator = new UniqueAttachmentNameValidator(MaxAttachmentNameBytes - 1, _provider, this);

	_ui.Name->setValidator(attachmentNameValidator);

	connect(_ui.Attachments, qOverload<int>(&QComboBox::currentIndexChanged),
		attachmentNameValidator, &UniqueAttachmentNameValidator::SetCurrentIndex);

	connect(_provider, &StudioModelAssetProvider::AssetChanged, this, &AttachmentsPanel::OnAssetChanged);

	connect(_ui.Attachments, qOverload<int>(&QComboBox::currentIndexChanged),
		this, &AttachmentsPanel::OnAttachmentChanged);
	connect(_ui.HighlightAttachment, &QCheckBox::stateChanged, this, &AttachmentsPanel::OnHighlightAttachmentChanged);

	connect(_ui.Name, &QLineEdit::textChanged, this, &AttachmentsPanel::OnPropsChanged);
	connect(_ui.Name, &QLineEdit::inputRejected, this, &AttachmentsPanel::OnNameRejected);

	connect(_ui.Bone, qOverload<int>(&QComboBox::currentIndexChanged), this, &AttachmentsPanel::OnPropsChanged);
	connect(_ui.Origin, &qt::widgets::ShortVector3Edit::ValueChanged, this, &AttachmentsPanel::OnPropsChanged);

	OnAssetChanged(_provider->GetDummyAsset());
}

AttachmentsPanel::~AttachmentsPanel() = default;

void AttachmentsPanel::OnLayoutDirectionChanged(QBoxLayout::Direction direction)
{
	_ui.DockLayout->setDirection(direction);
}

void AttachmentsPanel::OnAssetChanged(StudioModelAsset* asset)
{
	if (_asset)
	{
		_asset->disconnect(this);
	}

	_asset = asset;

	auto modelData = _asset->GetModelData();

	connect(_asset, &StudioModelAsset::SaveSnapshot, this, &AttachmentsPanel::OnSaveSnapshot);
	connect(_asset, &StudioModelAsset::LoadSnapshot, this, &AttachmentsPanel::OnLoadSnapshot);

	{
		const QSignalBlocker blocker{_ui.Bone};
		_ui.Bone->setModel(modelData->Bones);

		//Start off with nothing selected
		_ui.Bone->setCurrentIndex(-1);
	}

	_ui.Attachments->setModel(modelData->Attachments);

	this->setEnabled(_ui.Attachments->count() > 0);

	if (_previousModelData)
	{
		_previousModelData->DisconnectFromAll(this);
	}

	_previousModelData = modelData;

	connect(modelData->Attachments, &QAbstractItemModel::dataChanged, this, &AttachmentsPanel::UpdateQCString);

	connect(modelData, &StudioModelData::AttachmentDataChanged, this, [this](int index)
		{
			if (index == _ui.Attachments->currentIndex())
			{
				OnAttachmentChanged(index);
			}
		});
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
			.arg(QString::fromStdString(attachment.Bone->Name))
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
	// Don't refresh the UI if this is getting called in response to a change we made.
	if (_changingAttachmentProperties)
	{
		return;
	}

	const auto model = _asset->GetEntity()->GetEditableModel();

	const studiomdl::StudioAttachment emptyAttachment{};

	const auto& attachment = index != -1 ? *model->Attachments[index] : emptyAttachment;

	{
		const QSignalBlocker nameBlocker{_ui.Name};
		const QSignalBlocker boneBlocker{_ui.Bone};
		const QSignalBlocker originBlocker{_ui.Origin};

		const auto name = QString::fromStdString(attachment.Name);

		//Avoid resetting the edit position if possible
		if (_ui.Name->text() != name)
		{
			_ui.Name->setText(name);
		}

		_ui.Bone->setCurrentIndex(attachment.Bone ? attachment.Bone->ArrayIndex : -1);
		_ui.Origin->SetValue(attachment.Origin);
	}

	UpdateQCString();

	OnHighlightAttachmentChanged();
}

void AttachmentsPanel::OnHighlightAttachmentChanged()
{
	_asset->DrawSingleAttachmentIndex = _ui.HighlightAttachment->isChecked() ? _ui.Attachments->currentIndex() : -1;
}

void AttachmentsPanel::OnNameRejected()
{
	QToolTip::showText(_ui.Name->mapToGlobal({0, -20}), "Attachment names must be unique");
}

void AttachmentsPanel::OnPropsChanged()
{
	const auto model = _asset->GetEntity()->GetEditableModel();
	const auto& attachment = *model->Attachments[_ui.Attachments->currentIndex()];

	const AttachmentProps oldProps
	{
		.Name = attachment.Name,
		.Bone = attachment.Bone->ArrayIndex,
		.Origin = attachment.Origin
	};

	const AttachmentProps newProps
	{
		.Name = _ui.Name->text().toStdString(),
		.Bone = _ui.Bone->currentIndex(),
		.Origin = _ui.Origin->GetValue()
	};

	_changingAttachmentProperties = true;
	_asset->AddUndoCommand(new ChangeAttachmentPropsCommand(_asset, _ui.Attachments->currentIndex(), oldProps, newProps));
	_changingAttachmentProperties = false;
}
}
