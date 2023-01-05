#include <cmath>

#include <QApplication>
#include <QDesktopWidget>
#include <QTableWidget>

#include "ui/options/gameconfigurations/AutodetectConfigurePropertiesPage.hpp"
#include "ui/options/gameconfigurations/AutodetectGameConfigurationsWizard.hpp"
#include "ui/options/gameconfigurations/AutodetectScanResultsPage.hpp"
#include "ui/options/gameconfigurations/GameConfigurationsOptions.hpp"

AutodetectGameConfigurationsWizard::AutodetectGameConfigurationsWizard(
	GameConfigurationsOptions* options, QWidget* parent)
	: QWizard(parent)
	, _options(options)
{
	//Set the dialog size to a fraction of the current screen
	const QRect screenSize = QApplication::desktop()->screenGeometry(this);

	this->resize(
		static_cast<int>(std::ceil(screenSize.width() * 0.9)),
		static_cast<int>(std::ceil(screenSize.height() * 0.9)));

	setWindowTitle("Auto-detect Game Configurations");
	setWindowIcon(QIcon{":/hlam.ico"});
	setButtonText(QWizard::FinishButton, "Add Configurations");

	addPage(new AutodetectConfigurePropertiesPage());
	auto scanResultsPage = new AutodetectScanResultsPage(options);
	_configurationsToAdd = scanResultsPage->GetConfigurationsToAdd();
	addPage(scanResultsPage);
}

AutodetectGameConfigurationsWizard::~AutodetectGameConfigurationsWizard() = default;

void AutodetectGameConfigurationsWizard::accept()
{
	const auto gameExecutable = field("GameExecutable").toString();
	const auto baseGameDirectory = field("BaseGameDirectory").toString();

	for (int i = 0; i < _configurationsToAdd->rowCount(); ++i)
	{
		const auto name = _configurationsToAdd->item(i, 0)->text();
		const auto modDirectory = _configurationsToAdd->item(i, 1)->text();

		// If this mod is the base game don't set its mod directory.
		_options->AddGameConfiguration(std::make_unique<GameConfiguration>(
			QUuid::createUuid(),
			name,
			gameExecutable,
			baseGameDirectory,
			baseGameDirectory != modDirectory ? modDirectory : QString{}),
			true);
	}

	QWizard::accept();
}
