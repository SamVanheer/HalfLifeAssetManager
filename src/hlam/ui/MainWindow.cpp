#include <algorithm>
#include <cassert>
#include <string_view>
#include <utility>
#include <vector>

#include <QActionGroup>
#include <QApplication>
#include <QCloseEvent>
#include <QDir>
#include <QDockWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QScreen>
#include <QWindow>

#include "ProjectInfo.hpp"

#include "assets/AssetIO.hpp"

#include "filesystem/IFileSystem.hpp"
#include "filesystem/FileSystemConstants.hpp"

#include "graphics/TextureLoader.hpp"

#include "qt/QtLogging.hpp"

#include "soundsystem/ISoundSystem.hpp"

#include "ui/DragNDropEventFilter.hpp"
#include "ui/EditorContext.hpp"
#include "ui/FileListPanel.hpp"
#include "ui/FullscreenWidget.hpp"
#include "ui/MainWindow.hpp"

#include "ui/assets/Assets.hpp"

#include "ui/options/OptionsDialog.hpp"

#include "ui/settings/GameConfigurationsSettings.hpp"
#include "ui/settings/GeneralSettings.hpp"
#include "ui/settings/RecentFilesSettings.hpp"

constexpr std::string_view TabWidgetAssetProperty{"TabWidgetAssetProperty"};
const QString AssetPathName{QStringLiteral("AssetPath")};

