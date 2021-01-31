#pragma once

#include <QWidget>

#include <glm/vec3.hpp>

#include "ui_Vector3Edit.h"

namespace qt::widgets
{
class Vector3Edit : public QWidget
{
	Q_OBJECT

public:
	Vector3Edit(QWidget* parent = nullptr);
	~Vector3Edit();

	glm::vec3 GetValue() const;

	double GetMinimum(int axis) const;
	double GetMaximum(int axis) const;

	void SetRange(int axis, double min, double max);

	void SetRange(double min, double max);

	int GetDecimals() const;

	void SetDecimals(int prec);

private:
	[[nodiscard]] QDoubleSpinBox* SpinBoxByAxis(int axis) const;

signals:
	void ValueChanged(const glm::vec3& value);

public slots:
	void SetValue(const glm::vec3& value);

private slots:
	void OnValueChanged();

private:
	Ui_Vector3Edit _ui{};
};
}
