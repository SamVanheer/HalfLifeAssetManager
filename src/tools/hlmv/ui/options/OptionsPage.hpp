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

	QString GetId() const { return _id; }

	QString GetTitle() const { return _title; }

	virtual QWidget* GetWidget(EditorContext* editorContext);

	virtual void ApplyChanges(QSettings& settings);

	virtual void DestroyWidget();

protected:
	void SetId(QString&& id)
	{
		_id = std::move(id);
	}

	void SetTitle(QString&& title)
	{
		_title = std::move(title);
	}

	void SetWidgetFactory(WidgetFactory&& factory)
	{
		_widgetFactory = std::move(factory);
	}

protected:
	QString _id;
	QString _title;

	WidgetFactory _widgetFactory;
	QPointer<OptionsWidget> _widget;
};
}
}
