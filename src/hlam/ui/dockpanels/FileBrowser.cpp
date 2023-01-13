#include <algorithm>
#include <cstdio>
#include <utility>
#include <vector>

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QString>
#include <QStringList>

#include "application/AssetManager.hpp"
#include "application/Assets.hpp"

#include "settings/ApplicationSettings.hpp"
#include "settings/GameConfigurationsSettings.hpp"

#include "ui/MainWindow.hpp"
#include "ui/dialogs/SelectGameConfigurationDialog.hpp"
#include "ui/dockpanels/FileBrowser.hpp"

class AssetFilterModel final : public QSortFilterProxyModel
{
public:
	explicit AssetFilterModel(QObject* parent = nullptr)
		: QSortFilterProxyModel(parent)
	{
	}

	void SetProvider(AssetProvider* provider)
	{
		if (_provider != provider)
		{
			_provider = provider;

			if (_provider)
			{
				_extensions = _provider->GetFileTypes();
			}
			else
			{
				_extensions.clear();
			}

			invalidate();
		}
	}

protected:
	bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override
	{
		if (!_provider)
		{
			return true;
		}

		auto fileSystemModel = static_cast<QFileSystemModel*>(sourceModel());

		if (!fileSystemModel)
		{
			return false;
		}

		QModelIndex index = fileSystemModel->index(source_row, 0, source_parent);

		const auto fileInfo = fileSystemModel->fileInfo(index);

		if (!fileInfo.isFile())
		{
			return true;
		}

		// Source 1 has vpk files that start with valid model data; this check filters them out.
		if (!_extensions.contains(fileInfo.suffix()))
		{
			return false;
		}

		const auto fileName = fileInfo.absoluteFilePath();

		FILE* file = fopen(fileName.toStdString().c_str(), "rb");

		if (!file)
		{
			return false;
		}

		const bool acceptFile = _provider->IsCandidateForLoading(fileName, file);

		fclose(file);

		return acceptFile;
	}

private:
	AssetProvider* _provider{};
	QStringList _extensions;
};

