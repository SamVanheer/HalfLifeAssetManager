#pragma once

#include <memory>

#include <QDialog>

class GameConfiguration;
class GameConfigurationsSettings;
class Ui_SelectGameConfigurationDialog;

class SelectGameConfigurationDialog final : public QDialog
{
public:
	explicit SelectGameConfigurationDialog(GameConfigurationsSettings* settings, QWidget* parent);
	~SelectGameConfigurationDialog();

	const GameConfiguration* GetSelectedConfiguration() const;

private:
	std::unique_ptr<Ui_SelectGameConfigurationDialog> _ui;
	GameConfigurationsSettings* const _settings;
};
