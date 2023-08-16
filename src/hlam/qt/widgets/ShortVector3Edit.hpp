#pragma once

#include <QBoxLayout>
#include <QWidget>

#include <glm/vec3.hpp>

#include "ui_ShortVector3Edit.h"

namespace qt::widgets
{
class ShortDoubleSpinBox;

class ShortVector3Edit : public QWidget
{
	Q_OBJECT

public:
	ShortVector3Edit(QWidget* parent = nullptr);
	~ShortVector3Edit();

	glm::vec3 GetValue() const;

	double GetMinimum(int axis) const;
	double GetMaximum(int axis) const;

	void SetRange(int axis, double min, double max);

	void SetRange(double min, double max);

	int GetDecimals() const;

	void SetDecimals(int prec);

	QBoxLayout::Direction GetDirection() const;

	void SetDirection(QBoxLayout::Direction direction);

private:
	[[nodiscard]] ShortDoubleSpinBox* SpinBoxByAxis(int axis) const;

signals:
	void ValueChanged(const glm::vec3& value);

public slots:
	void SetValue(const glm::vec3& value);

private slots:
	void OnValueChanged();

private:
	Ui_ShortVector3Edit _ui{};
};
}
