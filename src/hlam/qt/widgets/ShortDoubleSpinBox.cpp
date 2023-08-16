#include <QLocale>

#include "qt/widgets/ShortDoubleSpinBox.hpp"

namespace qt::widgets
{
ShortDoubleSpinBox::ShortDoubleSpinBox(QWidget* parent)
	: QDoubleSpinBox(parent)
{
}

ShortDoubleSpinBox::~ShortDoubleSpinBox() = default;

QString ShortDoubleSpinBox::textFromValue(double val) const
{
	return QString::number(val, 'f', QLocale::FloatingPointShortest);
}
}
