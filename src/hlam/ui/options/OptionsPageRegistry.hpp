#pragma once

#include <memory>
#include <vector>

#include <QString>
#include <QWidget>

namespace ui::options
{
class OptionsPage;

class OptionsPageRegistry final
{
public:
	OptionsPageRegistry();
	~OptionsPageRegistry();

	OptionsPage* GetPageById(const QString& id) const;

	std::vector<OptionsPage*> GetPages();

	void AddPage(std::unique_ptr<OptionsPage>&& page);

private:
	std::vector<std::unique_ptr<OptionsPage>> _pages;

	bool _sorted{false};
};
}
