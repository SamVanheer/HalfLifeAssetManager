#pragma once

#include <algorithm>
#include <iterator>
#include <unordered_map>
#include <utility>
#include <vector>

#include <QColor>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QStringList>

#include <glm/vec3.hpp>

#include "qt/HashFunctions.hpp"

inline glm::vec3 ColorToVector(const QColor& color)
{
	return {color.redF(), color.greenF(), color.blueF()};
}

inline QColor VectorToColor(const glm::vec3& color)
{
	return QColor::fromRgbF(color.r, color.g, color.b);
}

inline glm::vec3 RGB888ToVector(int r, int g, int b)
{
	return {r / 255.f, g / 255.f, b / 255.f};
}

class ColorSettings final : public QObject
{
	Q_OBJECT

public:
	ColorSettings(QObject* parent = nullptr)
		: QObject(parent)
	{
	}

	~ColorSettings() = default;

	void LoadSettings(QSettings& settings)
	{
		settings.beginGroup("colors");

		const int colorCount{settings.beginReadArray("List")};

		for (int i = 0; i < colorCount; ++i)
		{
			settings.setArrayIndex(i);

			Set(settings.value("Name").toString(), ColorToVector(settings.value("Color").value<QColor>()));
		}

		settings.endArray();
		settings.endGroup();
	}

	void SaveSettings(QSettings& settings)
	{
		settings.beginGroup("colors");
		settings.remove("List");

		settings.beginWriteArray("List", _colors.size());

		const auto keys{GetKeys()};

		for (int i = 0; i < keys.size(); ++i)
		{
			settings.setArrayIndex(i);

			settings.setValue("Name", keys[i]);
			settings.setValue("Color", VectorToColor(GetColor(keys[i])));
		}

		settings.endArray();
		settings.endGroup();
	}

	QStringList GetKeys() const
	{
		QStringList keys;

		keys.reserve(_colors.size());

		std::transform(_colors.begin(), _colors.end(), std::back_inserter(keys), [](const auto& color)
			{
				return color.first;
			});

		return keys;
	}

	glm::vec3 GetDefaultColor(const QString& key) const
	{
		if (auto it = _colors.find(key); it != _colors.end())
		{
			return it->second.first;
		}

		return glm::vec3{0};
	}

	glm::vec3 GetColor(const QString& key, const glm::vec3& defaultValue = glm::vec3{0}) const
	{
		if (auto it = _colors.find(key); it != _colors.end())
		{
			return it->second.second;
		}

		return defaultValue;
	}

	void Add(const QString& key, const glm::vec3& defaultColor)
	{
		_colors.emplace(key, std::make_pair(defaultColor, defaultColor));
	}

	void Set(const QString& key, const glm::vec3& color)
	{
		if (auto it = _colors.find(key); it != _colors.end())
		{
			it->second.second = color;
		}
	}

signals:
	void ColorsChanged();

private:
	std::unordered_map<QString, std::pair<glm::vec3, glm::vec3>> _colors;
};
