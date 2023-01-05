#include <algorithm>

#include <QDebug>
#include <QFileInfo>
#include <QSettings>

#include "filesystem/FileSystem.hpp"
#include "filesystem/FileSystemConstants.hpp"

#include "qt/QtLogging.hpp"

#include "settings/ApplicationSettings.hpp"
#include "settings/GameConfigurationsSettings.hpp"

void GameConfigurationsSettings::LoadSettings()
{
	RemoveAll();

	_settings->beginGroup("GameConfigurations");

	const int configurationCount = _settings->beginReadArray("List");

	for (int i = 0; i < configurationCount; ++i)
	{
		_settings->setArrayIndex(i);

		const auto configurationId{_settings->value("Id").toUuid()};

		if (!configurationId.isNull())
		{
			GameConfiguration configuration
			{
				.Id = configurationId,
				.Name = _settings->value("Name").toString(),
				.GameExecutable = _settings->value("GameExecutable").toString(),
				.BaseGameDirectory = _settings->value("BaseGameDirectory").toString(),
				.ModDirectory = _settings->value("ModDirectory").toString()
			};

			AddConfiguration(std::make_unique<GameConfiguration>(std::move(configuration)));
		}
		else
		{
			qDebug() << "Game configuration \"" << i << "\" has invalid settings data";
		}
	}

	_settings->endArray();

	SetDefaultConfiguration(_settings->value("DefaultConfigurationId").toUuid());

	_settings->endGroup();
}

void GameConfigurationsSettings::SaveSettings()
{
	_settings->beginGroup("GameConfigurations");

	_settings->remove("List");

	_settings->beginWriteArray("List", _gameConfigurations.size());

	for (int i = 0; i < _gameConfigurations.size(); ++i)
	{
		const auto configuration = _gameConfigurations[i].get();

		_settings->setArrayIndex(i);

		_settings->setValue("Id", configuration->Id);
		_settings->setValue("Name", configuration->Name);
		_settings->setValue("GameExecutable", configuration->GameExecutable);
		_settings->setValue("BaseGameDirectory", configuration->BaseGameDirectory);
		_settings->setValue("ModDirectory", configuration->ModDirectory);
	}

	_settings->endArray();

	_settings->setValue("DefaultConfigurationId", _defaultConfiguration);

	_settings->endGroup();
}

std::vector<const GameConfiguration*> GameConfigurationsSettings::GetConfigurations() const
{
	std::vector<const GameConfiguration*> configurations;

	configurations.reserve(_gameConfigurations.size());

	std::transform(_gameConfigurations.begin(), _gameConfigurations.end(), std::back_inserter(configurations),
		[](const auto& configuration)
		{
			return configuration.get();
		});

	return configurations;
}

const GameConfiguration* GameConfigurationsSettings::GetConfigurationById(const QUuid& id) const
{
	if (auto it = std::find_if(_gameConfigurations.begin(), _gameConfigurations.end(),
		[&](const auto& configuration)
		{
			return configuration->Id == id;
		}
	); it != _gameConfigurations.end())
	{
		return it->get();
	}

	return nullptr;
}

void GameConfigurationsSettings::AddConfiguration(std::unique_ptr<GameConfiguration>&& configuration)
{
	auto& ref = _gameConfigurations.emplace_back(std::move(configuration));

	SanitizeConfiguration(*ref);

	emit ConfigurationAdded(ref.get());
}

void GameConfigurationsSettings::RemoveConfiguration(const QUuid& id)
{
	if (auto it = std::find_if(_gameConfigurations.begin(), _gameConfigurations.end(),
		[&](const auto& configuration)
		{
			return configuration->Id == id;
		}
	); it != _gameConfigurations.end())
	{
		if (_defaultConfiguration == id)
		{
			SetDefaultConfiguration({});
		}

		const std::unique_ptr<GameConfiguration> gameConfiguration{std::move(*it)};

		_gameConfigurations.erase(it);

		emit ConfigurationRemoved(gameConfiguration.get());
	}
}

void GameConfigurationsSettings::UpdateConfiguration(const GameConfiguration& configuration)
{
	auto target = const_cast<GameConfiguration*>(GetConfigurationById(configuration.Id));

	if (!target)
	{
		qDebug() << "Invalid configuration id passed to UpdateConfiguration";
		return;
	}

	*target = configuration;

	SanitizeConfiguration(*target);

	emit ConfigurationUpdated(target);
}

void GameConfigurationsSettings::RemoveAll()
{
	SetDefaultConfiguration(QUuid{});

	for (auto it = _gameConfigurations.begin(); it != _gameConfigurations.end();)
	{
		const std::unique_ptr<GameConfiguration> gameConfiguration{std::move(*it)};
		it = _gameConfigurations.erase(it);
		emit ConfigurationRemoved(gameConfiguration.get());
	}
}

