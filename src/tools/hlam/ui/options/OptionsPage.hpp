#pragma once

#include <functional>
#include <memory>

#include <QPointer>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QWidget>

namespace ui
{
class EditorContext;

namespace options
{
class OptionsWidget : public QWidget
{
public:
	using QWidget::QWidget;

	virtual void ApplyChanges(QSettings& settings) = 0;

	virtual void Destroy() {}
};

/**
*	@brief Base class for options pages
*	Responsible for providing the unique id, which is used to identify the page as well as sort it alphabetically in the list of pages,
*	the title and the widget
*/
class OptionsPage
{
public:
	using WidgetFactory = std::function<OptionsWidget* (EditorContext* editorContext)>;

	virtual ~OptionsPage() = default;

	QString GetCategory() const { return _category; }

	QString GetCategoryTitle() const { return _categoryTitle; }

	QString GetId() const { return _id; }

	QString GetPageTitle() const { return _pageTitle; }

	virtual QWidget* GetWidget(EditorContext* editorContext);

	virtual void ApplyChanges(QSettings& settings);

	virtual void DestroyWidget();

protected:
	void SetCategory(QString&& category)
	{
		_category = std::move(category);
	}

	void SetCategoryTitle(QString&& title)
	{
		_categoryTitle = std::move(title);
	}

	void SetId(QString&& id)
	{
		_id = std::move(id);
	}

	void SetPageTitle(QString&& title)
	{
		_pageTitle = std::move(title);
	}

	void SetWidgetFactory(WidgetFactory&& factory)
	{
		_widgetFactory = std::move(factory);
	}

protected:
	QString _category;
	QString _categoryTitle;
	QString _id;
	QString _pageTitle;

	WidgetFactory _widgetFactory;
	QPointer<OptionsWidget> _widget;
};
}
}
