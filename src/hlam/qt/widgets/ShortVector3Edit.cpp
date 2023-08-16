#include <QDebug>

#include "qt/widgets/ShortVector3Edit.hpp"

namespace qt::widgets
{
ShortVector3Edit::ShortVector3Edit(QWidget* parent)
	: QWidget(parent)
{
	_ui.setupUi(this);

	SetRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
	SetDecimals(6);

	connect(_ui.XEdit, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ShortVector3Edit::OnValueChanged);
	connect(_ui.YEdit, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ShortVector3Edit::OnValueChanged);
	connect(_ui.ZEdit, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ShortVector3Edit::OnValueChanged);
}

ShortVector3Edit::~ShortVector3Edit() = default;

glm::vec3 ShortVector3Edit::GetValue() const
{
	return
	{
		_ui.XEdit->value(),
		_ui.YEdit->value(),
		_ui.ZEdit->value()
	};
}

double ShortVector3Edit::GetMinimum(int axis) const
{
	if (auto spinBox = SpinBoxByAxis(axis); spinBox)
	{
		return spinBox->minimum();
	}

	return 0;
}

double ShortVector3Edit::GetMaximum(int axis) const
{
	if (auto spinBox = SpinBoxByAxis(axis); spinBox)
	{
		return spinBox->maximum();
	}

	return 0;
}

void ShortVector3Edit::SetRange(int axis, double min, double max)
{
	if (auto spinBox = SpinBoxByAxis(axis); spinBox)
	{
		spinBox->setRange(min, max);
	}
}

void ShortVector3Edit::SetRange(double min, double max)
{
	_ui.XEdit->setRange(min, max);
	_ui.YEdit->setRange(min, max);
	_ui.ZEdit->setRange(min, max);
}

int ShortVector3Edit::GetDecimals() const
{
	return _ui.XEdit->decimals();
}

void ShortVector3Edit::SetDecimals(int prec)
{
	_ui.XEdit->setDecimals(prec);
	_ui.YEdit->setDecimals(prec);
	_ui.ZEdit->setDecimals(prec);
}

QBoxLayout::Direction ShortVector3Edit::GetDirection() const
{
	return _ui.MainLayout->direction();
}

void ShortVector3Edit::SetDirection(QBoxLayout::Direction direction)
{
	_ui.MainLayout->setDirection(direction);
}

ShortDoubleSpinBox* ShortVector3Edit::SpinBoxByAxis(int axis) const
{
	if (axis < 0 || axis >= glm::vec3::length())
	{
		qDebug() << "Axis out of range";
		return nullptr;
	}

	ShortDoubleSpinBox* const spinBoxes[] =
	{
		_ui.XEdit,
		_ui.YEdit,
		_ui.ZEdit
	};

	return spinBoxes[axis];
}

void ShortVector3Edit::SetValue(const glm::vec3& value)
{
	_ui.XEdit->setValue(value.x);
	_ui.YEdit->setValue(value.y);
	_ui.ZEdit->setValue(value.z);
}

void ShortVector3Edit::OnValueChanged()
{
	const glm::vec3 value{GetValue()};
	emit ValueChanged(value);
}
}
