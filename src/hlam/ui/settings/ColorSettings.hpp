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

#include "qt/HashFunctions.hpp"

namespace ui::settings
{
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

			Set(settings.value("Name").toString(), settings.value("Color").value<QColor>());
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
			settings.setValue("Color", GetColor(keys[i]));
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

	QColor GetDefaultColor(const QString& key) const
	{
		if (auto it = _colors.find(key); it != _colors.end())
		{
			return it->second.first;
		}

		return Qt::GlobalColor::black;
	}

	QColor GetColor(const QString& key, const QColor& defaultValue = Qt::GlobalColor::black) const
	{
		if (auto it = _colors.find(key); it != _colors.end())
		{
			return it->second.second;
		}

		return defaultValue;
	}

	void Add(const QString& key, const QColor& defaultColor)
	{
		_colors.emplace(key, std::make_pair(defaultColor, defaultColor));
	}

	void Set(const QString& key, const QColor& color)
	{
		if (auto it = _colors.find(key); it != _colors.end())
		{
			it->second.second = color;
		}
	}

signals:
	void ColorsChanged();

private:
	std::unordered_map<QString, std::pair<QColor, QColor>> _colors;
};
}
