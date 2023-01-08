#include <algorithm>
#include <cassert>

#include <QFileDialog>
#include <QSignalBlocker>
#include <QStringLiteral>
#include <QWizard>

#include "settings/GameConfigurationsSettings.hpp"

#include "application/AssetManager.hpp"

#include "ui/options/gameconfigurations/AutodetectGameConfigurationsWizard.hpp"
#include "ui/options/gameconfigurations/EditGameConfigurationsDialog.hpp"
#include "ui/options/gameconfigurations/OptionsPageGameConfigurations.hpp"
#include "ui/options/gameconfigurations/GameConfigurationsOptions.hpp"

const QString OptionsPageGameConfigurationsCategory{QStringLiteral("T.GameConfigurations")};
const QString OptionsPageGameConfigurationsId{QStringLiteral("T.GameConfigurations")};

OptionsPageGameConfigurations::OptionsPageGameConfigurations()
{
	SetCategory(QString{OptionsPageGameConfigurationsCategory});
	SetCategoryTitle("Game Configurations");
	SetId(QString{OptionsPageGameConfigurationsId});
	SetPageTitle("Game Configurations");
	SetWidgetFactory([](AssetManager* application)
		{
			return new OptionsPageGameConfigurationsWidget(application);
		});
}

OptionsPageGameConfigurationsWidget::OptionsPageGameConfigurationsWidget(AssetManager* application)
	: _application(application)
	, _options(std::make_unique<GameConfigurationsOptions>())
{
	_ui.setupUi(this);

	_ui.DefaultConfiguration->setModel(_options->GameConfigurationsModel.get());
	_ui.Configurations->setModel(_options->GameConfigurationsModel.get());

	connect(_options->GameConfigurationsModel.get(), &QStandardItemModel::rowsInserted, this,
		[this](const QModelIndex& parent, int first)
		{
			_ui.DefaultConfiguration->setEnabled(true);
			_ui.Configurations->setEnabled(true);
			_ui.Configurations->setCurrentIndex(first);
		});

	connect(_ui.EditConfigurations, &QPushButton::clicked,
		this, &OptionsPageGameConfigurationsWidget::OnEditGameConfigurations);

	connect(_ui.Configurations, qOverload<int>(&QComboBox::currentIndexChanged),
		this, &OptionsPageGameConfigurationsWidget::OnConfigurationChanged);

	connect(_ui.GameExecutable, &QLineEdit::textChanged,
		this, &OptionsPageGameConfigurationsWidget::OnGameExecutableChanged);
	connect(_ui.BrowseGameExecutable, &QPushButton::clicked,
		this, &OptionsPageGameConfigurationsWidget::OnBrowseGameExecutable);

	connect(_ui.BaseGameDirectory, &QLineEdit::textChanged,
		this, &OptionsPageGameConfigurationsWidget::OnBaseGameDirectoryChanged);
	connect(_ui.BrowseBaseGameDirectory, &QPushButton::clicked,
		this, &OptionsPageGameConfigurationsWidget::OnBrowseBaseGameDirectory);

	connect(_ui.ModDirectory, &QLineEdit::textChanged,
		this, &OptionsPageGameConfigurationsWidget::OnModDirectoryChanged);
	connect(_ui.BrowseModDirectory, &QPushButton::clicked,
		this, &OptionsPageGameConfigurationsWidget::OnBrowseModDirectory);

	connect(_ui.Autodetect, &QPushButton::clicked, this, &OptionsPageGameConfigurationsWidget::OnAutodetect);

	const auto gameConfigurations = _application->GetGameConfigurations();

	const auto defaultConfiguration = gameConfigurations->GetDefaultConfiguration();

	for (int index = 0; const auto configuration : gameConfigurations->GetConfigurations())
	{
		AddGameConfiguration(std::make_unique<GameConfiguration>(*configuration), false);

		if (configuration == defaultConfiguration)
		{
			_ui.DefaultConfiguration->setCurrentIndex(index);
		}

		++index;
	}

	_ui.Configurations->setCurrentIndex(_ui.DefaultConfiguration->currentIndex());
}

OptionsPageGameConfigurationsWidget::~OptionsPageGameConfigurationsWidget() = default;

void OptionsPageGameConfigurationsWidget::ApplyChanges()
{
	const auto gameConfigurations = _application->GetGameConfigurations();

	for (const auto& configurationId : _options->GameConfigurationsChangeSet.RemovedObjects)
	{
		gameConfigurations->RemoveConfiguration(configurationId);
	}

	for (const auto& configurationId : _options->GameConfigurationsChangeSet.NewObjects)
	{
		auto it = std::find_if(_options->GameConfigurations.begin(), _options->GameConfigurations.end(),
			[&](const auto& configuration)
			{
				return configuration->Id == configurationId;
			}
		);

		assert(it != _options->GameConfigurations.end());

		gameConfigurations->AddConfiguration(std::make_unique<GameConfiguration>(**it));
	}

	for (const auto& environmentId : _options->GameConfigurationsChangeSet.UpdatedObjects)
	{
		auto it = std::find_if(_options->GameConfigurations.begin(), _options->GameConfigurations.end(),
			[&](const auto& configuration)
			{
				return configuration->Id == environmentId;
			}
		);

		assert(it != _options->GameConfigurations.end());

		gameConfigurations->UpdateConfiguration(**it);
	}

	_options->GameConfigurationsChangeSet.Clear();

	QUuid defaultConfigurationId{};

	if (_ui.DefaultConfiguration->currentIndex() != -1)
	{
		defaultConfigurationId = _options->GameConfigurations[_ui.DefaultConfiguration->currentIndex()]->Id;
	}

	gameConfigurations->SetDefaultConfiguration(defaultConfigurationId);
}

