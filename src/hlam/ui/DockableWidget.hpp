#pragma once

#include <QBoxLayout>
#include <QWidget>

#include "qt/QtUtilities.hpp"

/**
*	@brief Base class for widgets that may be wrapped by a <tt>QDockWidget</tt>.
*/
class DockableWidget : public QWidget
{
public:

	virtual void OnLayoutDirectionChanged(QBoxLayout::Direction direction)
	{
		// The default behavior is to adjust widgets that have a box layout as the root layout.
		qt::TrySetBoxLayoutDirection(this, direction);
	}

	virtual void OnVisibilityChanged(bool visible) {}
};
