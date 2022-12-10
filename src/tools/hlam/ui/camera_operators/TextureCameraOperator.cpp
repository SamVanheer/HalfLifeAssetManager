#include "entity/TextureEntity.hpp"

#include "ui/camera_operators/TextureCameraOperator.hpp"

namespace ui::camera_operators
{
void TextureCameraOperator::MouseEvent(QMouseEvent& event)
{
	switch (event.type())
	{
	case QEvent::Type::MouseButtonPress:
	{
		//Only reset the position if a single button is down
		if (event.buttons() & (Qt::MouseButton::LeftButton | Qt::MouseButton::RightButton) && !(event.buttons() & (event.buttons() << 1)))
		{
			const auto position = event.pos();

			_dragPosition.x = position.x();
			_dragPosition.y = position.y();

			_trackedMouseButtons.setFlag(event.button(), true);
		}
		break;
	}

	case QEvent::Type::MouseButtonRelease:
	{
		_trackedMouseButtons.setFlag(event.button(), false);
		break;
	}

	case QEvent::Type::MouseMove:
	{
		const glm::ivec2 position{event.pos().x(), event.pos().y()};

		const glm::ivec2 delta = position - _dragPosition;

		if (_trackedMouseButtons & Qt::MouseButton::LeftButton && event.buttons() & Qt::MouseButton::LeftButton)
		{
			_textureEntity->XOffset += delta.x;
			_textureEntity->YOffset += delta.y;
		}
		else if (_trackedMouseButtons & Qt::MouseButton::RightButton && event.buttons() & Qt::MouseButton::RightButton)
		{
			const double zoomAdjust = delta.y / -20.0;

			emit ScaleChanged(zoomAdjust);
		}

		_dragPosition = position;
		break;
	}
	}
}
}
