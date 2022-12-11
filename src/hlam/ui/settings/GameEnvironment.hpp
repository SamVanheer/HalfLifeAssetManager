#pragma once

#include <algorithm>
#include <cassert>
#include <iterator>
#include <vector>

#include <QObject>
#include <QString>
#include <QUuid>
#include <QVariant>

#include "ui/settings/GameConfiguration.hpp"

namespace ui::settings
{
/**
*	@brief Defines a game installation along with a set of game configurations
*/
class GameEnvironment final : public QObject
{
	Q_OBJECT

public:
	GameEnvironment() = delete;

	GameEnvironment(const QUuid& id, QString&& name)
		: QObject()
		, _id(id)
		, _name(std::move(name))
	{
	}

	GameEnvironment(const GameEnvironment& other)
		: _id(other._id)
		, _name(other._name)
		, _installationPath(other._installationPath)
		, _defaultMod(other._defaultMod)
	{
		_gameConfigurations.reserve(other._gameConfigurations.size());

		std::transform(other._gameConfigurations.begin(), other._gameConfigurations.end(), std::back_inserter(_gameConfigurations), [](const auto& configuration)
			{
				return std::make_unique<GameConfiguration>(*configuration);
			});
	}

	GameEnvironment& operator=(const GameEnvironment&) = delete;

	GameEnvironment(GameEnvironment&&) = delete;
	GameEnvironment& operator=(GameEnvironment&&) = delete;

	~GameEnvironment() = default;

	const QUuid& GetId() const { return _id; }

	QString GetName() const { return _name; }

	void SetName(QString&& value)
	{
		_name = std::move(value);
	}

	QString GetInstallationPath() const { return _installationPath; }

	void SetInstallationPath(QString&& value)
	{
		_installationPath = std::move(value);
	}

	const QUuid& GetDefaultModId() const { return _defaultMod; }

	void SetDefaultModId(const QUuid& value)
	{
		_defaultMod = value;
	}

	std::vector<GameConfiguration*> GetGameConfigurations() const
	{
		std::vector<GameConfiguration*> configurations;

		configurations.reserve(_gameConfigurations.size());

		for (auto it = _gameConfigurations.begin(); it != _gameConfigurations.end(); ++it)
		{
			configurations.emplace_back(it->get());
		}

		return configurations;
	}

	GameConfiguration* GetGameConfigurationById(const QUuid& id) const
	{
		if (auto it = std::find_if(_gameConfigurations.begin(), _gameConfigurations.end(), [&](const auto& config)
			{
				return config->GetId() == id;
			}
		); it != _gameConfigurations.end())
		{
			return it->get();
		}

		return nullptr;
	}

	void AddGameConfiguration(std::unique_ptr<GameConfiguration>&& configuration)
	{
		assert(configuration);

		auto& ref = _gameConfigurations.emplace_back(std::move(configuration));

		emit GameConfigurationAdded(ref.get());
	}

	void RemoveGameConfiguration(const QUuid& id)
	{
		if (auto it = std::find_if(_gameConfigurations.begin(), _gameConfigurations.end(), [&](const auto& config)
			{
				return config->GetId() == id;
			}
		); it != _gameConfigurations.end())
		{
			const std::unique_ptr<GameConfiguration> gameConfiguration{std::move(*it)};

			_gameConfigurations.erase(it);

			emit GameConfigurationRemoved(gameConfiguration.get());
		}
	}

signals:
	void GameConfigurationAdded(GameConfiguration* gameConfiguration);
	void GameConfigurationRemoved(GameConfiguration* gameConfiguration);

private:
	const QUuid _id;

	QString _name;

	QString _installationPath;
	QUuid _defaultMod;

	std::vector<std::unique_ptr<GameConfiguration>> _gameConfigurations;
};
}

Q_DECLARE_METATYPE(ui::settings::GameEnvironment*)
