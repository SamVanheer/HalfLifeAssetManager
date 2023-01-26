#include <cassert>
#include <cmath>
#include <unordered_map>

#include <QApplication>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QFileInfo>
#include <QMap>
#include <QSettings>
#include <QUrl>

#include "settings/ApplicationSettings.hpp"

#include "application/AssetManager.hpp"

#include "ui/options/OptionsDialog.hpp"
#include "ui/options/OptionsPage.hpp"
#include "ui/options/OptionsPageRegistry.hpp"

const int PageIndexRole = Qt::ItemDataRole::UserRole;

OptionsDialog::OptionsDialog(AssetManager* application, QWidget* parent)
	: QDialog(parent)
	, _application(application)
{
	_ui.setupUi(this);

	//Set the dialog size to a fraction of the current screen
	const QRect screenSize = QApplication::desktop()->screenGeometry(this);

	this->resize(static_cast<int>(std::ceil(screenSize.width() * 0.9)), static_cast<int>(std::ceil(screenSize.height() * 0.9)));

	QMap<QString, QTreeWidgetItem*> categoryMap;

	_pages = _application->GetOptionsPageRegistry()->GetPages();

	for (auto page : _pages)
	{
		auto it = categoryMap.find(page->GetCategory());

		if (it == categoryMap.end())
		{
			auto newCategory = new QTreeWidgetItem();

			newCategory->setText(0, page->GetCategoryTitle());
			//Set the page index to the first page of this category
			newCategory->setData(0, PageIndexRole, _ui.OptionsPagesStack->count());

			_ui.OptionsPagesList->addTopLevelItem(newCategory);

			newCategory->setExpanded(true);

			it = categoryMap.insert(page->GetCategory(), newCategory);
		}

		auto pageItem = new QTreeWidgetItem();

		pageItem->setText(0, page->GetPageTitle());
		pageItem->setData(0, PageIndexRole, _ui.OptionsPagesStack->count());

		it.value()->addChild(pageItem);

		_ui.OptionsPagesStack->addWidget(page->GetWidget(_application));
	}

	connect(_ui.OptionsPagesList, &QTreeWidget::currentItemChanged, this, &OptionsDialog::OnPageSelected);
	connect(_ui.DialogButtons, &QDialogButtonBox::clicked, this, &OptionsDialog::OnButtonClicked);
	connect(_ui.OpenConfigDirectory, &QPushButton::clicked, this, [this]()
		{
			const QString fileName = _application->GetSettings()->fileName();
			const QFileInfo fileInfo{fileName};
			const QString fullPath = fileInfo.absolutePath();
			QDesktopServices::openUrl(QUrl{QString{"file:///%1"}.arg(fullPath), QUrl::TolerantMode});
		});

	bool setDefaultPage = true;

	if (!_lastActivePageName.isEmpty())
	{
		const auto items = _ui.OptionsPagesList->findItems(_lastActivePageName,
			Qt::MatchFixedString | Qt::MatchRecursive);

		assert(items.size() == 1);

		if (!items.isEmpty())
		{
			_ui.OptionsPagesList->setCurrentItem(items.front());
			setDefaultPage = false;
		}
	}

	if (setDefaultPage)
	{
		_ui.OptionsPagesList->setCurrentItem(_ui.OptionsPagesList->topLevelItem(0));
	}
}

OptionsDialog::~OptionsDialog()
{
	const auto currentPage = _ui.OptionsPagesList->currentItem();

	if (currentPage)
	{
		_lastActivePageName = currentPage->text(0);
	}

	for (auto page : _pages)
	{
		page->DestroyWidget();
	}
}

void OptionsDialog::OnPageSelected(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
	bool ok{false};

	const int pageIndex = current->data(0, PageIndexRole).toInt(&ok);

	if (ok)
	{
		_ui.OptionsPagesStack->setCurrentIndex(pageIndex);
	}
}

void OptionsDialog::OnButtonClicked(QAbstractButton* button)
{
	const auto role = _ui.DialogButtons->buttonRole(button);

	if (role == QDialogButtonBox::ButtonRole::AcceptRole ||
		role == QDialogButtonBox::ButtonRole::ApplyRole)
	{
		for (auto page : _pages)
		{
			page->ApplyChanges();
		}

		_application->GetApplicationSettings()->SaveSettings();

		_application->GetApplicationSettings()->GetSettings()->sync();

		emit _application->SettingsChanged();
	}
}
