#include <QLineEdit>
#include <QPushButton>
#include <QStyledItemDelegate>

#include "entity/CHLMVStudioModelEntity.h"

#include "ui/assets/studiomodel/StudioModelContext.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelEditEventsDialog.hpp"

namespace ui::assets::studiomodel
{
class OptionsLengthValidator final : public QValidator
{
public:
	OptionsLengthValidator(QObject* parent = nullptr)
		: QValidator(parent)
	{
	}

	State validate(QString& text, int&) const override
	{
		const auto utf8 = text.toUtf8();

		if (utf8.size() >= STUDIO_MAX_EVENT_OPTIONS_LENGTH)
		{
			return State::Intermediate;
		}

		return State::Acceptable;
	}
};

class ValidatingOptionsDelegate final : public QStyledItemDelegate
{
public:
	ValidatingOptionsDelegate(QObject* parent = nullptr)
		: QStyledItemDelegate(parent)
	{
	}

	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		auto editor = new QLineEdit(parent);
		editor->setValidator(new OptionsLengthValidator());
		return editor;
	}

	void setEditorData(QWidget* editor, const QModelIndex& index) const
	{
		QString value = index.model()->data(index, Qt::EditRole).toString();
		auto line = static_cast<QLineEdit*>(editor);
		line->setText(value);
	}

	void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
	{
		auto line = static_cast<QLineEdit*>(editor);
		QString value = line->text();
		model->setData(index, value);
	}

	void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		editor->setGeometry(option.rect);
	}
};

StudioModelEditEventsDialog::StudioModelEditEventsDialog(StudioModelContext* context, int sequenceIndex, QWidget* parent)
	: QDialog(parent)
	, _context(context)
	, _sequenceIndex(sequenceIndex)
{
	_ui.setupUi(this);

	auto applyButton = _ui.DialogButtons->button(QDialogButtonBox::StandardButton::Apply);

	applyButton->setEnabled(false);

	auto header = _context->GetScene()->GetEntity()->GetModel()->GetStudioHeader();

	auto sequence = header->GetSequence(_sequenceIndex);

	auto events = reinterpret_cast<const mstudioevent_t*>(header->GetData() + sequence->eventindex);

	_model = new QStandardItemModel(sequence->numevents, 4, this);

	_ui.EventsTable->setModel(_model);

	QStringList headerLabels;

	headerLabels.append("Frame Index");
	headerLabels.append("Event Id");
	headerLabels.append("Options");
	headerLabels.append("Type");

	_model->setHorizontalHeaderLabels(headerLabels);

	_ui.EventsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeMode::Stretch);

	_ui.EventsTable->setItemDelegateForColumn(2, new ValidatingOptionsDelegate());

	for (int row = 0; row < sequence->numevents; ++row)
	{
		auto event = events + row;

		_model->setData(_model->index(row, 0), event->frame);
		_model->setData(_model->index(row, 1), event->event);
		_model->setData(_model->index(row, 2), QString{event->options});
		_model->setData(_model->index(row, 3), event->type);
	}

	connect(applyButton, &QPushButton::clicked, this, &StudioModelEditEventsDialog::OnApply);
	connect(_model, &QStandardItemModel::dataChanged, this, &StudioModelEditEventsDialog::OnDataChanged);
}

StudioModelEditEventsDialog::~StudioModelEditEventsDialog() = default;

void StudioModelEditEventsDialog::accept()
{
	OnApply();

	QDialog::accept();
}

void StudioModelEditEventsDialog::OnApply()
{
	auto header = _context->GetScene()->GetEntity()->GetModel()->GetStudioHeader();

	auto sequence = header->GetSequence(_sequenceIndex);

	auto events = reinterpret_cast<mstudioevent_t*>(header->GetData() + sequence->eventindex);

	for (int row = 0; row < sequence->numevents; ++row)
	{
		auto event = events + row;

		event->frame = _model->data(_model->index(row, 0)).toInt();
		event->event = _model->data(_model->index(row, 1)).toInt();

		const auto options = _model->data(_model->index(row, 2)).toString();

		strncpy(event->options, options.toUtf8().constData(), sizeof(event->options) - 1);
		event->options[sizeof(event->options) - 1] = '\0';

		event->type = _model->data(_model->index(row, 3)).toInt();
	}

	auto applyButton = _ui.DialogButtons->button(QDialogButtonBox::StandardButton::Apply);

	applyButton->setEnabled(false);

	//TODO: track data change for saving
}

void StudioModelEditEventsDialog::OnDataChanged()
{
	auto applyButton = _ui.DialogButtons->button(QDialogButtonBox::StandardButton::Apply);

	applyButton->setEnabled(true);
}
}
