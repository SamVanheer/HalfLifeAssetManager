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
		const auto formatFloat = [](double input)
		{
			if (input == static_cast<int>(input))
			{
				return QString::number(static_cast<int>(input));
			}
			else
			{
				auto text = QString::number(input, 'f', 6);

				int size = text.size();

				while (size > 0 && text[size - 1] == '0')
				{
					--size;
				}

				if (size > 0 && text[size - 1] == '.')
				{
					--size;
				}

				text.resize(size);

				return text;
			}
		};

		const QString newText = QString{"%1 %2 %3"}
			.arg(formatFloat(value.x))
			.arg(formatFloat(value.y))
			.arg(formatFloat(value.z));

		// Avoid changing cursor position if we're responding to a redo command.
		if (text() != newText)
		{
			const QSignalBlocker blocker{this};
			this->setText(newText);
		}
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
