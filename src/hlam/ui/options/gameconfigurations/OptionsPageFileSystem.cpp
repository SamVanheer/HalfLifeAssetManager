#include <QStringLiteral>

#include "application/AssetManager.hpp"
#include "filesystem/FileSystemConstants.hpp"
#include "settings/GameConfigurationsSettings.hpp"

#include "ui/options/gameconfigurations/AutodetectGameConfigurationsWizard.hpp"
#include "ui/options/gameconfigurations/EditGameConfigurationsDialog.hpp"
#include "ui/options/gameconfigurations/OptionsPageFileSystem.hpp"

const QString OptionsPageGameConfigurationsCategory{QStringLiteral("T.GameConfigurations")};
const QString OptionsPageFileSystemId{QStringLiteral("T.FileSystem")};

OptionsPageFileSystem::OptionsPageFileSystem()
{
	SetCategory(QString{OptionsPageGameConfigurationsCategory});
	SetCategoryTitle("Game Configurations");
	SetId(QString{OptionsPageFileSystemId});
	SetPageTitle("File System");
	SetWidgetFactory([](AssetManager* application)
		{
			return new OptionsPageFileSystemWidget(application);
		});
}

OptionsPageFileSystemWidget::OptionsPageFileSystemWidget(AssetManager* application)
	: _application(application)
{
	_ui.setupUi(this);

	const auto gameConfigurations = _application->GetGameConfigurations();

	_ui.CheckHDDirectories->setChecked(gameConfigurations->ShouldCheckHDDirectories());
	_ui.CheckAddonDirectories->setChecked(gameConfigurations->ShouldCheckAddonDirectories());
	_ui.CheckDownloadsDirectories->setChecked(gameConfigurations->ShouldCheckDownloadsDirectories());

	for (const auto& language : SteamLanguages)
	{
		_ui.SteamLanguage->addItem(QString::fromStdString(std::string{language}));
	}

	if (auto it = std::find(SteamLanguages.begin(), SteamLanguages.end(),
		gameConfigurations->GetSteamLanguage().toStdString());
		it != SteamLanguages.end())
	{
		_ui.SteamLanguage->setCurrentIndex(it - SteamLanguages.begin());
	}
}

OptionsPageFileSystemWidget::~OptionsPageFileSystemWidget() = default;

void OptionsPageFileSystemWidget::ApplyChanges()
{
	const auto gameConfigurations = _application->GetGameConfigurations();

	gameConfigurations->SetCheckHDDirectories(_ui.CheckHDDirectories->isChecked());
	gameConfigurations->SetCheckAddonDirectories(_ui.CheckAddonDirectories->isChecked());
	gameConfigurations->SetCheckDownloadsDirectories(_ui.CheckDownloadsDirectories->isChecked());
	gameConfigurations->SetSteamLanguage(_ui.SteamLanguage->currentText());
}
