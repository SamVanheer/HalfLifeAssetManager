#pragma once

#include <algorithm>
#include <cassert>
#include <memory>
#include <utility>
#include <vector>

#include <QDebug>
#include <QObject>
#include <QSettings>

#include "ui/settings/GameConfiguration.hpp"
#include "ui/settings/GameEnvironment.hpp"

class GameConfigurationsSettings final : public QObject
{
	Q_OBJECT

public:
	GameConfigurationsSettings(QObject* parent = nullptr)
		: QObject(parent)
	{
	}

	~GameConfigurationsSettings() = default;

	void LoadSettings(QSettings& settings)
	{
		RemoveAllGameEnvironments();

		settings.beginGroup("game_environments");

		const int environmentCount = settings.beginReadArray("List");

		for (int i = 0; i < environmentCount; ++i)
		{
			settings.setArrayIndex(i);

			const auto environmentId{settings.value("Id").toUuid()};
			auto environmentName{settings.value("Name").toString()};
			auto environmentInstallationPath{settings.value("InstallationPath").toString()};
			const auto environmentDefaultMod{settings.value("DefaultMod").toUuid()};

			if (!environmentId.isNull())
			{
				auto environment{std::make_unique<GameEnvironment>(environmentId, std::move(environmentName))};

				environment->SetInstallationPath(std::move(environmentInstallationPath));

				const int configurationCount = settings.beginReadArray("Configurations");

				for (int c = 0; c < configurationCount; ++c)
				{
					settings.setArrayIndex(c);

					const auto configurationId{settings.value("Id").toUuid()};
					auto configurationDirectory{settings.value("Directory").toString()};
					auto configurationName{settings.value("Name").toString()};

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

				settings.endArray();

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

		settings.endArray();

		{
			settings.beginGroup("ActiveConfiguration");

			const auto environmentId{settings.value("EnvironmentId").toUuid()};
			const auto configurationId{settings.value("ConfigurationId").toUuid()};

			auto activeEnvironment{GetGameEnvironmentById(environmentId)};

			GameConfiguration* activeConfiguration{nullptr};

			if (activeEnvironment)
			{
				activeConfiguration = activeEnvironment->GetGameConfigurationById(configurationId);
			}

			SetActiveConfiguration({activeEnvironment, activeConfiguration});

			settings.endGroup();
		}

		settings.endGroup();
	}

	void SaveSettings(QSettings& settings)
	{
		settings.beginGroup("game_environments");

		settings.remove("List");

		settings.beginWriteArray("List", _gameEnvironments.size());

		for (int i = 0; i < _gameEnvironments.size(); ++i)
		{
			const auto environment = _gameEnvironments[i].get();

			const auto configurations = environment->GetGameConfigurations();

			settings.setArrayIndex(i);

			settings.setValue("Id", environment->GetId());
			settings.setValue("Name", environment->GetName());
			settings.setValue("InstallationPath", environment->GetInstallationPath());
			settings.setValue("DefaultMod", environment->GetDefaultModId());

			settings.remove("Configurations");

			settings.beginWriteArray("Configurations", configurations.size());

			for (int c = 0; c < configurations.size(); ++c)
			{
				const auto configuration = configurations[c];

				settings.setArrayIndex(c);

				settings.setValue("Id", configuration->GetId());
				settings.setValue("Directory", configuration->GetDirectory());
				settings.setValue("Name", configuration->GetName());
			}

			settings.endArray();
		}

		settings.endArray();

		{
			const auto activeConfiguration = GetActiveConfiguration();

			settings.beginGroup("ActiveConfiguration");

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

			settings.setValue("EnvironmentId", environmentId);
			settings.setValue("ConfigurationId", configurationId);

			settings.endGroup();
		}

		settings.endGroup();
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
	std::vector<std::unique_ptr<GameEnvironment>> _gameEnvironments;

	std::pair<GameEnvironment*, GameConfiguration*> _activeConfiguration{};
};
