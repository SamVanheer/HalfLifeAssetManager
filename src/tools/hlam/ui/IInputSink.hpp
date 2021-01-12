#pragma once

#include <QMouseEvent>
#include <QWheelEvent>

namespace ui
{
class IInputSink
{
public:
	virtual ~IInputSink() {}

	virtual void OnMouseEvent(QMouseEvent* event) = 0;

	virtual void OnWheelEvent(QWheelEvent* event) = 0;
};
}
