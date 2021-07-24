#include <QMouseEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>

#include "TextureWidget.hpp"

namespace ui
{
void TextureWidget::paintEvent(QPaintEvent* event)
{
	QStyleOption opt;
	opt.init(this);

	QPainter painter{this};

	style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

	QBrush backgroundBrush{_backgroundColor};

	painter.fillRect(geometry(), backgroundBrush);

	if (!_image.isNull())
	{
		const auto center{QPoint{width(), height()} / 2};

		auto topLeft = center - (QPoint{_image.width(), _image.height()} / 2);

		topLeft += _imageOffset;

		painter.drawImage(topLeft, _image);
	}
}

void TextureWidget::mousePressEvent(QMouseEvent* event)
{
	//Only reset the position if a single button is down
	if (event->buttons() & (Qt::MouseButton::LeftButton | Qt::MouseButton::RightButton) && !(event->buttons() & (event->buttons() << 1)))
	{
		_dragPosition = event->pos();

		_trackedMouseButtons.setFlag(event->button(), true);
	}
}

void TextureWidget::mouseReleaseEvent(QMouseEvent* event)
{
	_trackedMouseButtons.setFlag(event->button(), false);
}

void TextureWidget::mouseMoveEvent(QMouseEvent* event)
{
	const auto position{event->pos()};

	const auto delta = position - _dragPosition;

	if (_trackedMouseButtons & Qt::MouseButton::LeftButton && event->buttons() & Qt::MouseButton::LeftButton)
	{
		_imageOffset += delta;
		update();
	}
	else if (_trackedMouseButtons & Qt::MouseButton::RightButton && event->buttons() & Qt::MouseButton::RightButton)
	{
		const double zoomAdjust = delta.y() / -20.0;

		emit ScaleChanged(zoomAdjust);
	}

	_dragPosition = position;
}
}