MainWindow::MainWindow(EditorContext* editorContext)
	: QMainWindow()
	, _editorContext(editorContext)
{
	_ui.setupUi(this);

	this->setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);

	this->setWindowIcon(QIcon{":/hlam.ico"});

	this->installEventFilter(_editorContext->GetDragNDropEventFilter());

	{
		auto undo = _undoGroup->createUndoAction(this);
		auto redo = _undoGroup->createRedoAction(this);

		undo->setShortcut(QKeySequence::StandardKey::Undo);
		redo->setShortcut(QKeySequence::StandardKey::Redo);

		_ui.MenuEdit->addAction(undo);
		_ui.MenuEdit->addAction(redo);
	}

	{
		const auto before = _ui.MenuTools->insertSeparator(_ui.ActionOptions);

		//Create the tool menu for each provider, sort by provider name, then add them all
		std::vector<std::pair<QString, QMenu*>> menus;

		for (auto provider : _editorContext->GetAssetProviderRegistry()->GetAssetProviders())
		{
			if (auto menu = provider->CreateToolMenu(_editorContext); menu)
			{
				menus.emplace_back(provider->GetProviderName(), menu);
			}
		}

		std::sort(menus.begin(), menus.end(), [](const auto& lhs, const auto& rhs)
			{
				return lhs.first.compare(rhs.first, Qt::CaseSensitivity::CaseInsensitive) < 0;
			});

		for (const auto& menu : menus)
		{
			menu.second->setParent(_ui.MenuTools, menu.second->windowFlags());
			_ui.MenuTools->insertMenu(before, menu.second);
		}
	}

	{
		auto fileList = new FileListPanel(_editorContext, this);

		connect(fileList, &FileListPanel::FileSelected, this, &MainWindow::OnFileSelected);

		_fileListDock = new QDockWidget(this);

		_fileListDock->setWidget(fileList);
		_fileListDock->setWindowTitle("File List");

		this->addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, _fileListDock);

		_fileListDock->hide();

		_ui.MenuWindows->addAction(_fileListDock->toggleViewAction());
	}

	_assetTabs = new QTabWidget(this);

	//Eliminate the border on the sides so the scene widget takes up all horizontal space
	_assetTabs->setDocumentMode(true);
	_assetTabs->setTabsClosable(true);
	_assetTabs->setElideMode(Qt::TextElideMode::ElideLeft);

	setCentralWidget(_assetTabs);

	setAcceptDrops(true);

	{
		_msaaActionGroup = new QActionGroup(this);

		_msaaActionGroup->addAction(_ui.ActionMSAANone);

		for (int i = 1; i < 5; ++i)
		{
			auto action = _ui.MenuMSAA->addAction(QString{"%1x MSAA"}.arg(1 << i));
			_msaaActionGroup->addAction(action);

			action->setCheckable(true);
		}

		int index = _editorContext->GetGeneralSettings()->GetMSAALevel();

		// Won't match the actual setting but this lets the user override the level manually.
		if (index < 0 || index >= _msaaActionGroup->actions().size())
		{
			index = 0;
		}

		_msaaActionGroup->actions()[index]->setChecked(true);
	}

	{
		const int index = static_cast<int>(_editorContext->GetGeneralSettings()->GuidelinesAspectRatio);
		_ui.GuidelinesAspectRatioGroup->actions()[index]->setChecked(true);
	}

	connect(_ui.ActionLoad, &QAction::triggered, this, &MainWindow::OnOpenLoadAssetDialog);
	connect(_ui.ActionSave, &QAction::triggered, this, &MainWindow::OnSaveAsset);
	connect(_ui.ActionSaveAs, &QAction::triggered, this, &MainWindow::OnSaveAssetAs);
	connect(_ui.ActionClose, &QAction::triggered, this, &MainWindow::OnCloseAsset);
	connect(_ui.ActionExit, &QAction::triggered, this, &MainWindow::OnExit);

	connect(_ui.ActionFullscreen, &QAction::triggered, this, &MainWindow::OnGoFullscreen);

	connect(_ui.ActionPowerOf2Textures, &QAction::toggled,
		_editorContext->GetGeneralSettings(), &GeneralSettings::SetResizeTexturesToPowerOf2);

	connect(_ui.ActionMinPoint, &QAction::triggered, this, &MainWindow::OnTextureFiltersChanged);
	connect(_ui.ActionMinLinear, &QAction::triggered, this, &MainWindow::OnTextureFiltersChanged);

	connect(_ui.ActionMagPoint, &QAction::triggered, this, &MainWindow::OnTextureFiltersChanged);
	connect(_ui.ActionMagLinear, &QAction::triggered, this, &MainWindow::OnTextureFiltersChanged);

	connect(_ui.ActionMipmapNone, &QAction::triggered, this, &MainWindow::OnTextureFiltersChanged);
	connect(_ui.ActionMipmapPoint, &QAction::triggered, this, &MainWindow::OnTextureFiltersChanged);
	connect(_ui.ActionMipmapLinear, &QAction::triggered, this, &MainWindow::OnTextureFiltersChanged);

	{
		const auto lambda = [this]()
		{
			const int index = _msaaActionGroup->actions().indexOf(_msaaActionGroup->checkedAction());
			_editorContext->GetGeneralSettings()->SetMSAALevel(index);
		};

		for (auto action : _msaaActionGroup->actions())
		{
			connect(action, &QAction::triggered, this, lambda);
		}
	}

	connect(_ui.ActionTransparentScreenshots, &QAction::triggered, this, [this](bool value)
		{
			_editorContext->GetGeneralSettings()->TransparentScreenshots = value;
		});

	connect(_ui.ActionRefresh, &QAction::triggered, this, &MainWindow::OnRefreshAsset);

	connect(_ui.ActionPlaySounds, &QAction::triggered, this, &MainWindow::OnPlaySoundsChanged);
	connect(_ui.ActionFramerateAffectsPitch, &QAction::triggered, this, &MainWindow::OnFramerateAffectsPitchChanged);

	{
		const auto lambda = [this]()
		{
			const int index = _ui.GuidelinesAspectRatioGroup->actions()
				.indexOf(_ui.GuidelinesAspectRatioGroup->checkedAction());
			_editorContext->GetGeneralSettings()->GuidelinesAspectRatio = static_cast<GuidelinesAspectRatio>(index);
		};

		for (auto action : _ui.GuidelinesAspectRatioGroup->actions())
		{
			connect(action, &QAction::triggered, this, lambda);
		}
	}

	connect(_ui.ActionOptions, &QAction::triggered, this, &MainWindow::OnOpenOptionsDialog);
	connect(_ui.ActionAbout, &QAction::triggered, this, &MainWindow::OnShowAbout);
	connect(_ui.ActionAboutQt, &QAction::triggered, QApplication::instance(), &QApplication::aboutQt);

	connect(_editorContext->GetRecentFiles(), &RecentFilesSettings::RecentFilesChanged, this, &MainWindow::OnRecentFilesChanged);

	connect(_undoGroup, &QUndoGroup::cleanChanged, this, &MainWindow::OnAssetCleanChanged);

	connect(_assetTabs, &QTabWidget::currentChanged, this, &MainWindow::OnAssetTabChanged);
	connect(_assetTabs, &QTabWidget::tabCloseRequested, this, &MainWindow::OnAssetTabCloseRequested);

	connect(_editorContext, &EditorContext::TryingToLoadAsset, this, &MainWindow::TryLoadAsset);
	connect(_editorContext, &EditorContext::SettingsChanged, this, &MainWindow::SyncSettings);
	connect(_editorContext->GetGameConfigurations(), &GameConfigurationsSettings::ActiveConfigurationChanged,
		this, &MainWindow::OnActiveConfigurationChanged);

	{
		const bool isSoundAvailable = _editorContext->GetSoundSystem()->IsSoundAvailable();

		_ui.ActionPlaySounds->setEnabled(isSoundAvailable);
		_ui.ActionFramerateAffectsPitch->setEnabled(isSoundAvailable);

		if (isSoundAvailable)
		{
			_ui.ActionPlaySounds->setChecked(_editorContext->GetGeneralSettings()->PlaySounds);
			_ui.ActionFramerateAffectsPitch->setChecked(_editorContext->GetGeneralSettings()->FramerateAffectsPitch);
		}
	}

	_ui.ActionSave->setEnabled(false);
	_ui.ActionSaveAs->setEnabled(false);
	_ui.ActionClose->setEnabled(false);
	_ui.MenuAsset->setEnabled(false);
	_assetTabs->setVisible(false);

	OnRecentFilesChanged();
	OnActiveConfigurationChanged(_editorContext->GetGameConfigurations()->GetActiveConfiguration(), {});

	setWindowTitle({});

	{
		//Construct the file filters used for loading and saving
		auto setupFileFilters = [this](ProviderFeature feature)
		{
			QStringList filters;

			for (auto provider : _editorContext->GetAssetProviderRegistry()->GetAssetProviders())
			{
				if (provider->GetFeatures() & feature)
				{
					auto fileTypes = provider->GetFileTypes();

					for (auto& fileType : fileTypes)
					{
						fileType = QString{"*.%1"}.arg(fileType);
					}

					filters.append(QString{"%1 Files (%2)"}.arg(provider->GetProviderName()).arg(fileTypes.join(' ')));
				}
			}

			QString fileFilters;

			if (!filters.isEmpty())
			{
				fileFilters = filters.join(";;");
			}

			if (!fileFilters.isEmpty())
			{
				fileFilters += ";;";
			}

			fileFilters += "All Files (*.*)";

			return fileFilters;
		};

		_loadFileFilter = setupFileFilters(ProviderFeature::AssetLoading);
		_saveFileFilter = setupFileFilters(ProviderFeature::AssetSaving);
	}

	// TODO: it might be easier to load settings after creating the main window and letting signals set this up.
	{
		auto textureLoader = _editorContext->GetTextureLoader();

		_ui.ActionPowerOf2Textures->setChecked(textureLoader->ShouldResizeToPowerOf2());
		_ui.MinFilterGroup->actions()[static_cast<int>(textureLoader->GetMinFilter())]->setChecked(true);
		_ui.MagFilterGroup->actions()[static_cast<int>(textureLoader->GetMagFilter())]->setChecked(true);
		_ui.MipmapFilterGroup->actions()[static_cast<int>(textureLoader->GetMipmapFilter())]->setChecked(true);
	}

	SyncSettings();

	_editorContext->StartTimer();
}

