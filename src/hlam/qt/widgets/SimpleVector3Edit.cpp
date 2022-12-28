#include <cstring>

#include <QLocale>
#include <QRegularExpressionValidator>
#include <QSignalBlocker>

#include "qt/widgets/SimpleVector3Edit.hpp"

namespace qt::widgets
{
static const QRegularExpression Vector3Regex(R"(^-?\d+(\.\d+)? -?\d+(\.\d+)? -?\d+(\.\d+)?$)");

SimpleVector3Edit::SimpleVector3Edit(QWidget* parent)
	: QLineEdit(parent)
	, _validator(new QRegularExpressionValidator(Vector3Regex, this))
{
	this->setValidator(_validator);

	connect(this, &QLineEdit::textChanged, this, &SimpleVector3Edit::OnValueChanged);

	SetValue(glm::vec3{0});
}

SimpleVector3Edit::~SimpleVector3Edit() = default;

glm::vec3 SimpleVector3Edit::GetValue() const
{
	return _value;
}

void SimpleVector3Edit::SetValue(const glm::vec3& value)
{
	{
		const QSignalBlocker blocker{this};
		this->setText(QString{"%1 %2 %3"}
			.arg(value.x, 0, 'f', QLocale::FloatingPointShortest)
			.arg(value.y, 0, 'f', QLocale::FloatingPointShortest)
			.arg(value.z, 0, 'f', QLocale::FloatingPointShortest));
	}

	OnValueChanged();
}

void SimpleVector3Edit::OnValueChanged()
{
	// Only notify listeners if the vector value is valid.
	if (!hasAcceptableInput())
	{
		return;
	}

	std::string text{this->text().toStdString()};

	glm::vec3 value{0};

	char* start = text.data();

	for (int i = 0; i < 3; ++i)
	{
		value[i] = std::strtod(start, &start);
	}

	_value = value;

	emit ValueChanged(value);
}
}
