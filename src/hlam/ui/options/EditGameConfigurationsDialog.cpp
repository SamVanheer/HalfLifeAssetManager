#include <QInputDialog>
#include <QStandardItemModel>

#include "ui/options/EditGameConfigurationsDialog.hpp"

const QString DefaultConfigurationName{"New Game"};

static QString GenerateUniqueName(const QString& baseName, const QStandardItemModel& model)
{
	QString name{baseName};

	for (int i = 1; i < std::numeric_limits<int>::max(); ++i)
	{
		if (model.findItems(name).isEmpty())
		{
			return name;
		}

		name = baseName + QString(" (%1)").arg(i);
	}

	//TODO: maybe use additional (%1) suffixes instead to retain unique names
	return baseName + " (Duplicate)";
}

EditGameConfigurationsDialog::EditGameConfigurationsDialog(
	QStandardItemModel* gameConfigurationsModel, int defaultConfigurationIndex, QWidget* parent)
	: QDialog(parent)
	, _gameConfigurationsModel(gameConfigurationsModel)
{
	_ui.setupUi(this);

	_ui.Configurations->setModel(gameConfigurationsModel);

	connect(_ui.Configurations->selectionModel(), &QItemSelectionModel::currentChanged, this,
		[this](const QModelIndex& index)
		{
			const bool hasSelections = index.isValid();

			_ui.Remove->setEnabled(hasSelections);
			_ui.Copy->setEnabled(hasSelections);
			_ui.Rename->setEnabled(hasSelections);
		});

	connect(_ui.Add, &QPushButton::clicked, this,
		[this]
		{
			const auto defaultName = GenerateUniqueName(DefaultConfigurationName, *_gameConfigurationsModel);

			bool ok = false;
			const auto name = QInputDialog::getText(this, "Add a game", "Enter the game's name:",
				QLineEdit::Normal, defaultName, &ok).trimmed();

			if (ok && !name.isEmpty())
			{
				emit ConfigurationAdded(name);
				_ui.Configurations->setCurrentIndex(
					_ui.Configurations->model()->index(_ui.Configurations->model()->rowCount() - 1, 0));
			}
		});

	connect(_ui.Remove, &QPushButton::clicked, this,
		[this]
		{
			const auto index = _ui.Configurations->currentIndex();

			if (index.isValid())
			{
				emit ConfigurationRemoved(index.row());
			}
		});

	connect(_ui.Copy, &QPushButton::clicked, this,
		[this]
		{
			const auto index = _ui.Configurations->currentIndex();

			if (index.isValid())
			{
				emit ConfigurationCopied(index.row());
				_ui.Configurations->setCurrentIndex(
					_ui.Configurations->model()->index(_ui.Configurations->model()->rowCount() - 1, 0));
			}
		});

	connect(_ui.Rename, &QPushButton::clicked, this,
		[this]
		{
			const auto index = _ui.Configurations->currentIndex();

			if (index.isValid())
			{
				const auto item = _gameConfigurationsModel->item(index.row());

				bool ok = false;
				const auto name = QInputDialog::getText(this, "Rename the game", "Enter the game's new name:",
					QLineEdit::Normal, item->text(), &ok).trimmed();

				if (ok && !name.isEmpty())
				{
					emit ConfigurationRenamed(index.row(), name);
				}
			}		
		});

	_ui.Configurations->setCurrentIndex(_ui.Configurations->model()->index(defaultConfigurationIndex, 0));
}

EditGameConfigurationsDialog::~EditGameConfigurationsDialog() = default;