MainWindow::~MainWindow() = default;

void MainWindow::LoadSettings()
{
	auto settings = _editorContext->GetSettings();

	{
		settings->beginGroup("MainWindow");
		const auto screenName = settings->value("ScreenName");
		const auto geometry = settings->value("ScreenGeometry");
		settings->endGroup();

		//Calling this forces the creation of a QWindow handle now, instead of later
		winId();

		//Try to open the window on the screen it was last on
		if (screenName.isValid())
		{
			auto name = screenName.toString();

			for (auto screen : QApplication::screens())
			{
				if (screen->name() == name)
				{
					windowHandle()->setScreen(screen);
					break;
				}
			}
		}

		if (geometry.isValid())
		{
			restoreGeometry(geometry.toByteArray());
		}
	}
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	//If the user cancels any close request cancel the window close event as well
	for (int i = 0; i < _assetTabs->count(); ++i)
	{
		const auto asset = GetAsset(i);

		if (!VerifyNoUnsavedChanges(asset))
		{
			event->ignore();
			return;
		}
	}

	//Close each asset
	while (_assetTabs->count() > 0)
	{
		//Don't ask the user to save again
		TryCloseAsset(0, false);
	}

	event->accept();

	auto screen = this->windowHandle()->screen();

	auto name = screen->name();

	auto settings = _editorContext->GetSettings();

	settings->beginGroup("MainWindow");
	settings->setValue("ScreenName", name);
	settings->setValue("ScreenGeometry", saveGeometry());
	settings->endGroup();

	//Main window cleanup has to be done here because Qt won't call the destructor
	{
		_editorContext->GetTimer()->stop();

		delete _fileListDock;
		_fullscreenWidget.reset();
		_currentAsset.clear();
		delete _assetTabs;
	}
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event)
{
	if (watched == _assetTabs->tabBar())
	{
		if (event->type() == QEvent::Type::MouseButtonPress)
		{
			auto mouseEvent = static_cast<QMouseEvent*>(event);

			if (mouseEvent->button() == Qt::MouseButton::MiddleButton)
			{
				auto tab = _assetTabs->tabBar()->tabAt(mouseEvent->pos());

				if (tab != -1)
				{
					TryCloseAsset(tab, true);
					return true;
				}
			}
		}
	}

	return QMainWindow::eventFilter(watched, event);
}

