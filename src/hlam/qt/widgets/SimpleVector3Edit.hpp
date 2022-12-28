#pragma once

#include <QLineEdit>

#include <glm/vec3.hpp>

class QRegularExpressionValidator;

namespace qt::widgets
{
class SimpleVector3Edit : public QLineEdit
{
	Q_OBJECT

public:
	SimpleVector3Edit(QWidget* parent = nullptr);
	~SimpleVector3Edit();

	glm::vec3 GetValue() const;

signals:
	void ValueChanged(const glm::vec3& value);

public slots:
	void SetValue(const glm::vec3& value);

private slots:
	void OnValueChanged();

private:
	QRegularExpressionValidator* const _validator;

	glm::vec3 _value{0};
};
}
