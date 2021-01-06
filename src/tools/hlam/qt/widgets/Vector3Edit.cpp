#include <QDebug>

#include "qt/widgets/Vector3Edit.hpp"

namespace qt::widgets
{
Vector3Edit::Vector3Edit(QWidget* parent)
	: QWidget(parent)
{
	_ui.setupUi(this);

	connect(_ui.XEdit, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Vector3Edit::OnValueChanged);
	connect(_ui.YEdit, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Vector3Edit::OnValueChanged);
	connect(_ui.ZEdit, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Vector3Edit::OnValueChanged);
}

Vector3Edit::~Vector3Edit() = default;

glm::vec3 Vector3Edit::GetValue() const
{
	return
	{
		_ui.XEdit->value(),
		_ui.YEdit->value(),
		_ui.ZEdit->value()
	};
}

double Vector3Edit::GetMinimum(int axis) const
{
	if (auto spinBox = SpinBoxByAxis(axis); spinBox)
	{
		return spinBox->minimum();
	}

	return 0;
}

double Vector3Edit::GetMaximum(int axis) const
{
	if (auto spinBox = SpinBoxByAxis(axis); spinBox)
	{
		return spinBox->maximum();
	}

	return 0;
}

void Vector3Edit::SetRange(int axis, double min, double max)
{
	if (auto spinBox = SpinBoxByAxis(axis); spinBox)
	{
		spinBox->setRange(min, max);
	}
}

void Vector3Edit::SetRange(double min, double max)
{
	_ui.XEdit->setRange(min, max);
	_ui.YEdit->setRange(min, max);
	_ui.ZEdit->setRange(min, max);
}

int Vector3Edit::GetDecimals() const
{
	return _ui.XEdit->decimals();
}

void Vector3Edit::SetDecimals(int prec)
{
	_ui.XEdit->setDecimals(prec);
	_ui.YEdit->setDecimals(prec);
	_ui.ZEdit->setDecimals(prec);
}

QDoubleSpinBox* Vector3Edit::SpinBoxByAxis(int axis) const
{
	if (axis < 0 || axis >= glm::vec3::length())
	{
		qDebug() << "Axis out of range";
		return nullptr;
	}

	QDoubleSpinBox* const spinBoxes[] =
	{
		_ui.XEdit,
		_ui.YEdit,
		_ui.ZEdit
	};

	return spinBoxes[axis];
}

void Vector3Edit::SetValue(const glm::vec3& value)
{
	_ui.XEdit->setValue(value.x);
	_ui.YEdit->setValue(value.y);
	_ui.ZEdit->setValue(value.z);
}

void Vector3Edit::OnValueChanged()
{
	const glm::vec3 value{GetValue()};
	emit ValueChanged(value);
}
}
