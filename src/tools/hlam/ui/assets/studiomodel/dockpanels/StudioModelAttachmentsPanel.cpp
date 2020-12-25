#include <limits>

#include <QSignalBlocker>

#include "entity/HLMVStudioModelEntity.hpp"

#include "qt/ByteLengthValidator.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelUndoCommands.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelAttachmentsPanel.hpp"

namespace ui::assets::studiomodel
{
StudioModelAttachmentsPanel::StudioModelAttachmentsPanel(StudioModelAsset* asset, QWidget* parent)
	: QWidget(parent)
	, _asset(asset)
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
		spinBox->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
	}

	connect(_asset, &StudioModelAsset::ModelChanged, this, &StudioModelAttachmentsPanel::OnModelChanged);

	connect(_ui.Attachments, qOverload<int>(&QComboBox::currentIndexChanged), this, &StudioModelAttachmentsPanel::OnAttachmentChanged);
	connect(_ui.HighlightAttachment, &QCheckBox::stateChanged, this, &StudioModelAttachmentsPanel::OnHighlightAttachmentChanged);
	connect(_ui.Name, &QLineEdit::textChanged, this, &StudioModelAttachmentsPanel::OnNameChanged);
	connect(_ui.Type, qOverload<int>(&QSpinBox::valueChanged), this, &StudioModelAttachmentsPanel::OnTypeChanged);
	connect(_ui.Bone, qOverload<int>(&QComboBox::currentIndexChanged), this, &StudioModelAttachmentsPanel::OnBoneChanged);

	connect(_ui.OriginX, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelAttachmentsPanel::OnOriginChanged);
	connect(_ui.OriginY, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelAttachmentsPanel::OnOriginChanged);
	connect(_ui.OriginZ, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelAttachmentsPanel::OnOriginChanged);

	_ui.Name->setValidator(new qt::ByteLengthValidator(MaxAttachmentNameBytes - 1, this));

	const auto model = _asset->GetScene()->GetEntity()->GetModel();
	const auto header = model->GetStudioHeader();

	this->setEnabled(header->numattachments > 0);

	{
		const QSignalBlocker blocker{_ui.Bone};

		QStringList bones;

		bones.reserve(header->numbones);

		for (int i = 0; i < header->numbones; ++i)
		{
			bones.append(QString{"%1 (%2)"}.arg(header->GetBone(i)->name).arg(i));
		}

		_ui.Bone->addItems(bones);

		//Start off with nothing selected
		_ui.Bone->setCurrentIndex(-1);
	}

	if (header->numattachments > 0)
	{
		QStringList attachments;

		attachments.reserve(header->numattachments);

		for (int i = 0; i < header->numattachments; ++i)
		{
			attachments.append(QString{"Attachment %1"}.arg(i + 1));
		}

		_ui.Attachments->addItems(attachments);
	}
}

StudioModelAttachmentsPanel::~StudioModelAttachmentsPanel() = default;

void StudioModelAttachmentsPanel::OnModelChanged(const ModelChangeEvent& event)
{
	const auto model = _asset->GetScene()->GetEntity()->GetModel();
	const auto header = model->GetStudioHeader();

	switch (event.GetId())
	{
	case ModelChangeId::RenameBone:
	{
		const QSignalBlocker boneBlocker{_ui.Bone};

		const auto& listChange{static_cast<const ModelListChangeEvent&>(event)};

		const auto bone = header->GetBone(listChange.GetSourceIndex());

		_ui.Bone->setItemText(listChange.GetSourceIndex(), QString{"%1 (%2)"}.arg(bone->name).arg(listChange.GetSourceIndex()));
		UpdateQCString();
		break;
	}

	case ModelChangeId::ChangeAttachmentName:
	{
		const auto& listChange{static_cast<const ModelListChangeEvent&>(event)};

		if (listChange.GetSourceIndex() == _ui.Attachments->currentIndex())
		{
			const auto attachment = header->GetAttachment(listChange.GetSourceIndex());

			const QString text{attachment->name};

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
			const auto attachment = header->GetAttachment(listChange.GetSourceIndex());

			const QSignalBlocker type{_ui.Type};

			_ui.Type->setValue(attachment->type);
			UpdateQCString();
		}
		break;
	}

	case ModelChangeId::ChangeAttachmentBone:
	{
		const auto& listChange{static_cast<const ModelListChangeEvent&>(event)};

		if (listChange.GetSourceIndex() == _ui.Attachments->currentIndex())
		{
			const auto attachment = header->GetAttachment(listChange.GetSourceIndex());

			const QSignalBlocker bone{_ui.Bone};

			_ui.Bone->setCurrentIndex(attachment->bone);
			UpdateQCString();
		}
		break;
	}

	case ModelChangeId::ChangeAttachmentOrigin:
	{
		const auto& listChange{static_cast<const ModelListChangeEvent&>(event)};

		if (listChange.GetSourceIndex() == _ui.Attachments->currentIndex())
		{
			const auto attachment = header->GetAttachment(listChange.GetSourceIndex());

			const QSignalBlocker originX{_ui.OriginX};
			const QSignalBlocker originY{_ui.OriginY};
			const QSignalBlocker originZ{_ui.OriginZ};

			_ui.OriginX->setValue(attachment->org[0]);
			_ui.OriginY->setValue(attachment->org[1]);
			_ui.OriginZ->setValue(attachment->org[2]);
			UpdateQCString();
		}
		break;
	}
	}
}

