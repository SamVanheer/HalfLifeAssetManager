#pragma once

#include <QWizardPage>

#include "ui_AutodetectScanResultsPage.h"

class GameConfigurationsOptions;
class QTableWidget;

class AutodetectScanResultsPage final : public QWizardPage
{
public:
	explicit AutodetectScanResultsPage(GameConfigurationsOptions* options, QWidget* parent = nullptr);
	~AutodetectScanResultsPage();

	void initializePage() override;

	void cleanupPage() override;

	bool isComplete() const override;

	QTableWidget* GetConfigurationsToAdd() const;

private slots:
	void OnStartScan();

private:
	Ui_AutodetectScanResultsPage _ui;
	GameConfigurationsOptions* const _options;
};
