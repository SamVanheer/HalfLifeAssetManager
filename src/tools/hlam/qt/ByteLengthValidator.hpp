#pragma once

#include <QValidator>

namespace qt
{
/**
*	@brief Validates that a QString does not exceed a given length in bytes
*	If it exceeds the length it will be trimmed to the nearest whole character (i.e. multibyte characters are not chopped in half)
*/
class ByteLengthValidator : public QValidator
{
public:
	ByteLengthValidator(int maxByteLength, QObject* parent = nullptr)
		: QValidator(parent)
		, _maxByteLength(maxByteLength)
	{
	}

	State validate(QString& text, int& pos) const override
	{
		QByteArray output = text.toUtf8();

		if (output.size() > _maxByteLength)
		{
			return State::Invalid;
		}

		return State::Acceptable;
	}

	void fixup(QString& text) const override
	{
		QByteArray output = text.toUtf8();

		if (output.size() > _maxByteLength)
		{
			int truncateAt = 0;
			for (int i = _maxByteLength; i > 0; --i)
			{
				if ((output[i] & 0xC0) != 0x80)
				{
					truncateAt = i;
					break;
				}
			}

			output.truncate(truncateAt);
			text = output;
		}
	}

private:
	const int _maxByteLength;
};
}
