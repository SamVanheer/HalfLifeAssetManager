#pragma once

#include <algorithm>
#include <cassert>
#include <memory>
#include <utility>
#include <vector>

#include <QDebug>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QUuid>

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
	explicit GameConfigurationsSettings(QSettings* settings)
		: _settings(settings)
	{
	}

	~GameConfigurationsSettings() = default;

	void LoadSettings()
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

	void SaveSettings()
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

	std::vector<const GameConfiguration*> GetConfigurations() const
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

	const GameConfiguration* GetConfigurationById(const QUuid& id) const
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

	void AddConfiguration(std::unique_ptr<GameConfiguration>&& configuration)
	{
		auto& ref = _gameConfigurations.emplace_back(std::move(configuration));

		emit ConfigurationAdded(ref.get());
	}

	void RemoveConfiguration(const QUuid& id)
	{
		if (auto it = std::find_if(_gameConfigurations.begin(), _gameConfigurations.end(),
			[&](const auto& configuration)
			{
				return configuration->Id == id;
			}
		); it != _gameConfigurations.end())
		{
			const std::unique_ptr<GameConfiguration> gameConfiguration{std::move(*it)};

			if (_defaultConfiguration == id)
			{
				SetDefaultConfiguration({});
			}

			_gameConfigurations.erase(it);

			emit ConfigurationRemoved(gameConfiguration.get());
		}
	}

	void UpdateConfiguration(const GameConfiguration& configuration)
	{
		auto target = const_cast<GameConfiguration*>(GetConfigurationById(configuration.Id));

		if (!target)
		{
			qDebug() << "Invalid configuration id passed to UpdateConfiguration";
		}

		*target = configuration;

		emit ConfigurationUpdated(target);
	}

	void RemoveAll()
	{
		SetDefaultConfiguration(QUuid{});

		for (auto it = _gameConfigurations.begin(); it != _gameConfigurations.end();)
		{
			const std::unique_ptr<GameConfiguration> gameConfiguration{std::move(*it)};
			it = _gameConfigurations.erase(it);
			emit ConfigurationRemoved(gameConfiguration.get());
		}
	}

	const GameConfiguration* GetDefaultConfiguration() const { return GetConfigurationById(_defaultConfiguration); }

	void SetDefaultConfiguration(const QUuid& id)
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

signals:
	void ConfigurationAdded(const GameConfiguration* configuration);
	void ConfigurationRemoved(const GameConfiguration* configuration);
	void ConfigurationUpdated(const GameConfiguration* configuration);

	void DefaultConfigurationChanged(const GameConfiguration* current, const GameConfiguration* previous);

private:
	QSettings* const _settings;

	std::vector<std::unique_ptr<GameConfiguration>> _gameConfigurations;

	QUuid _defaultConfiguration;
};
