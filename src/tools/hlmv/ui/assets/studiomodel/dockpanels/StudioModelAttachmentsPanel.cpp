#include <limits>

#include <QSignalBlocker>

#include "entity/CHLMVStudioModelEntity.h"

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
	case ModelChangeId::BoneRename:
	{
		const QSignalBlocker boneBlocker{_ui.Bone};

		const auto& listChange{static_cast<const ModelListChangeEvent&>(event)};

		const auto bone = header->GetBone(listChange.GetSourceIndex());

		_ui.Bone->setItemText(listChange.GetSourceIndex(), QString{"%1 (%2)"}.arg(bone->name).arg(listChange.GetSourceIndex()));
		UpdateQCString();
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
		const QSignalBlocker originX{_ui.OriginX};
		const QSignalBlocker originY{_ui.OriginY};
		const QSignalBlocker originZ{_ui.OriginZ};

		_ui.Name->setText(attachment->name);
		_ui.Type->setValue(attachment->type);

		_ui.OriginX->setValue(attachment->org[0]);
		_ui.OriginX->setValue(attachment->org[1]);
		_ui.OriginX->setValue(attachment->org[2]);
	}

	_ui.Bone->setCurrentIndex(attachment->bone);

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

	strncpy(attachment->name, _ui.Name->text().toUtf8().constData(), sizeof(attachment->name) - 1);
	attachment->name[sizeof(attachment->name) - 1] = '\0';

	UpdateQCString();

	//TODO: mark model changed
}

void StudioModelAttachmentsPanel::OnTypeChanged()
{
	const auto model = _asset->GetScene()->GetEntity()->GetModel();

	const auto header = model->GetStudioHeader();

	const auto attachment = header->GetAttachment(_ui.Attachments->currentIndex());

	attachment->type = _ui.Type->value();

	UpdateQCString();
}

void StudioModelAttachmentsPanel::OnBoneChanged()
{
	const auto model = _asset->GetScene()->GetEntity()->GetModel();

	const auto header = model->GetStudioHeader();

	const auto attachment = header->GetAttachment(_ui.Attachments->currentIndex());

	attachment->bone = _ui.Bone->currentIndex();

	UpdateQCString();
}

void StudioModelAttachmentsPanel::OnOriginChanged()
{
	const auto model = _asset->GetScene()->GetEntity()->GetModel();

	const auto header = model->GetStudioHeader();

	const auto attachment = header->GetAttachment(_ui.Attachments->currentIndex());

	attachment->org[0] = _ui.OriginX->value();
	attachment->org[1] = _ui.OriginY->value();
	attachment->org[2] = _ui.OriginZ->value();

	UpdateQCString();
}
}