void GameConfigurationsSettings::SetDefaultConfiguration(const QUuid& id)
{
	if (_defaultConfiguration != id)
	{
		const auto current = GetConfigurationById(id);

		if (!id.isNull() && !current)
		{
			qDebug() << "Invalid configuration id passed to SetDefaultConfiguration";
			return;
		}

		const auto previous = GetConfigurationById(_defaultConfiguration);

		_defaultConfiguration = id;

		emit DefaultConfigurationChanged(current, previous);
	}
}

std::unique_ptr<IFileSystem> GameConfigurationsSettings::CreateFileSystem(const QString& assetFileName) const
{
	const auto [configuration, isDefault] = DetectGameConfiguration(assetFileName);

	auto fileSystem = std::make_unique<FileSystem>();

	const auto language = _applicationSettings->GetSteamLanguage().toStdString();
	const bool hasLanguage = language != DefaultSteamLanguage;

	const auto addConfiguration = [&](const GameConfiguration& configuration)
	{
		const auto gameDir{configuration.BaseGameDirectory.toStdString()};
		const auto modDir{configuration.ModDirectory.toStdString()};

		const auto addGameDirectory = [&](const std::string& directory)
		{
			fileSystem->AddSearchPath(directory + "_addon");

			if (hasLanguage)
			{
				fileSystem->AddSearchPath(directory + "_" + language);
			}

			fileSystem->AddSearchPath(directory + "_hd");
			fileSystem->AddSearchPath(std::string{directory});
			fileSystem->AddSearchPath(directory + "_downloads");
		};

		//Add mod dirs first since they override game dirs
		if (!modDir.empty() && gameDir != modDir)
		{
			addGameDirectory(modDir);
		}

		if (!gameDir.empty())
		{
			addGameDirectory(gameDir);
		}
	};

	if (isDefault)
	{
		const std::filesystem::path assetPath{assetFileName.toStdString()};
		const auto [isInGameAssetDirectory, baseGameDirectoryPath] = IsFileInGameAssetsDirectory(assetPath);

		if (isInGameAssetDirectory)
		{
			const auto baseGameDirectory = QString::fromStdString(baseGameDirectoryPath.string());

			addConfiguration(GameConfiguration{.BaseGameDirectory = baseGameDirectory});

			qCDebug(logging::HLAMFileSystem) << "Using auto-detected game configuration for asset"
				<< assetFileName << "based on base game directory" << baseGameDirectory;
		}
	}

	if (configuration)
	{
		addConfiguration(*configuration);
	}

	return fileSystem;
}

void GameConfigurationsSettings::SanitizeConfiguration(GameConfiguration& configuration)
{
	configuration.GameExecutable = QFileInfo{configuration.GameExecutable}.absoluteFilePath();
	configuration.BaseGameDirectory = QFileInfo{configuration.BaseGameDirectory}.absoluteFilePath();
	configuration.ModDirectory = QFileInfo{configuration.ModDirectory}.absoluteFilePath();
}

std::pair<const GameConfiguration*, bool> GameConfigurationsSettings::DetectGameConfiguration(
	const QString& assetFileName) const
{
	const QFileInfo file{assetFileName};

	const auto directory = file.absolutePath();

	// Search mod directories first in case a mod was added as both a mod and base game.
	// Mods fall back on the base game for assets so configurations with that setup will work better.
	for (const auto& configuration : _gameConfigurations)
	{
		if (!configuration->ModDirectory.isEmpty()
			&& directory.startsWith(configuration->ModDirectory, Qt::CaseInsensitive))
		{
			qCDebug(logging::HLAMFileSystem) << "Using game configuration"
				<< configuration->Name << "(" << configuration->Id << ") for asset"
				<< assetFileName << "based on mod directory" << configuration->ModDirectory;
			return {configuration.get(), false};
		}
	}

	for (const auto& configuration : _gameConfigurations)
	{
		if (!configuration->BaseGameDirectory.isEmpty()
			&& directory.startsWith(configuration->BaseGameDirectory, Qt::CaseInsensitive))
		{
			qCDebug(logging::HLAMFileSystem) << "Using game configuration"
				<< configuration->Name << "(" << configuration->Id << ") for asset"
				<< assetFileName << "based on base game directory" << configuration->BaseGameDirectory;
			return {configuration.get(), false};
		}
	}

	const auto configuration = GetDefaultConfiguration();

	if (configuration)
	{
		qCDebug(logging::HLAMFileSystem) << "Using default game configuration"
			<< configuration->Name << "(" << configuration->Id << "for asset"
			<< assetFileName;
	}
	else
	{
		qCDebug(logging::HLAMFileSystem) << "No game configuration for asset" << assetFileName;
	}

	return {configuration, true};
}
