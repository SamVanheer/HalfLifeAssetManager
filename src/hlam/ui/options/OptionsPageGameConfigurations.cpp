#include <algorithm>
#include <cassert>

#include <QFileDialog>
#include <QSignalBlocker>
#include <QStringLiteral>

#include "settings/GameConfigurationsSettings.hpp"

#include "ui/EditorContext.hpp"

#include "ui/options/EditGameConfigurationsDialog.hpp"
#include "ui/options/OptionsPageGameConfigurations.hpp"

//TODO: implement automatic scanning of mods

const QString OptionsPageGameConfigurationsCategory{QStringLiteral("T.GameConfigurations")};
const QString OptionsPageGameConfigurationsId{QStringLiteral("T.GameConfigurations")};

OptionsPageGameConfigurations::OptionsPageGameConfigurations()
{
	SetCategory(QString{OptionsPageGameConfigurationsCategory});
	SetCategoryTitle("Game Configurations");
	SetId(QString{OptionsPageGameConfigurationsId});
	SetPageTitle("Game Configurations");
	SetWidgetFactory([](EditorContext* editorContext)
		{
			return new OptionsPageGameConfigurationsWidget(editorContext);
		});
}

OptionsPageGameConfigurationsWidget::OptionsPageGameConfigurationsWidget(EditorContext* editorContext)
	: _editorContext(editorContext)
{
	_ui.setupUi(this);

	_gameConfigurationsModel = new QStandardItemModel(this);

	_ui.DefaultConfiguration->setModel(_gameConfigurationsModel);
	_ui.Configurations->setModel(_gameConfigurationsModel);

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

	const auto gameConfigurations = _editorContext->GetGameConfigurations();

	const auto defaultConfiguration = gameConfigurations->GetDefaultConfiguration();

	for (int index = 0; const auto configuration : gameConfigurations->GetConfigurations())
	{
		AddGameConfiguration(std::make_unique<GameConfiguration>(*configuration));

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
	const auto gameConfigurations = _editorContext->GetGameConfigurations();

	for (const auto& configurationId : _gameConfigurationsChangeSet.RemovedObjects)
	{
		gameConfigurations->RemoveConfiguration(configurationId);
	}

	for (const auto& configurationId : _gameConfigurationsChangeSet.NewObjects)
	{
		auto it = std::find_if(_gameConfigurations.begin(), _gameConfigurations.end(),
			[&](const auto& configuration)
			{
				return configuration->Id == configurationId;
			}
		);

		assert(it != _gameConfigurations.end());

		gameConfigurations->AddConfiguration(std::make_unique<GameConfiguration>(**it));
	}

	for (const auto& environmentId : _gameConfigurationsChangeSet.UpdatedObjects)
	{
		auto it = std::find_if(_gameConfigurations.begin(), _gameConfigurations.end(),
			[&](const auto& configuration)
			{
				return configuration->Id == environmentId;
			}
		);

		assert(it != _gameConfigurations.end());

		gameConfigurations->UpdateConfiguration(**it);
	}

	_gameConfigurationsChangeSet.Clear();

	QUuid defaultConfigurationId{};

	if (_ui.DefaultConfiguration->currentIndex() != -1)
	{
		defaultConfigurationId = _gameConfigurations[_ui.DefaultConfiguration->currentIndex()]->Id;
	}

	gameConfigurations->SetDefaultConfiguration(defaultConfigurationId);
}

void OptionsPageGameConfigurationsWidget::AddGameConfiguration(std::unique_ptr<GameConfiguration>&& configuration)
{
	auto& ref = _gameConfigurations.emplace_back(std::move(configuration));

	_ui.DefaultConfiguration->setEnabled(true);
	_ui.Configurations->setEnabled(true);

	auto item = new QStandardItem(ref->Name);

	_gameConfigurationsModel->insertRow(_gameConfigurationsModel->rowCount(), item);
	_ui.Configurations->setCurrentIndex(item->index().row());
}

void OptionsPageGameConfigurationsWidget::OnEditGameConfigurations()
{
	EditGameConfigurationsDialog dialog{_gameConfigurationsModel, _ui.Configurations->currentIndex(), this};

	connect(&dialog, &EditGameConfigurationsDialog::ConfigurationAdded, this,
		[this](const QString& name)
		{
			auto configuration = std::make_unique<GameConfiguration>();

			configuration->Id = QUuid::createUuid();
			configuration->Name = name;

			_gameConfigurationsChangeSet.MarkNew(configuration->Id);
			AddGameConfiguration(std::move(configuration));
		});

	connect(&dialog, &EditGameConfigurationsDialog::ConfigurationRemoved, this,
		[this](int index)
		{
			_gameConfigurationsChangeSet.MarkRemoved(_gameConfigurations[index]->Id);
			_gameConfigurations.erase(_gameConfigurations.begin() + index);
			_gameConfigurationsModel->removeRow(index);
			_ui.DefaultConfiguration->setEnabled(_gameConfigurationsModel->rowCount() > 0);
			_ui.Configurations->setEnabled(_gameConfigurationsModel->rowCount() > 0);
		});

	connect(&dialog, &EditGameConfigurationsDialog::ConfigurationCopied, this,
		[this](int index)
		{
			auto configuration = std::make_unique<GameConfiguration>(*_gameConfigurations[index]);

			configuration->Id = QUuid::createUuid();
			configuration->Name += " (Copy)";

			_gameConfigurationsChangeSet.MarkNew(configuration->Id);
			AddGameConfiguration(std::move(configuration));
		});

	connect(&dialog, &EditGameConfigurationsDialog::ConfigurationRenamed, this,
		[this](int index, const QString& name)
		{
			_gameConfigurationsChangeSet.MarkChanged(_gameConfigurations[index]->Id);
			_gameConfigurations[index]->Name = name;
			_gameConfigurationsModel->item(index)->setText(name);
		});

	dialog.exec();
}

void OptionsPageGameConfigurationsWidget::OnConfigurationChanged(int index)
{
	const GameConfiguration dummy;

	const GameConfiguration* configuration = index != -1 ? _gameConfigurations[index].get() : &dummy;

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
		auto configuration = _gameConfigurations[index].get();
		configuration->GameExecutable = text;
		_gameConfigurationsChangeSet.MarkChanged(configuration->Id);
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
		auto configuration = _gameConfigurations[index].get();
		configuration->BaseGameDirectory = text;
		_gameConfigurationsChangeSet.MarkChanged(configuration->Id);
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
		auto configuration = _gameConfigurations[index].get();
		configuration->ModDirectory = text;
		_gameConfigurationsChangeSet.MarkChanged(configuration->Id);
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
