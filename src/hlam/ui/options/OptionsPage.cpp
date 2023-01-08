#include <cassert>

#include "ui/options/OptionsPage.hpp"

QWidget* OptionsPage::GetWidget(AssetManager* application)
{
	assert(application);
	assert(_widgetFactory);

	if (!_widget && application && _widgetFactory)
	{
		_widget = _widgetFactory(application);
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
