#pragma once

#include <QObject>

class QSettings;

class BaseSettings : public QObject
{
public:
	explicit BaseSettings(QSettings* settings)
		: _settings(settings)
	{
	}

	~BaseSettings() = default;

	virtual void LoadSettings() = 0;
	virtual void SaveSettings() = 0;

protected:
	QSettings* const _settings;
};
