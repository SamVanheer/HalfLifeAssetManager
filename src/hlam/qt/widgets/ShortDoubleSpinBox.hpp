#pragma once

#include <QDoubleSpinBox>

namespace qt::widgets
{
class ShortDoubleSpinBox : public QDoubleSpinBox
{
	Q_OBJECT

public:
	ShortDoubleSpinBox(QWidget* parent = nullptr);
	~ShortDoubleSpinBox();

	QString textFromValue(double val) const override;
};
}
