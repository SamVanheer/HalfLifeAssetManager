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
#include <glm/vec4.hpp>

#include "qt/HashFunctions.hpp"

inline glm::vec4 ColorToVector(const QColor& color)
{
	return {color.redF(), color.greenF(), color.blueF(), color.alphaF()};
}

inline QColor VectorToColor(const glm::vec4& color)
{
	return QColor::fromRgbF(color.r, color.g, color.b, color.a);
}

inline glm::vec3 RGB888ToVector(int r, int g, int b)
{
	return {r / 255.f, g / 255.f, b / 255.f};
}

inline glm::vec4 RGBA8888ToVector(int r, int g, int b, int a)
{
	return {r / 255.f, g / 255.f, b / 255.f, a / 255.f};
}

class ColorSettings final : public QObject
{
	Q_OBJECT

private:
	struct ColorData
	{
		glm::vec4 DefaultColor;
		glm::vec4 Color;
		bool HasAlphaChannel;
	};

	static constexpr glm::vec4 DefaultColor{0, 0, 0, 1};

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

	glm::vec4 GetDefaultColor(const QString& key) const
	{
		if (auto it = _colors.find(key); it != _colors.end())
		{
			return it->second.DefaultColor;
		}

		return DefaultColor;
	}

	glm::vec4 GetColor(const QString& key, const glm::vec4& defaultValue = DefaultColor) const
	{
		if (auto it = _colors.find(key); it != _colors.end())
		{
			return it->second.Color;
		}

		return defaultValue;
	}

	bool HasAlphaChannel(const QString& key) const
	{
		if (auto it = _colors.find(key); it != _colors.end())
		{
			return it->second.HasAlphaChannel;
		}

		return false;
	}

	void Add(const QString& key, const glm::vec4& defaultColor)
	{
		_colors.emplace(key, ColorData(defaultColor, defaultColor, true));
	}

	void Add(const QString& key, const glm::vec3& defaultColor)
	{
		const glm::vec4 rgba{defaultColor, 1};
		_colors.emplace(key, ColorData(rgba, rgba, false));
	}

	void Set(const QString& key, const glm::vec4& color)
	{
		if (auto it = _colors.find(key); it != _colors.end())
		{
			it->second.Color = color;
		}
	}

signals:
	void ColorsChanged();

private:
	std::unordered_map<QString, ColorData> _colors;
};
