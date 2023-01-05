#pragma once

#include <memory>
#include <unordered_set>

#include <QStandardItemModel>
#include <QUuid>

#include "qt/HashFunctions.hpp"

#include "settings/GameConfigurationsSettings.hpp"

class GameConfigurationsOptions final
{
public:
	struct ChangeSet
	{
		std::unordered_set<QUuid> NewObjects;
		std::unordered_set<QUuid> RemovedObjects;
		std::unordered_set<QUuid> UpdatedObjects;

		void Clear()
		{
			NewObjects.clear();
			RemovedObjects.clear();
			UpdatedObjects.clear();
		}

		void MarkNew(const QUuid& id)
		{
			NewObjects.emplace(id);
		}

		void MarkChanged(const QUuid& id)
		{
			if (!NewObjects.count(id))
			{
				UpdatedObjects.emplace(id);
			}
		}

		void MarkRemoved(const QUuid& id)
		{
			if (auto it = NewObjects.find(id); it != NewObjects.end())
			{
				NewObjects.erase(it);
			}
			else if (auto it = UpdatedObjects.find(id); it != UpdatedObjects.end())
			{
				UpdatedObjects.erase(it);
			}

			RemovedObjects.emplace(id);
		}
	};

	explicit GameConfigurationsOptions() = default;

	std::vector<std::unique_ptr<GameConfiguration>> GameConfigurations;

	ChangeSet GameConfigurationsChangeSet;

	const std::unique_ptr<QStandardItemModel> GameConfigurationsModel = std::make_unique<QStandardItemModel>();

	void AddGameConfiguration(std::unique_ptr<GameConfiguration>&& configuration, bool markAsNew)
	{
		if (markAsNew)
		{
			GameConfigurationsChangeSet.MarkNew(configuration->Id);
		}

		auto item = new QStandardItem(configuration->Name);
		GameConfigurations.emplace_back(std::move(configuration));
		GameConfigurationsModel->insertRow(GameConfigurationsModel->rowCount(), item);
	}
};
