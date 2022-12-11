#include <cassert>

#include "ui/options/OptionsPage.hpp"

namespace ui::options
{
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

void OptionsPage::ApplyChanges(QSettings& settings)
{
	assert(_widgetFactory);

	if (_widgetFactory && _widget)
	{
		_widget->ApplyChanges(settings);
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
}
