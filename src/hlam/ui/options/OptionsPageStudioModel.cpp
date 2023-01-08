#include <algorithm>
#include <cassert>
#include <limits>

#include <QFileDialog>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

#include "settings/StudioModelSettings.hpp"

#include "ui/EditorContext.hpp"
#include "ui/options/OptionsPageStudioModel.hpp"

class NumericItemDelegate : public QStyledItemDelegate
{
public:
	explicit NumericItemDelegate(OptionsPageStudioModelWidget* optionsWidget, QObject* parent = nullptr)
		: QStyledItemDelegate(parent)
		, _optionsWidget(optionsWidget)
	{
	}

	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const
	{
		QSpinBox* editor = new QSpinBox(parent);
		editor->setFrame(false);
		editor->setMinimum(1);
		editor->setMaximum(std::numeric_limits<int>::max());

		return editor;
	}

	void setEditorData(QWidget* editor,const QModelIndex& index) const
	{
		int value = index.model()->data(index, Qt::EditRole).toInt();

		QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
		spinBox->setValue(value);
	}

	void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
	{
		QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
		spinBox->interpretText();
		int value = spinBox->value();

		model->setData(index, _optionsWidget->FindFirstFreeEventId(value), Qt::EditRole);
	}

	void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex&) const
	{
		editor->setGeometry(option.rect);
	}

private:
	OptionsPageStudioModelWidget* const _optionsWidget;
};

const QString OptionsPageStudioModelCategory{QStringLiteral("D.Assets")};
const QString OptionsPageStudioModelId{QStringLiteral("Studiomodel")};

OptionsPageStudioModel::OptionsPageStudioModel(const std::shared_ptr<StudioModelSettings>& studioModelSettings)
	: _studioModelSettings(studioModelSettings)
{
	assert(_studioModelSettings);

	SetCategory(QString{OptionsPageStudioModelCategory});
	SetCategoryTitle("Assets");
	SetId(QString{OptionsPageStudioModelId});
	SetPageTitle("Studiomodel");
	SetWidgetFactory([this](EditorContext* editorContext)
		{
			return new OptionsPageStudioModelWidget(editorContext, _studioModelSettings.get());
		});
}

OptionsPageStudioModel::~OptionsPageStudioModel() = default;

OptionsPageStudioModelWidget::OptionsPageStudioModelWidget(
	EditorContext* editorContext, StudioModelSettings* studioModelSettings)
	: _editorContext(editorContext)
	, _studioModelSettings(studioModelSettings)
{
	_ui.setupUi(this);

	_ui.AutodetectViewmodels->setChecked(_studioModelSettings->ShouldAutodetectViewmodels());
	_ui.ActivateTextureViewWhenTexturesPanelOpened->setChecked(
		_studioModelSettings->ShouldActivateTextureViewWhenTexturesPanelOpened());

	_ui.FloorLengthSlider->setRange(_studioModelSettings->MinimumFloorLength, _studioModelSettings->MaximumFloorLength);
	_ui.FloorLengthSpinner->setRange(_studioModelSettings->MinimumFloorLength, _studioModelSettings->MaximumFloorLength);

	_ui.FloorLengthSlider->setValue(_studioModelSettings->GetFloorLength());
	_ui.FloorLengthSpinner->setValue(_studioModelSettings->GetFloorLength());

	connect(_ui.FloorLengthSlider, &QSlider::valueChanged, _ui.FloorLengthSpinner, &QSpinBox::setValue);
	connect(_ui.FloorLengthSpinner, qOverload<int>(&QSpinBox::valueChanged), _ui.FloorLengthSlider, &QSlider::setValue);
	connect(_ui.ResetFloorLength, &QPushButton::clicked, this, &OptionsPageStudioModelWidget::OnResetFloorLength);

	auto model = new QStandardItemModel();

	const auto addItem = [model](int id)
	{
		auto item = new QStandardItem();
		item->setData(id, Qt::EditRole);
		model->appendRow(item);
	};

	model->setSortRole(Qt::EditRole);

	for (auto id : _studioModelSettings->GetSoundEventIds())
	{
		addItem(id);
	}

	_ui.SoundEventIds->setItemDelegate(new NumericItemDelegate(this, this));
	_ui.SoundEventIds->setModel(model);

	connect(_ui.AddSoundEventId, &QPushButton::clicked, this,
		[this, addItem]
		{
			addItem(FindFirstFreeEventId(1));
		});

	connect(_ui.RemoveSoundEventId, &QPushButton::clicked, this,
		[this, model]
		{
			auto selectedRows = _ui.SoundEventIds->selectionModel()->selectedRows();

			if (selectedRows.isEmpty())
			{
				return;
			}

			std::sort(selectedRows.begin(), selectedRows.end(),
				[](const auto& lhs, const auto& rhs)
				{
					return rhs < lhs;
				});

			for (const auto& selected : selectedRows)
			{
				model->removeRow(selected.row());
			}
		});

	connect(_ui.SoundEventIds->selectionModel(), &QItemSelectionModel::selectionChanged, this,
		[this](const QItemSelection& selected)
		{
			_ui.RemoveSoundEventId->setEnabled(!selected.isEmpty());
		});
	
	const auto sort = [model]
	{
		model->sort(0);
	};

	connect(model, &QStandardItemModel::dataChanged, this, sort);
	connect(model, &QStandardItemModel::rowsInserted, this, sort);

	sort();
}

OptionsPageStudioModelWidget::~OptionsPageStudioModelWidget() = default;

void OptionsPageStudioModelWidget::ApplyChanges()
{
	_studioModelSettings->SetAutodetectViewmodels(_ui.AutodetectViewmodels->isChecked());
	_studioModelSettings->SetActivateTextureViewWhenTexturesPanelOpened(
		_ui.ActivateTextureViewWhenTexturesPanelOpened->isChecked());
	_studioModelSettings->SetFloorLength(_ui.FloorLengthSlider->value());

	QSet<int> soundEventIds;

	auto model = _ui.SoundEventIds->model();

	for (int i = 0; i < model->rowCount(); ++i)
	{
		soundEventIds.insert(model->data(model->index(i, 0), Qt::EditRole).toInt());
	}

	_studioModelSettings->SetSoundEventIds(std::move(soundEventIds));
}

int OptionsPageStudioModelWidget::FindFirstFreeEventId(const int startId) const
{
	auto model = _ui.SoundEventIds->model();

	int id = startId;

	while (true)
	{
		int i;

		for (i = 0; i < model->rowCount(); ++i)
		{
			if (model->data(model->index(i, 0), Qt::EditRole).toInt() == id)
			{
				break;
			}
		}

		if (i >= model->rowCount())
		{
			break;
		}

		++id;

		// Failsafe. So we don't loop forever or get a negative value.
		if (startId == id)
		{
			break;
		}

		if (id == std::numeric_limits<int>::max())
		{
			if (startId == 1)
			{
				break;
			}

			id = 1;
		}
	}

	return id;
}

void OptionsPageStudioModelWidget::OnResetFloorLength()
{
	_ui.FloorLengthSlider->setValue(_studioModelSettings->DefaultFloorLength);
	_ui.FloorLengthSpinner->setValue(_studioModelSettings->DefaultFloorLength);
}
