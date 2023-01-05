#pragma once

#include <QWizard>

class GameConfigurationsOptions;
class QTableWidget;

class AutodetectGameConfigurationsWizard final : public QWizard
{
public:
	explicit AutodetectGameConfigurationsWizard(GameConfigurationsOptions* options, QWidget* parent = nullptr);
	~AutodetectGameConfigurationsWizard();

	void accept() override;

private:
	GameConfigurationsOptions* const _options;
	QTableWidget* _configurationsToAdd;
};
