#pragma once

#include <memory>
#include <utility>
#include <vector>

#include <QString>
#include <QUuid>

#include <spdlog/logger.h>

#include "settings/BaseSettings.hpp"

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

class GameConfigurationsSettings final : public BaseSettings
{
	Q_OBJECT

public:
	explicit GameConfigurationsSettings(QSettings* settings, ApplicationSettings* applicationSettings,
		std::shared_ptr<spdlog::logger> logger)
		: BaseSettings(settings)
		, _applicationSettings(applicationSettings)
		, _logger(logger)
	{
	}

	void LoadSettings();
	void SaveSettings();

	std::vector<const GameConfiguration*> GetConfigurations() const;

	const GameConfiguration* GetConfigurationById(const QUuid& id) const;

	const GameConfiguration* FindConfigurationByModDirectory(const QString& modDirectory) const;

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

	void AddConfigurationToFileSystem(IFileSystem& fileSystem, const GameConfiguration& configuration) const;

	std::pair<const GameConfiguration*, bool> DetectGameConfiguration(const QString& assetFileName) const;

signals:
	void ConfigurationAdded(const GameConfiguration* configuration);
	void ConfigurationRemoved(const GameConfiguration* configuration);
	void ConfigurationUpdated(const GameConfiguration* configuration);

	void DefaultConfigurationChanged(const GameConfiguration* current, const GameConfiguration* previous);

private:
	ApplicationSettings* const _applicationSettings;
	const std::shared_ptr<spdlog::logger> _logger;

	std::vector<std::unique_ptr<GameConfiguration>> _gameConfigurations;

	QUuid _defaultConfiguration;
};