Asset* MainWindow::GetAsset(int index) const
{
	return _assetTabs->widget(index)->property(TabWidgetAssetProperty.data()).value<Asset*>();
}

Asset* MainWindow::GetCurrentAsset() const
{
	return _currentAsset;
}

bool MainWindow::SaveAsset(Asset* asset)
{
	assert(asset);

	qCDebug(logging::HLAM) << "Trying to save asset" << asset->GetFileName();

	try
	{
		asset->Save();
	}
	catch (const AssetException& e)
	{
		QMessageBox::critical(this, "Error saving asset", QString{"Error saving asset:\n%1"}.arg(e.what()));
		return false;
	}

	auto undoStack = asset->GetUndoStack();

	undoStack->setClean();

	return true;
}

bool MainWindow::VerifyNoUnsavedChanges(Asset* asset)
{
	assert(asset);

	if (asset->GetUndoStack()->isClean())
	{
		return true;
	}

	const QMessageBox::StandardButton action = QMessageBox::question(
		this,
		{},
		QString{"Save changes made to \"%1\"?"}.arg(asset->GetFileName()),
		QMessageBox::StandardButton::Save | QMessageBox::StandardButton::Discard | QMessageBox::StandardButton::Cancel,
		QMessageBox::StandardButton::Save);

	switch (action)
	{
	case QMessageBox::StandardButton::Save: return SaveAsset(asset);
	case QMessageBox::StandardButton::Discard: return true;
	default:
	case QMessageBox::StandardButton::Cancel: return false;
	}
}

bool MainWindow::TryCloseAsset(int index, bool verifyUnsavedChanges)
{
	if (_fullscreenWidget)
	{
		//Always exit the fullscreen window if we're getting a close request
		//The user needs to be able to see the main window and interact with it,
		//and the fullscreen window may be holding a reference to the asset being closed
		_fullscreenWidget->ExitFullscreen();
	}

	{
		const auto asset = GetAsset(index);

		if (verifyUnsavedChanges && !VerifyNoUnsavedChanges(asset))
		{
			//User cancelled or an error occurred
			return false;
		}

		_assetTabs->removeTab(index);

		_undoGroup->removeStack(asset->GetUndoStack());

		asset->SetActive(false);

		delete asset;
	}

	return true;
}

void MainWindow::UpdateTitle(const QString& fileName, bool hasUnsavedChanges)
{
	setWindowTitle(QString{"%1[*]"}.arg(fileName));
	setWindowModified(hasUnsavedChanges);
}

