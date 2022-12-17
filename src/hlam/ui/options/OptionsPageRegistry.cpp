#include <algorithm>
#include <cassert>

#include <QDebug>

#include "ui/options/OptionsPage.hpp"
#include "ui/options/OptionsPageRegistry.hpp"

static bool ComparePages(const std::unique_ptr<OptionsPage>& lhs, const std::unique_ptr<OptionsPage>& rhs)
{
	if (lhs->GetCategory() != rhs->GetCategory())
	{
		return lhs->GetCategory().compare(rhs->GetCategory(), Qt::CaseSensitivity::CaseInsensitive) < 0;
	}

	return lhs->GetId().compare(rhs->GetId(), Qt::CaseSensitivity::CaseInsensitive) < 0;
}

OptionsPageRegistry::OptionsPageRegistry() = default;
OptionsPageRegistry::~OptionsPageRegistry() = default;

OptionsPage* OptionsPageRegistry::GetPageById(const QString& id) const
{
	if (auto it = std::find_if(_pages.begin(), _pages.end(), [&](const auto& candidate)
		{
			return candidate->GetId() == id;
		}); it != _pages.end())
	{
		return it->get();
	}

	return nullptr;
}

std::vector<OptionsPage*> OptionsPageRegistry::GetPages()
{
	if (!_sorted)
	{
		_sorted = true;

		std::sort(_pages.begin(), _pages.end(), ComparePages);
	}

	std::vector<OptionsPage*> pages;

	pages.reserve(_pages.size());

	std::transform(_pages.begin(), _pages.end(), std::back_inserter(pages), [](const auto& page)
		{
			return page.get();
		});

	return pages;
}

void OptionsPageRegistry::AddPage(std::unique_ptr<OptionsPage>&& page)
{
	assert(page);

	if (page->GetId().isEmpty())
	{
		qDebug() << "Options pages must have an id";
		return;
	}

	if (page->GetCategory().isEmpty())
	{
		qDebug() << "The options page with id \"" << page->GetId() << "\" has no category";
		return;
	}

	if (page->GetCategoryTitle().isEmpty())
	{
		qDebug() << "The options page with id \"" << page->GetId() << "\" has no category title";
		return;
	}

	if (page->GetPageTitle().isEmpty())
	{
		qDebug() << "The options page with id \"" << page->GetId() << "\" has no page title";
		return;
	}

	if (GetPageById(page->GetId()))
	{
		qDebug() << "The options page with id \"" << page->GetId() << "\" has already been registered";
		return;
	}

	_pages.emplace_back(std::move(page));

	_sorted = false;
}
