#include <algorithm>
#include <utility>
#include <vector>

#include <QDir>
#include <QFileSystemModel>
#include <QString>

#include "ui/EditorContext.hpp"
#include "ui/FileListPanel.hpp"

#include "ui/assets/Assets.hpp"

#include "ui/settings/GameConfigurationsSettings.hpp"

namespace ui
{
FileListPanel::FileListPanel(EditorContext* editorContext, QWidget* parent)
	: QWidget(parent)
{
	_ui.setupUi(this);

	_model = new QFileSystemModel(this);

	_model->setNameFilterDisables(false);

	_ui.FileView->setModel(_model);

	const auto gameConfigurations = editorContext->GetGameConfigurations();

	const auto activeConfiguration = gameConfigurations->GetActiveConfiguration();

	QString directory;

	if (activeConfiguration.first)
	{
		if (activeConfiguration.second)
		{
			directory = QString{"%1/%2"}.arg(activeConfiguration.first->GetInstallationPath()).arg(activeConfiguration.second->GetDirectory());
		}
		else
		{
			directory = activeConfiguration.first->GetInstallationPath();
		}
	}

	if (directory.isEmpty())
	{
		directory = QDir::currentPath();
	}

	_model->setRootPath(directory);

	_ui.FileView->setRootIndex(_model->index(directory));

	connect(_ui.Filters, qOverload<int>(&QComboBox::currentIndexChanged), this, &FileListPanel::OnFilterChanged);
	connect(_ui.FileView, &QTreeView::activated, this, &FileListPanel::OnFileSelected);

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
}
