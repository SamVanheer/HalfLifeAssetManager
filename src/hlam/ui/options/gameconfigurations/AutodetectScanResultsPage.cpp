#include <algorithm>
#include <functional>

#include <QDir>
#include <QProgressDialog>
#include <QTimer>

#include "formats/liblist/LiblistReader.hpp"

#include "ui/options/gameconfigurations/AutodetectScanResultsPage.hpp"
#include "ui/options/gameconfigurations/GameConfigurationsOptions.hpp"

static void MoveTableRows(QTableWidget* source, QTableWidget* destination)
{
	auto selectedRows = source->selectionModel()->selectedRows();

	if (selectedRows.isEmpty())
	{
		return;
	}

	std::sort(selectedRows.begin(), selectedRows.end(),
		[](const auto& lhs, const auto& rhs)
		{
			return rhs < lhs;
		});

	// Insert new rows here to maintain original sorting order.
	const int row = destination->rowCount();

	for (const auto& selectedRow : selectedRows)
	{
		auto name = source->takeItem(selectedRow.row(), 0);
		auto modDirectory = source->takeItem(selectedRow.row(), 1);
		source->removeRow(selectedRow.row());
		
		destination->insertRow(row);
		destination->setItem(row, 0, name);
		destination->setItem(row, 1, modDirectory);
	}

	destination->resizeColumnToContents(0);
}

AutodetectScanResultsPage::AutodetectScanResultsPage(GameConfigurationsOptions* options, QWidget* parent)
	: QWizardPage(parent)
	, _options(options)
{
	_ui.setupUi(this);

	auto style = this->style();

	_ui.Add->setIcon(style->standardIcon(QStyle::StandardPixmap::SP_ArrowDown));
	_ui.Remove->setIcon(style->standardIcon(QStyle::StandardPixmap::SP_ArrowUp));

	connect(_ui.ConfigurationsToAdd->model(), &QAbstractItemModel::rowsInserted,
		this, &AutodetectScanResultsPage::completeChanged);
	connect(_ui.ConfigurationsToAdd->model(), &QAbstractItemModel::rowsRemoved,
		this, &AutodetectScanResultsPage::completeChanged);

	connect(_ui.ConfigurationsFound->selectionModel(), &QItemSelectionModel::selectionChanged, this,
		[this](const QItemSelection& selected)
		{
			bool hasEnabledItems = false;

			for (const auto& selection : selected)
			{
				for (int i = selection.top(); i <= selection.bottom(); ++i)
				{
					if (_ui.ConfigurationsFound->item(i, 0)->flags() & Qt::ItemIsEnabled)
					{
						hasEnabledItems = true;
						break;
					}
				}
			}

			_ui.Add->setEnabled(hasEnabledItems);
		});

	connect(_ui.ConfigurationsToAdd->selectionModel(), &QItemSelectionModel::selectionChanged, this,
		[this](const QItemSelection& selected)
		{
			_ui.Remove->setEnabled(!selected.isEmpty());
		});

	connect(_ui.Add, &QPushButton::clicked, this,
		[this]
		{
			MoveTableRows(_ui.ConfigurationsFound, _ui.ConfigurationsToAdd);
		});

	connect(_ui.Remove, &QPushButton::clicked, this,
		[this]
		{
			MoveTableRows(_ui.ConfigurationsToAdd, _ui.ConfigurationsFound);
		});
}

AutodetectScanResultsPage::~AutodetectScanResultsPage() = default;

void AutodetectScanResultsPage::initializePage()
{
	QTimer::singleShot(0, this, &AutodetectScanResultsPage::OnStartScan);
}

void AutodetectScanResultsPage::cleanupPage()
{
	_ui.ConfigurationsFound->setRowCount(0);
	_ui.ConfigurationsToAdd->setRowCount(0);
}

bool AutodetectScanResultsPage::isComplete() const
{
	return _ui.ConfigurationsToAdd->rowCount() > 0;
}

QTableWidget* AutodetectScanResultsPage::GetConfigurationsToAdd() const
{
	return _ui.ConfigurationsToAdd;
}

void AutodetectScanResultsPage::OnStartScan()
{
	// In case the user changed pages in the moment between setting the timer and this call.
	if (wizard()->currentPage() != this)
	{
		return;
	}

	const QDir gameDirectory{field("GameDirectory").toString()};
	const auto directories = gameDirectory.entryInfoList(QDir::Dirs, QDir::SortFlag::Name);

	{
		QProgressDialog dialog{"Scanning for games...", "Abort scan", 0, directories.size(), this};
		dialog.setWindowModality(Qt::WindowModal);
		dialog.setMinimumDuration(2000);

		for (int index = 0; const auto & directory : directories)
		{
			dialog.setValue(index);

			if (dialog.wasCanceled())
			{
				break;
			}

			const auto absolutePath{directory.absoluteFilePath()};

			const QDir modDirectory{absolutePath};

			if (modDirectory.exists("liblist.gam"))
			{
				const int row = _ui.ConfigurationsFound->rowCount();
				_ui.ConfigurationsFound->setRowCount(row + 1);

				auto name = new QTableWidgetItem("Unknown game");

				if (const auto keyvalues = LiblistReader::Read(modDirectory.filePath("liblist.gam").toStdString());
					keyvalues)
				{
					if (auto it = keyvalues->find("game"); it != keyvalues->end())
					{
						if (auto liblistName = QString::fromStdString(it->second).trimmed(); !liblistName.isEmpty())
						{
							name->setText(liblistName);
						}
					}
				}

				_ui.ConfigurationsFound->setItem(row, 0, name);

				auto modDirectory = new QTableWidgetItem(absolutePath);
				modDirectory->setFlags(modDirectory->flags() & ~Qt::ItemIsEditable);
				_ui.ConfigurationsFound->setItem(row, 1, modDirectory);

				// Filter out directories that already have a configuration.
				if (std::find_if(_options->GameConfigurations.begin(), _options->GameConfigurations.end(),
					[&](const auto& candidate)
					{
						return candidate->ModDirectory == absolutePath
						|| candidate->BaseGameDirectory == absolutePath;
					}) != _options->GameConfigurations.end())
				{
					name->setFlags(name->flags() & ~(Qt::ItemIsEnabled | Qt::ItemIsSelectable));
					modDirectory->setFlags(modDirectory->flags() & ~(Qt::ItemIsEnabled | Qt::ItemIsSelectable));
				}
			}

			++index;
		}

		dialog.setValue(directories.size());
	}

	_ui.ConfigurationsFound->resizeColumnToContents(0);

	// Select the first row to make it clear that these can be moved.
	_ui.ConfigurationsFound->selectionModel()->select(_ui.ConfigurationsFound->model()->index(0, 0),
		QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}
