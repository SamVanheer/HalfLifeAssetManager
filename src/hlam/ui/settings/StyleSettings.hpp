#pragma once

#include <QObject>
#include <QSettings>

class StyleSettings final : public QObject
{
	Q_OBJECT

public:
	StyleSettings() = default;

	void LoadSettings(QSettings& settings)
	{
		settings.beginGroup("style");
		const QString stylePath = settings.value("CurrentStyle").toString();
		settings.endGroup();

		SetStylePath(stylePath);
	}

	void SaveSettings(QSettings& settings)
	{
		settings.beginGroup("style");
		settings.setValue("CurrentStyle", _stylePath);
		settings.endGroup();
	}

	QString GetStylePath() const { return _stylePath; }

	void SetStylePath(const QString& stylePath)
	{
		if (_stylePath != stylePath)
		{
			_stylePath = stylePath;
			emit StylePathChanged(_stylePath);
		}
	}

signals:
	void StylePathChanged(const QString& stylePath);

private:
	QString _stylePath;
};