bool MainWindow::TryLoadAsset(QString fileName)
{
	fileName = QDir::cleanPath(fileName);

	qCDebug(logging::HLAM) << "Trying to load asset" << fileName;

	if (!QFile::exists(fileName))
	{
		qCDebug(logging::HLAM) << "Asset" << fileName << "does not exist";
		QMessageBox::critical(this, "Error loading asset", QString{"Asset \"%1\" does not exist"}.arg(fileName));
		return false;
	}

	try
	{
		auto asset = _editorContext->GetAssetProviderRegistry()->Load(_editorContext, fileName);

		if (nullptr != asset)
		{
			auto currentFileName = asset->GetFileName();

			qCDebug(logging::HLAM) << "Asset" << fileName << "loaded as" << currentFileName;

			connect(asset.get(), &Asset::FileNameChanged, this, &MainWindow::OnAssetFileNameChanged);

			const auto editWidget = asset->GetEditWidget();

			editWidget->setProperty(TabWidgetAssetProperty.data(), QVariant::fromValue(asset.get()));

			_undoGroup->addStack(asset->GetUndoStack());

			//Now owned by this window
			asset->setParent(this);
			asset.release();

			//Use the current filename for this
			const auto index = _assetTabs->addTab(editWidget, currentFileName);

			_assetTabs->setCurrentIndex(index);

			_editorContext->GetRecentFiles()->Add(fileName);

			qCDebug(logging::HLAM) << "Loaded asset" << fileName;

			return true;
		}
		else
		{
			qCDebug(logging::HLAM) << "Asset" << fileName << "couldn't be loaded";
			QMessageBox::critical(this, "Error loading asset", QString{"Error loading asset \"%1\":\nNull asset returned"}.arg(fileName));
		}
	}
	catch (const AssetException& e)
	{
		QMessageBox::critical(this, "Error loading asset", QString{"Error loading asset \"%1\":\n%2"}.arg(fileName).arg(e.what()));
	}

	return false;
}

void MainWindow::SyncSettings()
{
	auto settings = _editorContext->GetSettings();

	if (settings->value("General/AllowTabCloseWithMiddleClick", GeneralSettings::DefaultAllowTabCloseWithMiddleClick).toBool())
	{
		_assetTabs->tabBar()->installEventFilter(this);
	}
	else
	{
		_assetTabs->tabBar()->removeEventFilter(this);
	}
}

void MainWindow::OnOpenLoadAssetDialog()
{
	if (const auto fileName = QFileDialog::getOpenFileName(this, "Select asset",
		_editorContext->GetPath(AssetPathName), _loadFileFilter);
		!fileName.isEmpty())
	{
		_editorContext->SetPath(AssetPathName, fileName);

		TryLoadAsset(fileName);
	}
}

void MainWindow::OnAssetCleanChanged(bool clean)
{
	setWindowModified(!clean);
}

void MainWindow::OnAssetTabChanged(int index)
{
	_ui.MenuAsset->clear();

	if (_currentAsset)
	{
		_currentAsset->SetActive(false);
	}

	_currentAsset = index != -1 ? GetAsset(index) : nullptr;

	bool success = false;

	if (index != -1)
	{
		_undoGroup->setActiveStack(_currentAsset->GetUndoStack());

		UpdateTitle(_currentAsset->GetFileName(), !_undoGroup->isClean());
		_currentAsset->PopulateAssetMenu(_ui.MenuAsset);

		_currentAsset->SetActive(true);

		success = true;
	}

	if (!success)
	{
		_undoGroup->setActiveStack(nullptr);
		setWindowTitle({});
	}

	emit _editorContext->ActiveAssetChanged(_currentAsset);

	_ui.ActionSave->setEnabled(success);
	_ui.ActionSaveAs->setEnabled(success);
	_ui.ActionClose->setEnabled(success);
	_ui.MenuAsset->setEnabled(success);
	_assetTabs->setVisible(success);
	_ui.ActionFullscreen->setEnabled(success);
	_ui.ActionRefresh->setEnabled(success);
}

void MainWindow::OnAssetTabCloseRequested(int index)
{
	TryCloseAsset(index, true);
}

