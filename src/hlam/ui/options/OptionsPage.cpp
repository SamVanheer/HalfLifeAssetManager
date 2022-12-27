#include <cassert>

#include "ui/options/OptionsPage.hpp"

QWidget* OptionsPage::GetWidget(EditorContext* editorContext)
{
	assert(editorContext);
	assert(_widgetFactory);

	if (!_widget && editorContext && _widgetFactory)
	{
		_widget = _widgetFactory(editorContext);
	}

	return _widget;
}

void OptionsPage::ApplyChanges()
{
	assert(_widgetFactory);

	if (_widgetFactory && _widget)
	{
		_widget->ApplyChanges();
	}
}

void OptionsPage::DestroyWidget()
{
	assert(_widgetFactory);

	if (_widgetFactory && _widget)
	{
		_widget->Destroy();
		delete _widget;
	}
}