FileBrowser::FileBrowser(AssetManager* application, QWidget* parent)
	: QWidget(parent)
	, _application(application)
	, _model(new QFileSystemModel(this))
	, _filterModel(new AssetFilterModel(this))
{
	_ui.setupUi(this);

	// Allow navigation to parent directory using dotdot.
	_model->setFilter(QDir::AllEntries | QDir::AllDirs);

	_filterModel->setSourceModel(_model);
	_ui.FileView->setModel(_filterModel);
	_ui.FileView->setColumnWidth(0, 250);

	auto rootDirectory = _application->GetApplicationSettings()->GetFileListRootDirectory();

	if (rootDirectory.isEmpty() || !QFileInfo::exists(rootDirectory))
	{
		rootDirectory = QDir::currentPath();
	}

	SetRootDirectory(rootDirectory);

	connect(_ui.Filters, qOverload<int>(&QComboBox::currentIndexChanged), this,
		[this]
		{
			_filterModel->SetProvider(_ui.Filters->currentData().value<AssetProvider*>());
		});

	connect(_ui.FileView, &QTreeView::activated, this, &FileBrowser::OnFileSelected);
	connect(_ui.FileView, &QTreeView::doubleClicked, this, &FileBrowser::OnFileDoubleClicked);

	connect(_ui.BrowseRoot, &QPushButton::clicked, this,
		[this]
		{
			const QString fileName{QFileDialog::getExistingDirectory(this, {}, _ui.Root->text())};

			if (!fileName.isEmpty())
			{
				SetRootDirectory(fileName);
			}
		});

	connect(_ui.FromConfiguration, &QPushButton::clicked, this,
		[this]
		{
			SelectGameConfigurationDialog dialog{
				_application->GetApplicationSettings()->GetGameConfigurations(),
				_application->GetMainWindow()};

				if (dialog.exec() == QDialog::Accepted)
				{
					if (const auto gameConfiguration = dialog.GetSelectedConfiguration(); gameConfiguration)
					{
						QString directory;

						if (!gameConfiguration->ModDirectory.isEmpty())
						{
							directory = gameConfiguration->ModDirectory;
						}
						else
						{
							directory = gameConfiguration->BaseGameDirectory;
						}

						if (directory.isEmpty())
						{
							QMessageBox::critical(_application->GetMainWindow(), "Error",
								"The selected game configuration has not been configured and cannot be used.");
							return;
						}

						if (!QFileInfo::exists(directory))
						{
							QMessageBox::critical(_application->GetMainWindow(), "Error",
								"The selected game configuration has invalid paths and cannot be used.");
							return;
						}

						SetRootDirectory(directory);
					}
				}
		});

	connect(_ui.FileView->selectionModel(), &QItemSelectionModel::selectionChanged, this,
		[this](const QItemSelection& selected)
		{
			bool hasEnabledItems = false;

			for (const auto& selection : _filterModel->mapSelectionToSource(selected))
			{
				for (const auto& index : selection.indexes())
				{
					if (!_model->isDir(index))
					{
						hasEnabledItems = true;
						break;
					}
				}

				if (hasEnabledItems)
				{
					break;
				}
			}

			_ui.OpenSelected->setEnabled(hasEnabledItems);
		});

	connect(_ui.OpenSelected, &QPushButton::clicked, this,
		[this] { MaybeOpenFiles(_ui.FileView->selectionModel()->selectedIndexes()); });

	//Build list of provider filters
	std::vector<std::pair<QString, AssetProvider*>> filters;

	for (const auto provider : application->GetAssetProviderRegistry()->GetAssetProviders())
	{
		filters.emplace_back(
			QString{"%1 .%2 Files"}.arg(provider->GetProviderName()).arg(provider->GetPreferredFileType()),
			provider);
	}

	std::sort(filters.begin(), filters.end(), [](const auto& lhs, const auto& rhs)
		{
			return lhs.first.compare(rhs.first, Qt::CaseSensitivity::CaseInsensitive) < 0;
		});

	filters.insert(filters.begin(), {QStringLiteral("All Files"), nullptr});

	for (const auto& filter : filters)
	{
		_ui.Filters->addItem(filter.first, QVariant::fromValue(filter.second));
	}

	const auto filterName = _application->GetApplicationSettings()->GetFileListFilter();

	int filterIndex = 0;

	if (auto it = std::find_if(filters.begin(), filters.end(),
		[&](const auto& candidate)
		{
			return candidate.second && candidate.second->GetProviderName() == filterName;
		}); it != filters.end())
	{
		filterIndex = it - filters.begin();
	}

	_ui.Filters->setCurrentIndex(filterIndex);
}

FileBrowser::~FileBrowser()
{
	_application->GetApplicationSettings()->SetFileListRootDirectory(_ui.Root->text());

	auto provider = _ui.Filters->currentData().value<AssetProvider*>();
	auto providerName = provider ? provider->GetProviderName() : QString{};
	_application->GetApplicationSettings()->SetFileListFilter(providerName);
}

void FileBrowser::SetRootDirectory(const QString& directory)
{
	_model->setRootPath(directory);
	_ui.FileView->setRootIndex(_filterModel->mapFromSource(_model->index(directory)));
	_ui.Root->setText(directory);
}

void FileBrowser::MaybeOpenFiles(const QModelIndexList& indices)
{
	QStringList fileNames;

	for (const auto& index : indices)
	{
		if (index.column() != 0)
		{
			continue;
		}

		const auto fileName = _model->filePath(_filterModel->mapToSource(index));

		if (!QFileInfo{fileName}.isFile())
		{
			continue;
		}

		fileNames.append(fileName);
	}

	emit FilesSelected(fileNames);
}

void FileBrowser::OnFileSelected(const QModelIndex& index)
{
	if (index.isValid())
	{
		MaybeOpenFiles({index});
	}
}

void FileBrowser::OnFileDoubleClicked(const QModelIndex& index)
{
	const auto fileName = _model->filePath(_filterModel->mapToSource(index));
	const QFileInfo info{fileName};

	if (info.isDir())
	{
		SetRootDirectory(info.absoluteFilePath());
	}
}
