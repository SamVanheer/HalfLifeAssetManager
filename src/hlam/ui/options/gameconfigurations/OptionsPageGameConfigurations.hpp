#pragma once

#include <memory>

#include <QString>

#include "ui_OptionsPageGameConfigurations.h"

#include "ui/options/OptionsPage.hpp"

class AssetManager;
class GameConfiguration;
class GameConfigurationsOptions;

class OptionsPageGameConfigurations final : public OptionsPage
{
public:
	explicit OptionsPageGameConfigurations();
};

class OptionsPageGameConfigurationsWidget final : public OptionsWidget
{
public:
	explicit OptionsPageGameConfigurationsWidget(AssetManager* application);
	~OptionsPageGameConfigurationsWidget();

	void ApplyChanges() override;

private:
	void AddGameConfiguration(std::unique_ptr<GameConfiguration>&& configuration, bool markAsNew);

private slots:
	void OnEditGameConfigurations();

	void OnConfigurationChanged(int index);

	void OnGameExecutableChanged(const QString& text);
	void OnBrowseGameExecutable();

	void OnBaseGameDirectoryChanged(const QString& text);
	void OnBrowseBaseGameDirectory();

	void OnModDirectoryChanged(const QString& text);
	void OnBrowseModDirectory();

	void OnAutodetect();

private:
	Ui_OptionsPageGameConfigurations _ui;

	AssetManager* const _application;

	std::unique_ptr<GameConfigurationsOptions> _options;
};
