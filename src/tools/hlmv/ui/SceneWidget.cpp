#include <QWidget>

#include "ui/SceneWidget.hpp"

namespace ui
{
//TODO: implement rendering logic
SceneWidget::SceneWidget(QWidget* parent)
{
	_container = QWidget::createWindowContainer(this, parent);
}

SceneWidget::~SceneWidget() = default;
}
