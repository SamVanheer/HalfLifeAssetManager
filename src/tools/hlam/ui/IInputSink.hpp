#pragma once

#include <QMouseEvent>
#include <QWheelEvent>

namespace ui
{
class IInputSink
{
public:
	virtual ~IInputSink() = 0 {}

	virtual void OnMouseEvent(QMouseEvent* event) = 0;
};
}
