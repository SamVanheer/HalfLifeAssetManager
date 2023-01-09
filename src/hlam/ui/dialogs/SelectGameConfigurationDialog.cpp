#include "ui_SelectGameConfigurationDialog.h"

#include "settings/GameConfigurationsSettings.hpp"

#include "ui/dialogs/SelectGameConfigurationDialog.hpp"

SelectGameConfigurationDialog::SelectGameConfigurationDialog(GameConfigurationsSettings* settings, QWidget* parent)
	: QDialog(parent)
	, _ui(std::make_unique<Ui_SelectGameConfigurationDialog>())
	, _settings(settings)
{
	_ui->setupUi(this);

	const auto configurations = _settings->GetConfigurations();

	int selectedConfigurationIndex = 0;

	for (int index = 0; auto configuration : configurations)
	{
		auto item = new QListWidgetItem(configuration->Name);
		item->setData(Qt::UserRole, configuration->Id);
		_ui->Configurations->insertItem(index, item);

		if (_settings->GetDefaultConfiguration() == configuration)
		{
			selectedConfigurationIndex = index;
		}

		++index;
	}

	_ui->Configurations->setCurrentRow(selectedConfigurationIndex);
}

SelectGameConfigurationDialog::~SelectGameConfigurationDialog() = default;

const GameConfiguration* SelectGameConfigurationDialog::GetSelectedConfiguration() const
{
	const auto item = _ui->Configurations->currentItem();

	if (!item)
	{
		return nullptr;
	}

	return _settings->GetConfigurationById(item->data(Qt::UserRole).toUuid());
}