void StudioModelAttachmentsPanel::UpdateQCString()
{
	const auto model = _asset->GetScene()->GetEntity()->GetModel();
	const auto header = model->GetStudioHeader();
	const auto attachment = header->GetAttachment(_ui.Attachments->currentIndex());
	const auto bone = header->GetBone(attachment->bone);

	_ui.QCString->setText(QString{"$attachment %1 \"%2\" %3 %4 %5"}
		.arg(_ui.Attachments->currentIndex())
		.arg(bone->name)
		.arg(attachment->org[0], 0, 'f', 6)
		.arg(attachment->org[1], 0, 'f', 6)
		.arg(attachment->org[2], 0, 'f', 6));
}

void StudioModelAttachmentsPanel::OnDockPanelChanged(QWidget* current, QWidget* previous)
{
	_isActive = current == this;

	OnHighlightAttachmentChanged();
}

void StudioModelAttachmentsPanel::OnAttachmentChanged(int index)
{
	const auto model = _asset->GetScene()->GetEntity()->GetModel();
	const auto header = model->GetStudioHeader();
	const auto attachment = header->GetAttachment(_ui.Attachments->currentIndex());

	{
		const QSignalBlocker name{_ui.Name};
		const QSignalBlocker type{_ui.Type};
		const QSignalBlocker bone{_ui.Bone};
		const QSignalBlocker originX{_ui.OriginX};
		const QSignalBlocker originY{_ui.OriginY};
		const QSignalBlocker originZ{_ui.OriginZ};

		_ui.Name->setText(attachment->name);
		_ui.Type->setValue(attachment->type);
		_ui.Bone->setCurrentIndex(attachment->bone);

		_ui.OriginX->setValue(attachment->org[0]);
		_ui.OriginY->setValue(attachment->org[1]);
		_ui.OriginZ->setValue(attachment->org[2]);
	}

	UpdateQCString();
}

void StudioModelAttachmentsPanel::OnHighlightAttachmentChanged()
{
	_asset->GetScene()->DrawSingleAttachmentIndex = (_isActive && _ui.HighlightAttachment->isChecked()) ? _ui.Attachments->currentIndex() : -1;
}

void StudioModelAttachmentsPanel::OnNameChanged()
{
	const auto model = _asset->GetScene()->GetEntity()->GetModel();
	const auto header = model->GetStudioHeader();
	const auto attachment = header->GetAttachment(_ui.Attachments->currentIndex());

	_asset->AddUndoCommand(new ChangeAttachmentNameCommand(_asset, _ui.Attachments->currentIndex(), attachment->name, _ui.Name->text()));
}

void StudioModelAttachmentsPanel::OnTypeChanged()
{
	const auto model = _asset->GetScene()->GetEntity()->GetModel();
	const auto header = model->GetStudioHeader();
	const auto attachment = header->GetAttachment(_ui.Attachments->currentIndex());

	_asset->AddUndoCommand(new ChangeAttachmentTypeCommand(_asset, _ui.Attachments->currentIndex(), attachment->type, _ui.Type->value()));
}

void StudioModelAttachmentsPanel::OnBoneChanged()
{
	const auto model = _asset->GetScene()->GetEntity()->GetModel();
	const auto header = model->GetStudioHeader();
	const auto attachment = header->GetAttachment(_ui.Attachments->currentIndex());

	_asset->AddUndoCommand(new ChangeAttachmentBoneCommand(_asset, _ui.Attachments->currentIndex(), attachment->bone, _ui.Bone->currentIndex()));
}

void StudioModelAttachmentsPanel::OnOriginChanged()
{
	const auto model = _asset->GetScene()->GetEntity()->GetModel();
	const auto header = model->GetStudioHeader();
	const auto attachment = header->GetAttachment(_ui.Attachments->currentIndex());

	_asset->AddUndoCommand(new ChangeAttachmentOriginCommand(_asset, _ui.Attachments->currentIndex(),
		{attachment->org[0], attachment->org[1], attachment->org[2]},
		{_ui.OriginX->value(), _ui.OriginY->value(), _ui.OriginZ->value()}));
}
}
