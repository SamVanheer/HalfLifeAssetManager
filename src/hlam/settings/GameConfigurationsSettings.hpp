#pragma once

#include <memory>
#include <utility>
#include <vector>

#include <QObject>
#include <QString>
#include <QUuid>

class ApplicationSettings;
class IFileSystem;
class QSettings;

class GameConfiguration final
{
public:
	QUuid Id;
	QString Name;
	QString GameExecutable;
	QString BaseGameDirectory;
	QString ModDirectory;
};

class GameConfigurationsSettings final : public QObject
{
	Q_OBJECT

public:
	explicit GameConfigurationsSettings(ApplicationSettings* applicationSettings, QSettings* settings)
		: _applicationSettings(applicationSettings)
		, _settings(settings)
	{
	}

	~GameConfigurationsSettings() = default;

	void LoadSettings();
	void SaveSettings();

	std::vector<const GameConfiguration*> GetConfigurations() const;

	const GameConfiguration* GetConfigurationById(const QUuid& id) const;

	void AddConfiguration(std::unique_ptr<GameConfiguration>&& configuration);

	void RemoveConfiguration(const QUuid& id);

	void UpdateConfiguration(const GameConfiguration& configuration);

	void RemoveAll();

	const GameConfiguration* GetDefaultConfiguration() const { return GetConfigurationById(_defaultConfiguration); }

	void SetDefaultConfiguration(const QUuid& id);

	/**
	*	@brief Creates a filesystem for an asset with the given filename.
	*	If the file is located in a game directory with a known configuration it will be used,
	*	otherwise the default configuration is used.
	*/
	std::unique_ptr<IFileSystem> CreateFileSystem(const QString& assetFileName) const;

private:
	static void SanitizeConfiguration(GameConfiguration& configuration);

	std::pair<const GameConfiguration*, bool> DetectGameConfiguration(const QString& assetFileName) const;

signals:
	void ConfigurationAdded(const GameConfiguration* configuration);
	void ConfigurationRemoved(const GameConfiguration* configuration);
	void ConfigurationUpdated(const GameConfiguration* configuration);

	void DefaultConfigurationChanged(const GameConfiguration* current, const GameConfiguration* previous);

private:
	ApplicationSettings* const _applicationSettings;
	QSettings* const _settings;

	std::vector<std::unique_ptr<GameConfiguration>> _gameConfigurations;

	QUuid _defaultConfiguration;
};
