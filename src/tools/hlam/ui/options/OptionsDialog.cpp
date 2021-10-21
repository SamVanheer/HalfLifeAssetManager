#include <cmath>
#include <unordered_map>

#include <QApplication>
#include <QDesktopWidget>

#include "qt/HashFunctions.hpp"

#include "ui/EditorContext.hpp"

#include "ui/options/OptionsDialog.hpp"
#include "ui/options/OptionsPage.hpp"
#include "ui/options/OptionsPageRegistry.hpp"

namespace ui::options
{
const int PageIndexRole = Qt::ItemDataRole::UserRole;

OptionsDialog::OptionsDialog(EditorContext* editorContext, QWidget* parent)
	: QDialog(parent)
	, _editorContext(editorContext)
{
	_ui.setupUi(this);

	//Set the dialog size to a fraction of the current screen
	const QRect screenSize = QApplication::desktop()->screenGeometry(this);

	this->resize(static_cast<int>(std::ceil(screenSize.width() * 0.9)), static_cast<int>(std::ceil(screenSize.height() * 0.9)));

	std::unordered_map<QString, QTreeWidgetItem*> categoryMap;

	_pages = _editorContext->GetOptionsPageRegistry()->GetPages();

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

			it = categoryMap.emplace(page->GetCategory(), newCategory).first;
		}

		auto pageItem = new QTreeWidgetItem();

		pageItem->setText(0, page->GetPageTitle());
		pageItem->setData(0, PageIndexRole, _ui.OptionsPagesStack->count());

		it->second->addChild(pageItem);

		_ui.OptionsPagesStack->addWidget(page->GetWidget(_editorContext));
	}

	connect(_ui.OptionsPagesList, &QTreeWidget::currentItemChanged, this, &OptionsDialog::OnPageSelected);
	connect(_ui.DialogButtons, &QDialogButtonBox::clicked, this, &OptionsDialog::OnButtonClicked);

	_ui.OptionsPagesList->setCurrentItem(_ui.OptionsPagesList->topLevelItem(0));
}

OptionsDialog::~OptionsDialog()
{
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
		auto settings = _editorContext->GetSettings();

		for (auto page : _pages)
		{
			page->ApplyChanges(*settings);
		}

		settings->sync();

		emit _editorContext->SettingsChanged();
	}
}
}
