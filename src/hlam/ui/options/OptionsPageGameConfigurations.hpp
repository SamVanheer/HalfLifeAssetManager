#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <QStandardItemModel>
#include <QString>
#include <QWidget>

#include "ui_OptionsPageGameConfigurations.h"

#include "qt/HashFunctions.hpp"

#include "ui/options/OptionsPage.hpp"

class EditorContext;

class GameConfigurationsSettings;
class GameEnvironment;

extern const QString OptionsPageGameConfigurationsId;

class OptionsPageGameConfigurations final : public OptionsPage
{
public:
	OptionsPageGameConfigurations(const std::shared_ptr<GameConfigurationsSettings>& gameConfigurationsSettings);

private:
	const std::shared_ptr<GameConfigurationsSettings> _gameConfigurationsSettings;
};

class OptionsPageGameConfigurationsWidget final : public OptionsWidget
{
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

public:
	OptionsPageGameConfigurationsWidget(EditorContext* editorContext);
	~OptionsPageGameConfigurationsWidget();

	void ApplyChanges(QSettings& settings) override;

private:
	ChangeSet* GetOrCreateGameConfigurationChangeSet(const QUuid& id);

	void AddGameEnvironment(std::unique_ptr<GameEnvironment>&& gameEnvironment);

private slots:
	void OnActiveGameEnvironmentChanged(int index);

	void OnGameEnvironmentNameChanged(const QModelIndex& topLeft);
	void OnGameEnvironmentSelectionChanged(const QModelIndex& current, const QModelIndex& previous);

	void OnNewGameEnvironment();
	void OnRemoveGameEnvironment();

	void OnGameInstallLocationChanged(const QString& text);
	void OnBrowseGameInstallation();

	void OnDefaultGameChanged();

	void OnGameConfigurationDataChanged(const QModelIndex& topLeft);
	void OnGameConfigurationSelectionChanged(const QModelIndex& current, const QModelIndex& previous);

	void OnNewGameConfiguration();
	void OnRemoveGameConfiguration();

private:
	Ui_OptionsPageGameConfigurations _ui;

	EditorContext* const _editorContext;

	std::vector<std::unique_ptr<GameEnvironment>> _gameEnvironments;

	ChangeSet _gameEnvironmentsChangeSet;

	std::unordered_map<QUuid, std::unique_ptr<ChangeSet>> _gameConfigurationsChangeSet;

	QStandardItemModel* _gameEnvironmentsModel;
	QStandardItemModel* _gameConfigurationsModel;

	bool _currentEnvironmentIsActive = false;
};
