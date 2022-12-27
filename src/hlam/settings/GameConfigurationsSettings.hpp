#pragma once

#include <algorithm>
#include <cassert>
#include <memory>
#include <utility>
#include <vector>

#include <QDebug>
#include <QObject>
#include <QSettings>

#include "settings/GameConfiguration.hpp"
#include "settings/GameEnvironment.hpp"

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
		RemoveAllGameEnvironments();

		_settings->beginGroup("GameEnvironments");

		const int environmentCount = _settings->beginReadArray("List");

		for (int i = 0; i < environmentCount; ++i)
		{
			_settings->setArrayIndex(i);

			const auto environmentId{_settings->value("Id").toUuid()};
			auto environmentName{_settings->value("Name").toString()};
			auto environmentInstallationPath{_settings->value("InstallationPath").toString()};
			const auto environmentDefaultMod{_settings->value("DefaultMod").toUuid()};

			if (!environmentId.isNull())
			{
				auto environment{std::make_unique<GameEnvironment>(environmentId, std::move(environmentName))};

				environment->SetInstallationPath(std::move(environmentInstallationPath));

				const int configurationCount = _settings->beginReadArray("Configurations");

				for (int c = 0; c < configurationCount; ++c)
				{
					_settings->setArrayIndex(c);

					const auto configurationId{_settings->value("Id").toUuid()};
					auto configurationDirectory{_settings->value("Directory").toString()};
					auto configurationName{_settings->value("Name").toString()};

					if (!configurationId.isNull())
					{
						auto configuration{std::make_unique<GameConfiguration>(configurationId, std::move(configurationDirectory), std::move(configurationName))};

						environment->AddGameConfiguration(std::move(configuration));
					}
					else
					{
						qDebug() << "Game configuration \"" << c << "\" in environment \"" << environment->GetName() << "\" has invalid settings data";
					}
				}

				_settings->endArray();

				if (!environmentDefaultMod.isNull() && environment->GetGameConfigurationById(environmentDefaultMod))
				{
					environment->SetDefaultModId(environmentDefaultMod);
				}

				AddGameEnvironment(std::move(environment));
			}
			else
			{
				qDebug() << "Game environment \"" << i << "\" has invalid settings data";
			}
		}

		_settings->endArray();

		{
			_settings->beginGroup("ActiveConfiguration");

			const auto environmentId{_settings->value("EnvironmentId").toUuid()};
			const auto configurationId{_settings->value("ConfigurationId").toUuid()};

			auto activeEnvironment{GetGameEnvironmentById(environmentId)};

			GameConfiguration* activeConfiguration{nullptr};

			if (activeEnvironment)
			{
				activeConfiguration = activeEnvironment->GetGameConfigurationById(configurationId);
			}

			SetActiveConfiguration({activeEnvironment, activeConfiguration});

			_settings->endGroup();
		}

		_settings->endGroup();
	}

	void SaveSettings()
	{
		_settings->beginGroup("GameEnvironments");

		_settings->remove("List");

		_settings->beginWriteArray("List", _gameEnvironments.size());

		for (int i = 0; i < _gameEnvironments.size(); ++i)
		{
			const auto environment = _gameEnvironments[i].get();

			const auto configurations = environment->GetGameConfigurations();

			_settings->setArrayIndex(i);

			_settings->setValue("Id", environment->GetId());
			_settings->setValue("Name", environment->GetName());
			_settings->setValue("InstallationPath", environment->GetInstallationPath());
			_settings->setValue("DefaultMod", environment->GetDefaultModId());

			_settings->remove("Configurations");

			_settings->beginWriteArray("Configurations", configurations.size());

			for (int c = 0; c < configurations.size(); ++c)
			{
				const auto configuration = configurations[c];

				_settings->setArrayIndex(c);

				_settings->setValue("Id", configuration->GetId());
				_settings->setValue("Directory", configuration->GetDirectory());
				_settings->setValue("Name", configuration->GetName());
			}

			_settings->endArray();
		}

		_settings->endArray();

		{
			const auto activeConfiguration = GetActiveConfiguration();

			_settings->beginGroup("ActiveConfiguration");

			QUuid environmentId{};
			QUuid configurationId{};

			if (activeConfiguration.first)
			{
				environmentId = activeConfiguration.first->GetId();
			}

			if (activeConfiguration.second)
			{
				configurationId = activeConfiguration.second->GetId();
			}

			_settings->setValue("EnvironmentId", environmentId);
			_settings->setValue("ConfigurationId", configurationId);

			_settings->endGroup();
		}

		_settings->endGroup();
	}

	std::vector<GameEnvironment*> GetGameEnvironments() const
	{
		std::vector<GameEnvironment*> environments;

		environments.reserve(_gameEnvironments.size());

		std::transform(_gameEnvironments.begin(), _gameEnvironments.end(), std::back_inserter(environments), [](const auto& environment)
			{
				return environment.get();
			});

		return environments;
	}

	GameEnvironment* GetGameEnvironmentById(const QUuid& id) const
	{
		if (auto it = std::find_if(_gameEnvironments.begin(), _gameEnvironments.end(), [&](const auto& environment)
			{
				return environment->GetId() == id;
			}
		); it != _gameEnvironments.end())
		{
			return it->get();
		}

		return nullptr;
	}

	void AddGameEnvironment(std::unique_ptr<GameEnvironment>&& gameEnvironment)
	{
		assert(gameEnvironment);

		auto& ref = _gameEnvironments.emplace_back(std::move(gameEnvironment));

		emit GameEnvironmentAdded(ref.get());
	}

	void RemoveGameEnvironment(const QUuid& id)
	{
		if (auto it = std::find_if(_gameEnvironments.begin(), _gameEnvironments.end(), [&](const auto& environment)
			{
				return environment->GetId() == id;
			}
		); it != _gameEnvironments.end())
		{
			const std::unique_ptr<GameEnvironment> gameEnvironment{std::move(*it)};

			if (_activeConfiguration.first == gameEnvironment.get())
			{
				SetActiveConfiguration({});
			}

			_gameEnvironments.erase(it);

			emit GameEnvironmentRemoved(gameEnvironment.get());
		}
	}

	void RemoveAllGameEnvironments()
	{
		SetActiveConfiguration({nullptr, nullptr});

		for (auto it = _gameEnvironments.begin(); it != _gameEnvironments.end();)
		{
			const std::unique_ptr<GameEnvironment> gameEnvironment{std::move(*it)};
			it = _gameEnvironments.erase(it);
			emit GameEnvironmentRemoved(gameEnvironment.get());
		}
	}

	std::pair<GameEnvironment*, GameConfiguration*> GetActiveConfiguration() const { return _activeConfiguration; }

	void SetActiveConfiguration(std::pair<GameEnvironment*, GameConfiguration*> configuration)
	{
		if (_activeConfiguration != configuration)
		{
			const auto previous = _activeConfiguration;

			if (configuration.second && !configuration.first)
			{
				qDebug() << "The active configuration requires a valid environment if the configuration is not null";
				configuration.second = nullptr;
			}

			_activeConfiguration = configuration;

			emit ActiveConfigurationChanged(configuration, previous);
		}
	}

signals:
	void GameEnvironmentAdded(GameEnvironment* gameEnvironment);
	void GameEnvironmentRemoved(GameEnvironment* gameEnvironment);

	void ActiveConfigurationChanged(std::pair<GameEnvironment*, GameConfiguration*> current,
		std::pair<GameEnvironment*, GameConfiguration*> previous);

private:
	QSettings* const _settings;

	std::vector<std::unique_ptr<GameEnvironment>> _gameEnvironments;

	std::pair<GameEnvironment*, GameConfiguration*> _activeConfiguration{};
};
