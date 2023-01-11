#include <algorithm>
#include <cassert>
#include <limits>

#include <QFileDialog>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

#include "application/AssetManager.hpp"
#include "entity/Events.hpp"
#include "plugins/halflife/studiomodel/options/OptionsPageStudioModel.hpp"
#include "plugins/halflife/studiomodel/settings/StudioModelSettings.hpp"

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

		// Don't update the value unless it has changed to prevent auto-increment by selecting and de-selecting.
		if (model->data(index, Qt::EditRole).toInt() != value)
		{
			model->setData(index, _optionsWidget->FindFirstFreeEventId(value), Qt::EditRole);
		}
	}

	void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex&) const
	{
		editor->setGeometry(option.rect);
	}

private:
	OptionsPageStudioModelWidget* const _optionsWidget;
};

const QString OptionsPageStudioModelCategory{QStringLiteral("D.Assets")};
const QString OptionsPageStudioModelId{QStringLiteral("StudioModel")};

OptionsPageStudioModel::OptionsPageStudioModel(const std::shared_ptr<StudioModelSettings>& studioModelSettings)
	: _studioModelSettings(studioModelSettings)
{
	assert(_studioModelSettings);

	SetCategory(QString{OptionsPageStudioModelCategory});
	SetCategoryTitle("Assets");
	SetId(QString{OptionsPageStudioModelId});
	SetPageTitle("StudioModel");
	SetWidgetFactory([this](AssetManager* application)
		{
			return new OptionsPageStudioModelWidget(application, _studioModelSettings.get());
		});
}

OptionsPageStudioModel::~OptionsPageStudioModel() = default;

OptionsPageStudioModelWidget::OptionsPageStudioModelWidget(
	AssetManager* application, StudioModelSettings* studioModelSettings)
	: _application(application)
	, _studioModelSettings(studioModelSettings)
{
	_ui.setupUi(this);

	_ui.AutodetectViewmodels->setChecked(_studioModelSettings->ShouldAutodetectViewmodels());
	_ui.ActivateTextureViewWhenTexturesPanelOpened->setChecked(
		_studioModelSettings->ShouldActivateTextureViewWhenTexturesPanelOpened());

	_ui.GroundLengthSlider->setRange(_studioModelSettings->MinimumGroundLength, _studioModelSettings->MaximumGroundLength);
	_ui.GroundLengthSpinner->setRange(_studioModelSettings->MinimumGroundLength, _studioModelSettings->MaximumGroundLength);

	_ui.GroundLengthSlider->setValue(_studioModelSettings->GetGroundLength());
	_ui.GroundLengthSpinner->setValue(_studioModelSettings->GetGroundLength());

	connect(_ui.GroundLengthSlider, &QSlider::valueChanged, _ui.GroundLengthSpinner, &QSpinBox::setValue);
	connect(_ui.GroundLengthSpinner, qOverload<int>(&QSpinBox::valueChanged), _ui.GroundLengthSlider, &QSlider::setValue);
	connect(_ui.ResetGroundLength, &QPushButton::clicked, this, &OptionsPageStudioModelWidget::OnResetGroundLength);

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

			int lastRow = selectedRows.back().row() - 1;

			for (const auto& selected : selectedRows)
			{
				const int row = selected.row();
				model->removeRow(row);

				if (row <= lastRow)
				{
					--lastRow;
				}
			}

			// Select the row immediately preceding the last item, or the first item if there are no rows preceding it.
			lastRow = std::max(0, lastRow);
			_ui.SoundEventIds->selectionModel()->select(
				_ui.SoundEventIds->model()->index(lastRow, 0), QItemSelectionModel::ClearAndSelect);
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
	_studioModelSettings->SetGroundLength(_ui.GroundLengthSlider->value());

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
		// Exclude built-in events.
		if (!IsSoundEvent(id))
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

void OptionsPageStudioModelWidget::OnResetGroundLength()
{
	_ui.GroundLengthSlider->setValue(_studioModelSettings->DefaultGroundLength);
	_ui.GroundLengthSpinner->setValue(_studioModelSettings->DefaultGroundLength);
}