void MainWindow::OnAssetFileNameChanged(const QString& fileName)
{
	auto asset = static_cast<Asset*>(sender());

	const int index = _assetTabs->indexOf(asset->GetEditWidget());

	if (index != -1)
	{
		_assetTabs->setTabText(index, fileName);

		_editorContext->GetRecentFiles()->Add(fileName);

		if (_assetTabs->currentWidget() == asset->GetEditWidget())
		{
			UpdateTitle(asset->GetFileName(), !_undoGroup->isClean());
		}
	}
	else
	{
		QMessageBox::critical(this, "Internal Error", "Asset index not found in assets tab widget");
	}
}

void MainWindow::OnSaveAsset()
{
	SaveAsset(GetCurrentAsset());
}

void MainWindow::OnSaveAssetAs()
{
	const auto asset = GetCurrentAsset();

	QString fileName{QFileDialog::getSaveFileName(this, {}, asset->GetFileName(), _saveFileFilter)};

	if (!fileName.isEmpty())
	{
		//Also update the saved path when saving files
		_editorContext->SetPath(AssetPathName, QFileInfo(fileName).absolutePath());
		asset->SetFileName(std::move(fileName));
		SaveAsset(asset);
	}
}

void MainWindow::OnCloseAsset()
{
	if (auto index = this->_assetTabs->currentIndex(); index != -1)
	{
		TryCloseAsset(index, true);
	}
}

void MainWindow::OnRecentFilesChanged()
{
	const auto recentFiles = _editorContext->GetRecentFiles();

	_ui.MenuRecentFiles->clear();

	for (int i = 0; i < recentFiles->GetCount(); ++i)
	{
		_ui.MenuRecentFiles->addAction(recentFiles->At(i), this, &MainWindow::OnOpenRecentFile);
	}

	_ui.MenuRecentFiles->setEnabled(recentFiles->GetCount() > 0);
}

void MainWindow::OnOpenRecentFile()
{
	const auto action = static_cast<QAction*>(sender());

	const QString fileName{action->text()};

	if (!TryLoadAsset(fileName))
	{
		_editorContext->GetRecentFiles()->Remove(fileName);
	}
}

void MainWindow::OnExit()
{
	this->close();
}

void MainWindow::OnGoFullscreen()
{
	if (!_fullscreenWidget)
	{
		//Note: creating this window as a child of the main window causes problems with OpenGL rendering
		//This must be created with no parent to function properly
		_fullscreenWidget = std::make_unique<FullscreenWidget>();
	}

	const auto asset = GetCurrentAsset();

	asset->SetupFullscreenWidget(_fullscreenWidget.get());

	_fullscreenWidget->raise();
	_fullscreenWidget->showFullScreen();
	_fullscreenWidget->activateWindow();
}

void MainWindow::OnFileSelected(const QString& fileName)
{
	TryLoadAsset(fileName);
}

void MainWindow::OnTextureFiltersChanged()
{
	const auto currentIndex = [](QActionGroup* group)
	{
		const int index = group->actions().indexOf(group->checkedAction());

		if (index == -1)
		{
			return 0;
		}

		return index;
	};

	_editorContext->GetGeneralSettings()->SetTextureFilters(
		static_cast<graphics::TextureFilter>(currentIndex(_ui.MinFilterGroup)),
		static_cast<graphics::TextureFilter>(currentIndex(_ui.MagFilterGroup)),
		static_cast<graphics::MipmapFilter>(currentIndex(_ui.MipmapFilterGroup)));
}

void MainWindow::OnRefreshAsset()
{
	if (auto asset = GetCurrentAsset(); asset)
	{
		if (!VerifyNoUnsavedChanges(asset))
		{
			//User canceled, abort refresh
			return;
		}

		asset->TryRefresh();
	}
}

void MainWindow::OnPlaySoundsChanged()
{
	_editorContext->GetGeneralSettings()->PlaySounds = _ui.ActionPlaySounds->isChecked();
}

void MainWindow::OnFramerateAffectsPitchChanged()
{
	_editorContext->GetGeneralSettings()->FramerateAffectsPitch = _ui.ActionFramerateAffectsPitch->isChecked();
}