void OptionsPageGameConfigurationsWidget::AddGameConfiguration(
	std::unique_ptr<GameConfiguration>&& configuration, bool markAsNew)
{
	_options->AddGameConfiguration(std::move(configuration), markAsNew);
}

void OptionsPageGameConfigurationsWidget::OnEditGameConfigurations()
{
	EditGameConfigurationsDialog dialog{
		_options->GameConfigurationsModel.get(), _ui.Configurations->currentIndex(), this};

	connect(&dialog, &EditGameConfigurationsDialog::ConfigurationAdded, this,
		[this](const QString& name)
		{
			auto configuration = std::make_unique<GameConfiguration>();

			configuration->Id = QUuid::createUuid();
			configuration->Name = name;

			AddGameConfiguration(std::move(configuration), true);
		});

	connect(&dialog, &EditGameConfigurationsDialog::ConfigurationRemoved, this,
		[this](int index)
		{
			_options->GameConfigurationsChangeSet.MarkRemoved(_options->GameConfigurations[index]->Id);
			_options->GameConfigurations.erase(_options->GameConfigurations.begin() + index);
			_options->GameConfigurationsModel->removeRow(index);
			_ui.DefaultConfiguration->setEnabled(_options->GameConfigurationsModel->rowCount() > 0);
			_ui.Configurations->setEnabled(_options->GameConfigurationsModel->rowCount() > 0);
		});

	connect(&dialog, &EditGameConfigurationsDialog::ConfigurationCopied, this,
		[this](int index)
		{
			auto configuration = std::make_unique<GameConfiguration>(*_options->GameConfigurations[index]);

			configuration->Id = QUuid::createUuid();
			configuration->Name += " (Copy)";

			AddGameConfiguration(std::move(configuration), true);
		});

	connect(&dialog, &EditGameConfigurationsDialog::ConfigurationRenamed, this,
		[this](int index, const QString& name)
		{
			_options->GameConfigurationsChangeSet.MarkChanged(_options->GameConfigurations[index]->Id);
			_options->GameConfigurations[index]->Name = name;
			_options->GameConfigurationsModel->item(index)->setText(name);
		});

	dialog.exec();
}

void OptionsPageGameConfigurationsWidget::OnConfigurationChanged(int index)
{
	const GameConfiguration dummy;

	const GameConfiguration* configuration = index != -1 ? _options->GameConfigurations[index].get() : &dummy;

	const QSignalBlocker exe{_ui.GameExecutable};
	const QSignalBlocker base{_ui.BaseGameDirectory};
	const QSignalBlocker mod{_ui.ModDirectory};

	_ui.GameExecutable->setText(configuration->GameExecutable);
	_ui.BaseGameDirectory->setText(configuration->BaseGameDirectory);
	_ui.ModDirectory->setText(configuration->ModDirectory);

	_ui.ConfigurationData->setEnabled(index != -1);
}

void OptionsPageGameConfigurationsWidget::OnGameExecutableChanged(const QString& text)
{
	if (const int index = _ui.Configurations->currentIndex(); index != -1)
	{
		auto configuration = _options->GameConfigurations[index].get();
		configuration->GameExecutable = text;
		_options->GameConfigurationsChangeSet.MarkChanged(configuration->Id);
	}
}

void OptionsPageGameConfigurationsWidget::OnBrowseGameExecutable()
{
	const auto path = QFileDialog::getOpenFileName(this, "Select Game Executable", _ui.GameExecutable->text(),
		QStringLiteral("Executable Files (*.exe);;All Files (*.*)"));

	if (!path.isEmpty())
	{
		_ui.GameExecutable->setText(path);
	}
}

void OptionsPageGameConfigurationsWidget::OnBaseGameDirectoryChanged(const QString& text)
{
	if (const int index = _ui.Configurations->currentIndex(); index != -1)
	{
		auto configuration = _options->GameConfigurations[index].get();
		configuration->BaseGameDirectory = text;
		_options->GameConfigurationsChangeSet.MarkChanged(configuration->Id);
	}
}

void OptionsPageGameConfigurationsWidget::OnBrowseBaseGameDirectory()
{
	const auto path = QFileDialog::getExistingDirectory(
		this, "Select Base Game Directory", _ui.BaseGameDirectory->text());

	if (!path.isEmpty())
	{
		_ui.BaseGameDirectory->setText(path);
	}
}

void OptionsPageGameConfigurationsWidget::OnModDirectoryChanged(const QString& text)
{
	if (const int index = _ui.Configurations->currentIndex(); index != -1)
	{
		auto configuration = _options->GameConfigurations[index].get();
		configuration->ModDirectory = text;
		_options->GameConfigurationsChangeSet.MarkChanged(configuration->Id);
	}
}

void OptionsPageGameConfigurationsWidget::OnBrowseModDirectory()
{
	const auto path = QFileDialog::getExistingDirectory(this, "Select Mod Directory", _ui.ModDirectory->text());

	if (!path.isEmpty())
	{
		_ui.ModDirectory->setText(path);
	}
}

void OptionsPageGameConfigurationsWidget::OnAutodetect()
{
	AutodetectGameConfigurationsWizard wizard{_options.get()};
	wizard.exec();
}
