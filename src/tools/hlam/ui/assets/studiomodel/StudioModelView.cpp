#include "ui/assets/studiomodel/StudioModelView.hpp"

namespace ui::assets::studiomodel
{
StudioModelView::StudioModelView(QWidget* parent)
	: QWidget(parent)
{
	_ui.setupUi(this);

	_ui.ViewSelection->setShape(QTabBar::Shape::RoundedSouth);
	_ui.ViewSelection->setToolTip("The current view");

	connect(_ui.ViewSelection, &QTabBar::currentChanged, _ui.View, &QStackedWidget::setCurrentIndex);
}

StudioModelView::~StudioModelView() = default;

void StudioModelView::AddWidget(QWidget* widget, const QString& label)
{
	_ui.View->addWidget(widget);
	_ui.ViewSelection->addTab(label);
}

void StudioModelView::SetCurrentWidget(QWidget* widget)
{
	int index = -1;

	if (widget)
	{
		for (int i = 0; i < _ui.View->count(); ++i)
		{
			if (_ui.View->widget(i) == widget)
			{
				index = i;
				break;
			}
		}

		//Not contained in this view
		return;
	}

	_ui.ViewSelection->setCurrentIndex(index);
}
}
