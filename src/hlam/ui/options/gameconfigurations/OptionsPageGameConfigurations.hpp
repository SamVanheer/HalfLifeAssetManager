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

class ApplicationSettings;
class EditorContext;
class GameConfiguration;
class GameConfigurationsSettings;

extern const QString OptionsPageGameConfigurationsId;

class OptionsPageGameConfigurations final : public OptionsPage
{
public:
	explicit OptionsPageGameConfigurations();
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
	explicit OptionsPageGameConfigurationsWidget(EditorContext* editorContext);
	~OptionsPageGameConfigurationsWidget();

	void ApplyChanges() override;

private:
	void AddGameConfiguration(std::unique_ptr<GameConfiguration>&& configuration);

private slots:
	void OnEditGameConfigurations();

	void OnConfigurationChanged(int index);

	void OnGameExecutableChanged(const QString& text);
	void OnBrowseGameExecutable();

	void OnBaseGameDirectoryChanged(const QString& text);
	void OnBrowseBaseGameDirectory();

	void OnModDirectoryChanged(const QString& text);
	void OnBrowseModDirectory();

private:
	Ui_OptionsPageGameConfigurations _ui;

	EditorContext* const _editorContext;

	std::vector<std::unique_ptr<GameConfiguration>> _gameConfigurations;

	ChangeSet _gameConfigurationsChangeSet;

	QStandardItemModel* _gameConfigurationsModel;

	bool _currentEnvironmentIsActive = false;
};
