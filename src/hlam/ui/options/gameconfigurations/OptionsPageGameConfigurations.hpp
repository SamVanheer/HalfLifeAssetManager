#pragma once

#include <memory>

#include <QString>
#include <QWidget>

#include "ui_OptionsPageGameConfigurations.h"

#include "ui/options/OptionsPage.hpp"

class EditorContext;
class GameConfiguration;
class GameConfigurationsOptions;

extern const QString OptionsPageGameConfigurationsId;

class OptionsPageGameConfigurations final : public OptionsPage
{
public:
	explicit OptionsPageGameConfigurations();
};

class OptionsPageGameConfigurationsWidget final : public OptionsWidget
{
public:
	explicit OptionsPageGameConfigurationsWidget(EditorContext* editorContext);
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

	EditorContext* const _editorContext;

	std::unique_ptr<GameConfigurationsOptions> _options;
};
