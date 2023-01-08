#include <algorithm>
#include <utility>
#include <vector>

#include <QDir>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QSettings>
#include <QString>

#include "assets/Assets.hpp"

#include "settings/GameConfigurationsSettings.hpp"

#include "application/EditorContext.hpp"
#include "ui/FileListPanel.hpp"

FileListPanel::FileListPanel(EditorContext* editorContext, QWidget* parent)
	: QWidget(parent)
{
	_ui.setupUi(this);

	_model = new QFileSystemModel(this);

	{
		const auto settings = editorContext->GetSettings();
		settings->beginGroup("FileList");
		_model->setNameFilterDisables(!settings->value("HideFilesThatDontMatch", true).toBool());
		settings->endGroup();
	}

	_ui.FileView->setModel(_model);

	_ui.FileView->setColumnWidth(0, 250);

	//Initialize to current game configuration
	UpdateCurrentRootPath(editorContext->GetGameConfigurations()->GetDefaultConfiguration());

	/*
	connect(editorContext->GetGameConfigurations(), &settings::GameConfigurationsSettings::DefaultConfigurationChanged,
		this, &FileListPanel::UpdateCurrentRootPath);
	*/

	connect(_ui.Filters, qOverload<int>(&QComboBox::currentIndexChanged), this, &FileListPanel::OnFilterChanged);

	connect(_ui.HideFilesThatDontMatch, &QCheckBox::stateChanged, [this, editorContext]
		{
			_model->setNameFilterDisables(!_ui.HideFilesThatDontMatch->isChecked());

			const auto settings = editorContext->GetSettings();
			settings->beginGroup("FileList");
			settings->setValue("HideFilesThatDontMatch", _ui.HideFilesThatDontMatch->isChecked());
			settings->endGroup();
		});

	connect(_ui.FileView, &QTreeView::activated, this, &FileListPanel::OnFileSelected);

	connect(_ui.BrowseRoot, &QPushButton::clicked, [this]
		{
			const QString fileName{QFileDialog::getExistingDirectory(this, {}, _ui.Root->text())};

			if (!fileName.isEmpty())
			{
				SetRootDirectory(fileName);
			}
		});

	//Build list of file extension filters
	std::vector<std::pair<QString, QString>> filters;

	for (const auto provider : editorContext->GetAssetProviderRegistry()->GetAssetProviders())
	{
		for (const auto& fileType : provider->GetFileTypes())
		{
			filters.emplace_back(QString{"%1 .%2 Files"}.arg(provider->GetProviderName()).arg(fileType), QString{"*.%1"}.arg(fileType));
		}
	}

	std::sort(filters.begin(), filters.end(), [](const auto& lhs, const auto& rhs)
		{
			return lhs.first.compare(rhs.first, Qt::CaseSensitivity::CaseInsensitive) < 0;
		});

	filters.insert(filters.begin(), {QStringLiteral("All Files"), QStringLiteral("*.*")});

	for (const auto& filter : filters)
	{
		_ui.Filters->addItem(filter.first, filter.second);
	}
}

FileListPanel::~FileListPanel() = default;

void FileListPanel::SetRootDirectory(const QString& directory)
{
	_model->setRootPath(directory);
	_ui.FileView->setRootIndex(_model->index(directory));
	_ui.Root->setText(directory);
}

void FileListPanel::UpdateCurrentRootPath(const GameConfiguration* defaultConfiguration)
{
	QString directory;

	if (defaultConfiguration)
	{
		if (!defaultConfiguration->ModDirectory.isEmpty())
		{
			directory = defaultConfiguration->ModDirectory;
		}
		else
		{
			directory = defaultConfiguration->BaseGameDirectory;
		}
	}

	if (directory.isEmpty())
	{
		directory = QDir::currentPath();
	}

	SetRootDirectory(directory);
}

void FileListPanel::OnFilterChanged()
{
	QStringList filters;

	filters.append(_ui.Filters->currentData().toString());

	_model->setNameFilters(filters);
}

void FileListPanel::OnFileSelected(const QModelIndex& index)
{
	if (index.isValid())
	{
		emit FileSelected(_model->filePath(index));
	}
}