void MainWindow::OnOpenOptionsDialog()
{
	OptionsDialog dialog{_editorContext, this};

	dialog.exec();
}

void MainWindow::OnShowAbout()
{
	const QString programName{QApplication::applicationName()};

	QString buildConfiguration;

#ifdef NDEBUG
	buildConfiguration = "Release";
#else
	buildConfiguration = "Debug";
#endif

	QMessageBox::about(this, "About " + programName,
		QString::fromUtf8(
			reinterpret_cast<const char*>(u8R"(%1 %2.%3.%4

2022 Sam Vanheer

Email:	sam.vanheer@outlook.com

Build Configuration: %5
Git Info:
	Branch: %6
	Tag: %7
	Commit Hash: %8

Based on Jed's Half-Life Model Viewer v1.3 © 2004 Neil 'Jed' Jedrzejewski
Email:	jed@wunderboy.org
Web:	http://www.wunderboy.org/

Also based on Half-Life Model Viewer v1.25 © 2002 Mete Ciragan
Email:	mete@swissquake.ch
Web:	http://www.milkshape3d.com/

This product contains software technology licensed from Id Software, Inc.
( "Id Technology" ). Id Technology © 1996 Id Software, Inc.
All Rights Reserved.

Copyright © 1998-2002, Valve LLC.
All rights reserved.

Uses OpenAL Soft
Uses Ogg Vorbis
Uses Libnyquist, Copyright (c) 2019, Dimitri Diakopoulos All rights reserved.
Uses The OpenGL Mathemathics library (GLM)
Copyright © 2005 - 2016 G-Truc Creation

Uses Qt %9

Build Date: %10
)"))
			.arg(programName)
			.arg(HLAMVersionMajor)
			.arg(HLAMVersionMinor)
			.arg(HLAMVersionPatch)
			.arg(buildConfiguration)
			.arg(QString::fromUtf8(HLAMGitBranchName.data()))
			.arg(QString::fromUtf8(HLAMGitTagName.data()))
			.arg(QString::fromUtf8(HLAMGitCommitHash.data()))
			.arg(QT_VERSION_STR)
			.arg(__DATE__)
	);
}

void MainWindow::SetupFileSystem(std::pair<GameEnvironment*, GameConfiguration*> activeConfiguration)
{
	auto fileSystem = _editorContext->GetFileSystem();

	fileSystem->RemoveAllSearchPaths();

	const auto environment = activeConfiguration.first;
	const auto configuration = activeConfiguration.second;
	const auto defaultGameConfiguration = environment->GetGameConfigurationById(environment->GetDefaultModId());

	fileSystem->SetBasePath(environment->GetInstallationPath().toStdString().c_str());

	const auto directoryExtensions{GetSteamPipeDirectoryExtensions()};

	const auto gameDir{defaultGameConfiguration->GetDirectory().toStdString()};
	const auto modDir{configuration->GetDirectory().toStdString()};

	//Add mod dirs first since they override game dirs
	if (gameDir != modDir)
	{
		for (const auto& extension : directoryExtensions)
		{
			fileSystem->AddSearchPath((modDir + extension).c_str());
		}
	}

	for (const auto& extension : directoryExtensions)
	{
		fileSystem->AddSearchPath((gameDir + extension).c_str());
	}
}

void MainWindow::OnActiveConfigurationChanged(std::pair<GameEnvironment*, GameConfiguration*> current,
	std::pair<GameEnvironment*, GameConfiguration*> previous)
{
	if (previous.second)
	{
		disconnect(previous.second, &GameConfiguration::DirectoryChanged, this, &MainWindow::OnGameConfigurationDirectoryChanged);
	}

	if (current.second)
	{
		connect(current.second, &GameConfiguration::DirectoryChanged, this, &MainWindow::OnGameConfigurationDirectoryChanged);

		SetupFileSystem(current);
	}
	else
	{
		auto fileSystem = _editorContext->GetFileSystem();

		fileSystem->RemoveAllSearchPaths();
	}
}

void MainWindow::OnGameConfigurationDirectoryChanged()
{
	SetupFileSystem(_editorContext->GetGameConfigurations()->GetActiveConfiguration());
}
