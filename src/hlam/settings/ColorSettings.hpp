#pragma once

#include <algorithm>
#include <iterator>
#include <unordered_map>
#include <utility>
#include <vector>

#include <QColor>
#include <QMap>
#include <QSettings>
#include <QString>
#include <QStringList>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "settings/BaseSettings.hpp"

inline glm::vec4 ColorToVector(const QColor& color)
{
	return {color.redF(), color.greenF(), color.blueF(), color.alphaF()};
}

inline QColor VectorToColor(const glm::vec4& color)
{
	return QColor::fromRgbF(color.r, color.g, color.b, color.a);
}

inline QColor VectorToColor(const glm::vec3& color)
{
	return VectorToColor(glm::vec4{color.r, color.g, color.b, 1});
}

inline glm::vec3 RGB888ToVector(int r, int g, int b)
{
	return {r / 255.f, g / 255.f, b / 255.f};
}

inline glm::vec4 RGBA8888ToVector(int r, int g, int b, int a)
{
	return {r / 255.f, g / 255.f, b / 255.f, a / 255.f};
}

class ColorSettings final : public BaseSettings
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
	using BaseSettings::BaseSettings;

	void LoadSettings()
	{
		_settings->beginGroup("Colors");

		const int colorCount{_settings->beginReadArray("List")};

		for (int i = 0; i < colorCount; ++i)
		{
			_settings->setArrayIndex(i);

			Set(_settings->value("Name").toString(), ColorToVector(_settings->value("Color").value<QColor>()));
		}

		_settings->endArray();
		_settings->endGroup();
	}

	void SaveSettings()
	{
		_settings->beginGroup("Colors");
		_settings->remove("List");

		_settings->beginWriteArray("List", _colors.size());

		const auto keys{GetKeys()};

		for (int i = 0; i < keys.size(); ++i)
		{
			_settings->setArrayIndex(i);

			_settings->setValue("Name", keys[i]);
			_settings->setValue("Color", VectorToColor(GetColor(keys[i])));
		}

		_settings->endArray();
		_settings->endGroup();
	}

	QStringList GetKeys() const
	{
		return _colors.keys();
	}

	glm::vec4 GetDefaultColor(const QString& key) const
	{
		if (auto it = _colors.find(key); it != _colors.end())
		{
			return it->DefaultColor;
		}

		return DefaultColor;
	}

	glm::vec4 GetColor(const QString& key, const glm::vec4& defaultValue = DefaultColor) const
	{
		if (auto it = _colors.find(key); it != _colors.end())
		{
			return it->Color;
		}

		return defaultValue;
	}

	bool HasAlphaChannel(const QString& key) const
	{
		if (auto it = _colors.find(key); it != _colors.end())
		{
			return it->HasAlphaChannel;
		}

		return false;
	}

	void Add(const QString& key, const glm::vec4& defaultColor)
	{
		_colors.insert(key, ColorData(defaultColor, defaultColor, true));
	}

	void Add(const QString& key, const glm::vec3& defaultColor)
	{
		const glm::vec4 rgba{defaultColor, 1};
		_colors.insert(key, ColorData(rgba, rgba, false));
	}

	void Set(const QString& key, const glm::vec4& color)
	{
		if (auto it = _colors.find(key); it != _colors.end())
		{
			it->Color = color;
		}
	}

signals:
	void ColorsChanged();

private:
	QMap<QString, ColorData> _colors;
};
