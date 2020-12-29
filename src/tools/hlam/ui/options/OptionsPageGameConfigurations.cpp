#include <algorithm>
#include <cassert>
#include <iterator>
#include <limits>

#include <QFileDialog>
#include <QSignalBlocker>
#include <QStringLiteral>

#include "ui/EditorContext.hpp"

#include "ui/options/OptionsPageGameConfigurations.hpp"
#include "ui/settings/GameConfiguration.hpp"
#include "ui/settings/GameConfigurationsSettings.hpp"
#include "ui/settings/GameEnvironment.hpp"

//TODO: implement automatic scanning of mods

using namespace ui::settings;

namespace ui::options
{
const QString OptionsPageGameConfigurationsCategory{QStringLiteral("T.GameConfigurations")};
const QString OptionsPageGameConfigurationsId{QStringLiteral("T.GameConfigurations")};

constexpr int GameConfigurationNameColumn = 0;
constexpr int GameConfigurationDirectoryColumn = 1;

const QString DefaultGameEnvironmentName{QStringLiteral("New Environment")};
const QString DefaultGameConfigurationName{QStringLiteral("New Mod")};

template<typename Container, typename Predicate>
static QString GenerateUniqueName(const QString& baseName, const Container& container, Predicate predicate)
{
	QString name{baseName};

	for (int i = 1; i < std::numeric_limits<int>::max(); ++i)
	{
		if (std::find_if(container.begin(), container.end(), [&](const auto& object)
			{
				return predicate(object, name);
			}
		) == container.end())
		{
			return name;
		}

		name = baseName + QString(" (%1)").arg(i);
	}

	//TODO: maybe use additional (%1) suffixes instead to retain unique names
	return baseName + " (Duplicate)";
}

OptionsPageGameConfigurations::OptionsPageGameConfigurations(const std::shared_ptr<settings::GameConfigurationsSettings>& gameConfigurationsSettings)
	: _gameConfigurationsSettings(gameConfigurationsSettings)
{
	SetCategory(QString{OptionsPageGameConfigurationsCategory});
	SetCategoryTitle("Game Configurations");
	SetId(QString{OptionsPageGameConfigurationsId});
	SetPageTitle("Game Configurations");
	SetWidgetFactory([](EditorContext* editorContext) {return new OptionsPageGameConfigurationsWidget(editorContext); });
}

OptionsPageGameConfigurationsWidget::OptionsPageGameConfigurationsWidget(EditorContext* editorContext, QWidget* parent)
	: OptionsWidget(parent)
	, _editorContext(editorContext)
{
	_ui.setupUi(this);

	_gameEnvironmentsModel = new QStandardItemModel(this);

	_ui.GameEnvironmentList->setModel(_gameEnvironmentsModel);

	_gameConfigurationsModel = new QStandardItemModel(this);

	{
		QStringList headers;

		headers.append("Name");
		headers.append("Game Directory");

		_gameConfigurationsModel->setHorizontalHeaderLabels(headers);
	}

	_ui.GameConfigurationList->setModel(_gameConfigurationsModel);

	{
		auto horizontalHeader = _ui.GameConfigurationList->horizontalHeader();

		horizontalHeader->resizeSection(GameConfigurationNameColumn, 200);
		horizontalHeader->setSectionResizeMode(1, QHeaderView::ResizeMode::Stretch);
	}

	_ui.ActiveEnvironment->setEnabled(false);
	_ui.ActiveConfiguration->setEnabled(false);
	_ui.RemoveGameEnvironment->setEnabled(false);
	_ui.EnvironmentConfiguration->setEnabled(false);
	_ui.RemoveGameConfiguration->setEnabled(false);

	connect(_ui.ActiveEnvironment, qOverload<int>(&QComboBox::currentIndexChanged), this, &OptionsPageGameConfigurationsWidget::OnActiveGameEnvironmentChanged);

	connect(_gameEnvironmentsModel, &QStandardItemModel::dataChanged, this, &OptionsPageGameConfigurationsWidget::OnGameEnvironmentNameChanged);
	connect(_ui.GameEnvironmentList->selectionModel(), &QItemSelectionModel::currentRowChanged,
		this, &OptionsPageGameConfigurationsWidget::OnGameEnvironmentSelectionChanged);
	connect(_ui.AddNewGameEnvironment, &QPushButton::clicked, this, &OptionsPageGameConfigurationsWidget::OnNewGameEnvironment);
	connect(_ui.RemoveGameEnvironment, &QPushButton::clicked, this, &OptionsPageGameConfigurationsWidget::OnRemoveGameEnvironment);

	connect(_ui.GameInstallLocation, &QLineEdit::textChanged, this, &OptionsPageGameConfigurationsWidget::OnGameInstallLocationChanged);
	connect(_ui.BrowseGameInstallation, &QPushButton::clicked, this, &OptionsPageGameConfigurationsWidget::OnBrowseGameInstallation);
	connect(_ui.DefaultGame, qOverload<int>(&QComboBox::currentIndexChanged), this, &OptionsPageGameConfigurationsWidget::OnDefaultGameChanged);

	connect(_gameConfigurationsModel, &QStandardItemModel::dataChanged, this, &OptionsPageGameConfigurationsWidget::OnGameConfigurationDataChanged);
	connect(_ui.GameConfigurationList->selectionModel(), &QItemSelectionModel::currentRowChanged,
		this, &OptionsPageGameConfigurationsWidget::OnGameConfigurationSelectionChanged);
	connect(_ui.AddNewGameConfiguration, &QPushButton::clicked, this, &OptionsPageGameConfigurationsWidget::OnNewGameConfiguration);
	connect(_ui.RemoveGameConfiguration, &QPushButton::clicked, this, &OptionsPageGameConfigurationsWidget::OnRemoveGameConfiguration);

	const auto gameConfigurations = _editorContext->GetGameConfigurations();

	const auto environments = gameConfigurations->GetGameEnvironments();

	const auto activeConfiguration = gameConfigurations->GetActiveConfiguration();

	int index = 0;

	for (const auto environment : environments)
	{
		AddGameEnvironment(std::make_unique<GameEnvironment>(*environment));

		if (activeConfiguration.first && environment->GetId() == activeConfiguration.first->GetId())
		{
			_ui.ActiveEnvironment->setCurrentIndex(index);

			if (activeConfiguration.second)
			{
				for (int configIndex = 0; configIndex < _ui.ActiveConfiguration->count(); ++configIndex)
				{
					if (_ui.ActiveConfiguration->itemData(configIndex).toUuid() == activeConfiguration.second->GetId())
					{
						_ui.ActiveConfiguration->setCurrentIndex(configIndex);
						break;
					}
				}
			}
		}

		++index;
	}
}

OptionsPageGameConfigurationsWidget::~OptionsPageGameConfigurationsWidget() = default;

void OptionsPageGameConfigurationsWidget::ApplyChanges(QSettings& settings)
{
	const auto gameConfigurations = _editorContext->GetGameConfigurations();

	for (const auto& environmentId : _gameEnvironmentsChangeSet.RemovedObjects)
	{
		gameConfigurations->RemoveGameEnvironment(environmentId);
	}

	for (const auto& environmentId : _gameEnvironmentsChangeSet.NewObjects)
	{
		auto it = std::find_if(_gameEnvironments.begin(), _gameEnvironments.end(), [&](const auto& environment)
			{
				return environment->GetId() == environmentId;
			}
		);

		assert(it != _gameEnvironments.end());

		gameConfigurations->AddGameEnvironment(std::make_unique<GameEnvironment>(*(*it)));
	}

	for (const auto& environmentId : _gameEnvironmentsChangeSet.UpdatedObjects)
	{
		auto it = std::find_if(_gameEnvironments.begin(), _gameEnvironments.end(), [&](const auto& environment)
			{
				return environment->GetId() == environmentId;
			}
		);

		assert(it != _gameEnvironments.end());

		auto source = it->get();

		auto target = gameConfigurations->GetGameEnvironmentById(environmentId);

		assert(target);

		target->SetName(source->GetName());
		target->SetInstallationPath(source->GetInstallationPath());
		target->SetDefaultModId(source->GetDefaultModId());

		if (auto configIt = _gameConfigurationsChangeSet.find(environmentId); configIt != _gameConfigurationsChangeSet.end())
		{
			for (const auto& configId : configIt->second->RemovedObjects)
			{
				target->RemoveGameConfiguration(configId);
			}

			for (const auto& configId : configIt->second->NewObjects)
			{
				auto sourceConfig = source->GetGameConfigurationById(configId);

				target->AddGameConfiguration(std::make_unique<GameConfiguration>(*sourceConfig));
			}

			for (const auto& configId : configIt->second->UpdatedObjects)
			{
				auto sourceConfig = source->GetGameConfigurationById(configId);
				auto targetConfig = target->GetGameConfigurationById(configId);

				targetConfig->SetDirectory(sourceConfig->GetDirectory());
				targetConfig->SetName(sourceConfig->GetName());
			}
		}
	}

	_gameEnvironmentsChangeSet.Clear();
	_gameConfigurationsChangeSet.clear();

	GameEnvironment* activeEnvironment = nullptr;
	GameConfiguration* activeConfiguration = nullptr;

	if (_ui.ActiveEnvironment->currentIndex() != -1)
	{
		auto item = _gameEnvironmentsModel->item(_ui.ActiveEnvironment->currentIndex());

		auto environment = item->data().value<GameEnvironment*>();

		activeEnvironment = gameConfigurations->GetGameEnvironmentById(environment->GetId());
	}

	if (activeEnvironment && _ui.ActiveConfiguration->currentIndex() != -1)
	{
		const auto id = _ui.ActiveConfiguration->itemData(_ui.ActiveConfiguration->currentIndex()).toUuid();

		activeConfiguration = activeEnvironment->GetGameConfigurationById(id);
	}

	gameConfigurations->SetActiveConfiguration({activeEnvironment, activeConfiguration});

	gameConfigurations->SaveSettings(settings);
}

OptionsPageGameConfigurationsWidget::ChangeSet* OptionsPageGameConfigurationsWidget::GetOrCreateGameConfigurationChangeSet(const QUuid& id)
{
	if (auto it = _gameConfigurationsChangeSet.find(id); it != _gameConfigurationsChangeSet.end())
	{
		return it->second.get();
	}

	auto it = _gameConfigurationsChangeSet.emplace(id, std::make_unique<ChangeSet>());

	return it.first->second.get();
}

void OptionsPageGameConfigurationsWidget::AddGameEnvironment(std::unique_ptr<GameEnvironment>&& gameEnvironment)
{
	auto item = new QStandardItem(gameEnvironment->GetName());

	item->setData(QVariant::fromValue(gameEnvironment.get()));

	{
		const QSignalBlocker blocker{_ui.GameEnvironmentList};

		_gameEnvironmentsModel->insertRow(_gameEnvironmentsModel->rowCount(), item);

		_ui.GameEnvironmentList->setCurrentIndex(item->index());
	}

	auto& ref = _gameEnvironments.emplace_back(std::move(gameEnvironment));

	_ui.ActiveEnvironment->addItem(ref->GetName());

	_ui.ActiveEnvironment->setEnabled(true);
}

void OptionsPageGameConfigurationsWidget::OnActiveGameEnvironmentChanged(int index)
{
	_ui.ActiveConfiguration->clear();

	if (index != -1)
	{
		auto gameEnvironment = _gameEnvironments[index].get();

		for (const auto gameConfiguration : gameEnvironment->GetGameConfigurations())
		{
			_ui.ActiveConfiguration->addItem(gameConfiguration->GetName());
			_ui.ActiveConfiguration->setItemData(_ui.ActiveConfiguration->count() - 1, gameConfiguration->GetId());
		}
	}

	_ui.ActiveConfiguration->setEnabled(_ui.ActiveConfiguration->count() > 0);

	_currentEnvironmentIsActive = _ui.GameEnvironmentList->currentIndex().row() == index;
}

void OptionsPageGameConfigurationsWidget::OnGameEnvironmentNameChanged(const QModelIndex& topLeft)
{
	auto item = _gameEnvironmentsModel->itemFromIndex(topLeft);

	const QString text = item->text();

	auto gameEnvironment = item->data().value<GameEnvironment*>();

	if (gameEnvironment->GetName() != text)
	{
		auto name = GenerateUniqueName(text, _gameEnvironments, [](const auto& object, const QString& name)
			{
				return object->GetName() == name;
			});

		if (name != text)
		{
			const QSignalBlocker blocker{_ui.GameEnvironmentList};

			item->setText(name);
		}

		_ui.ActiveEnvironment->setItemText(item->row(), name);

		gameEnvironment->SetName(std::move(name));

		_gameEnvironmentsChangeSet.MarkChanged(gameEnvironment->GetId());
	}
}

void OptionsPageGameConfigurationsWidget::OnGameEnvironmentSelectionChanged(const QModelIndex& current, const QModelIndex& previous)
{
	if (current.isValid())
	{
		auto item = _gameEnvironmentsModel->itemFromIndex(current);

		auto gameEnvironment = item->data().value<GameEnvironment*>();

		_ui.GameInstallLocation->setText(gameEnvironment->GetInstallationPath());

		_ui.DefaultGame->clear();

		//Clear all data first
		_gameConfigurationsModel->setRowCount(0);

		const auto gameConfigurations = gameEnvironment->GetGameConfigurations();

		_gameConfigurationsModel->setRowCount(gameConfigurations.size());

		int row = 0;

		for (auto gameConfiguration : gameConfigurations)
		{
			_ui.DefaultGame->addItem(gameConfiguration->GetName(), QVariant::fromValue(gameConfiguration));

			auto name = new QStandardItem(gameConfiguration->GetName());
			auto directory = new QStandardItem(gameConfiguration->GetDirectory());

			_gameConfigurationsModel->setItem(row, GameConfigurationNameColumn, name);
			_gameConfigurationsModel->setItem(row, GameConfigurationDirectoryColumn, directory);

			++row;
		}

		_ui.DefaultGame->setEnabled(_ui.DefaultGame->count() > 0);

		const auto& defaultModId = gameEnvironment->GetDefaultModId();

		if (!defaultModId.isNull())
		{
			for (int i = 0; i < _ui.DefaultGame->count(); ++i)
			{
				auto configuration = _ui.DefaultGame->itemData(i).value<GameConfiguration*>();

				if (configuration->GetId() == defaultModId)
				{
					_ui.DefaultGame->setCurrentText(configuration->GetName());
					break;
				}
			}
		}
	}
	else
	{
		_ui.GameInstallLocation->setText({});

		_ui.DefaultGame->clear();

		_gameConfigurationsModel->setRowCount(0);
	}

	_ui.RemoveGameEnvironment->setEnabled(current.isValid());
	_ui.EnvironmentConfiguration->setEnabled(current.isValid());

	_currentEnvironmentIsActive = current.row() == _ui.ActiveEnvironment->currentIndex();
}

void OptionsPageGameConfigurationsWidget::OnNewGameEnvironment()
{
	auto environmentName = GenerateUniqueName(DefaultGameEnvironmentName, _gameEnvironments, [](const auto& object, const QString& name)
		{
			return object->GetName() == name;
		});

	auto gameEnvironment = std::make_unique<GameEnvironment>(QUuid::createUuid(), std::move(environmentName));

	_gameEnvironmentsChangeSet.MarkNew(gameEnvironment->GetId());

	AddGameEnvironment(std::move(gameEnvironment));
}

void OptionsPageGameConfigurationsWidget::OnRemoveGameEnvironment()
{
	auto item = _gameEnvironmentsModel->itemFromIndex(_ui.GameEnvironmentList->currentIndex());

	const int row = item->row();

	auto gameEnvironment = item->data().value<GameEnvironment*>();

	_gameEnvironmentsChangeSet.MarkRemoved(gameEnvironment->GetId());

	_ui.ActiveEnvironment->removeItem(row);

	_gameEnvironmentsModel->removeRow(row);

	_gameEnvironments.erase(_gameEnvironments.begin() + row);

	_ui.ActiveEnvironment->setEnabled(_ui.ActiveEnvironment->count() > 0);
}

void OptionsPageGameConfigurationsWidget::OnGameInstallLocationChanged(const QString& text)
{
	const QModelIndex index = _ui.GameEnvironmentList->currentIndex();

	if (index.isValid())
	{
		auto gameEnvironment = _gameEnvironmentsModel->itemFromIndex(index)->data().value<GameEnvironment*>();

		gameEnvironment->SetInstallationPath(QString{text});

		_gameEnvironmentsChangeSet.MarkChanged(gameEnvironment->GetId());
	}
}

void OptionsPageGameConfigurationsWidget::OnBrowseGameInstallation()
{
	const auto path = QFileDialog::getExistingDirectory(this, "Browse Game Installation", _ui.GameInstallLocation->text());

	if (!path.isEmpty())
	{
		_ui.GameInstallLocation->setText(path);
	}
}

void OptionsPageGameConfigurationsWidget::OnDefaultGameChanged()
{
	const QModelIndex index = _ui.GameEnvironmentList->currentIndex();

	if (index.isValid())
	{
		auto gameEnvironment = _gameEnvironmentsModel->itemFromIndex(index)->data().value<GameEnvironment*>();

		if (_ui.DefaultGame->count() > 0)
		{
			auto configuration = _ui.DefaultGame->currentData().value<GameConfiguration*>();

			gameEnvironment->SetDefaultModId(configuration->GetId());
		}
		else
		{
			gameEnvironment->SetDefaultModId(QUuid{});
		}

		_gameEnvironmentsChangeSet.MarkChanged(gameEnvironment->GetId());
	}
}

void OptionsPageGameConfigurationsWidget::OnGameConfigurationDataChanged(const QModelIndex& topLeft)
{
	auto item = _gameConfigurationsModel->itemFromIndex(topLeft);

	auto gameEnvironment = _gameEnvironmentsModel->itemFromIndex(_ui.GameEnvironmentList->currentIndex())->data().value<GameEnvironment*>();
	auto gameConfiguration = _ui.DefaultGame->itemData(item->row()).value<GameConfiguration*>();

	_gameEnvironmentsChangeSet.MarkChanged(gameEnvironment->GetId());

	GetOrCreateGameConfigurationChangeSet(gameEnvironment->GetId())->MarkChanged(gameConfiguration->GetId());

	if (topLeft.column() == GameConfigurationNameColumn)
	{
		_ui.DefaultGame->setItemText(topLeft.row(), item->text());

		if (_currentEnvironmentIsActive)
		{
			_ui.ActiveConfiguration->setItemText(topLeft.row(), item->text());
		}

		gameConfiguration->SetName(item->text());
	}
	else if (topLeft.column() == GameConfigurationDirectoryColumn)
	{
		gameConfiguration->SetDirectory(item->text());
	}
}

void OptionsPageGameConfigurationsWidget::OnGameConfigurationSelectionChanged(const QModelIndex& current, const QModelIndex& previous)
{
	_ui.RemoveGameConfiguration->setEnabled(current.isValid());
}

void OptionsPageGameConfigurationsWidget::OnNewGameConfiguration()
{
	auto gameEnvironment = _gameEnvironmentsModel->itemFromIndex(_ui.GameEnvironmentList->currentIndex())->data().value<GameEnvironment*>();

	const auto gameConfigurations = gameEnvironment->GetGameConfigurations();

	auto configurationName = GenerateUniqueName(DefaultGameConfigurationName, gameConfigurations, [](const auto& object, const QString& name)
		{
			return object->GetName() == name;
		});

	auto gameConfiguration = std::make_unique<GameConfiguration>(QUuid::createUuid(), QString{}, std::move(configurationName));

	_ui.DefaultGame->addItem(gameConfiguration->GetName(), QVariant::fromValue(gameConfiguration.get()));
	_ui.DefaultGame->setEnabled(true);

	if (_currentEnvironmentIsActive)
	{
		_ui.ActiveConfiguration->addItem(gameConfiguration->GetName(), QVariant::fromValue(gameConfiguration.get()));
		_ui.ActiveConfiguration->setEnabled(true);
	}

	const int row = _gameConfigurationsModel->rowCount();

	_gameConfigurationsModel->setRowCount(row + 1);

	auto name = new QStandardItem(gameConfiguration->GetName());
	auto directory = new QStandardItem(gameConfiguration->GetDirectory());

	_gameConfigurationsModel->setItem(row, GameConfigurationNameColumn, name);
	_gameConfigurationsModel->setItem(row, GameConfigurationDirectoryColumn, directory);

	_gameEnvironmentsChangeSet.MarkChanged(gameEnvironment->GetId());

	GetOrCreateGameConfigurationChangeSet(gameEnvironment->GetId())->MarkNew(gameConfiguration->GetId());

	gameEnvironment->AddGameConfiguration(std::move(gameConfiguration));

	_ui.GameConfigurationList->setCurrentIndex(name->index());
}

void OptionsPageGameConfigurationsWidget::OnRemoveGameConfiguration()
{
	auto gameEnvironment = _gameEnvironmentsModel->itemFromIndex(_ui.GameEnvironmentList->currentIndex())->data().value<GameEnvironment*>();

	auto item = _gameConfigurationsModel->itemFromIndex(_ui.GameConfigurationList->currentIndex());

	auto gameConfiguration = _ui.DefaultGame->itemData(item->row()).value<GameConfiguration*>();

	_gameEnvironmentsChangeSet.MarkChanged(gameEnvironment->GetId());

	GetOrCreateGameConfigurationChangeSet(gameEnvironment->GetId())->MarkRemoved(gameConfiguration->GetId());

	if (_currentEnvironmentIsActive)
	{
		_ui.ActiveConfiguration->removeItem(item->row());
		_ui.ActiveConfiguration->setEnabled(_ui.ActiveConfiguration->count() > 0);
	}

	_ui.DefaultGame->removeItem(item->row());
	_ui.DefaultGame->setEnabled(_ui.DefaultGame->count() > 0);

	gameEnvironment->RemoveGameConfiguration(gameConfiguration->GetId());

	_gameConfigurationsModel->removeRow(item->row());
}
}
