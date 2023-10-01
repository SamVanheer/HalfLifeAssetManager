#pragma once

#include <QBoxLayout>
#include <QString>
#include <QWidget>

namespace qt
{
// On Linux executables don't have an extension so we can't filter for them.
#ifdef WIN32
const inline QString ExeFilter{ QStringLiteral("Executable Files (*.exe);;All Files (*.*)") };
#else
const inline QString ExeFilter{ QStringLiteral("All Files (*)") };
#endif

bool LaunchDefaultProgram(const QString& fileName);

QString GetImagesFileFilter();

QString GetSeparatedImagesFileFilter();

inline QBoxLayout::Direction GetDirectionForDockArea(Qt::DockWidgetArea area)
{
	switch (area)
	{
	case Qt::DockWidgetArea::TopDockWidgetArea:
	case Qt::DockWidgetArea::BottomDockWidgetArea:
		return QBoxLayout::Direction::LeftToRight;

	default:
		return QBoxLayout::Direction::TopToBottom;
	}
}

inline Qt::Orientation GetOrientationForDockArea(Qt::DockWidgetArea area)
{
	switch (area)
	{
	case Qt::DockWidgetArea::TopDockWidgetArea:
	case Qt::DockWidgetArea::BottomDockWidgetArea:
		return Qt::Orientation::Horizontal;

	default:
		return Qt::Orientation::Vertical;
	}
}

inline void TrySetBoxLayoutDirection(QWidget* widget, QBoxLayout::Direction direction)
{
	if (!widget)
	{
		return;
	}

	if (auto layout = qobject_cast<QBoxLayout*>(widget->layout()); layout)
	{
		layout->setDirection(direction);
	}
}
}
