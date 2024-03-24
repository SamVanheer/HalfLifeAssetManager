#include <algorithm>
#include <cassert>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include <QActionGroup>
#include <QApplication>
#include <QCloseEvent>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QMenu>
#include <QMessageBox>
#include <QOpenGLFunctions>
#include <QScreen>
#include <QTabBar>
#include <QToolButton>
#include <QUndoGroup>
#include <QWidget>
#include <QWindow>
#include <QtPlatformHeaders/QWindowsWindowFunctions>

#include "application/AssetIO.hpp"
#include "application/AssetList.hpp"
#include "application/AssetManager.hpp"
#include "application/Assets.hpp"

#include "graphics/IGraphicsContext.hpp"
#include "graphics/TextureLoader.hpp"

#include "qt/QtLogging.hpp"
#include "qt/QtUtilities.hpp"

#include "settings/ApplicationSettings.hpp"
#include "settings/ExternalProgramSettings.hpp"
#include "settings/GameConfigurationsSettings.hpp"
#include "settings/RecentFilesSettings.hpp"

#include "soundsystem/ISoundSystem.hpp"

#include "ui/AboutDialog.hpp"
#include "ui/DragNDropEventFilter.hpp"
#include "ui/MainWindow.hpp"

#include "ui/dialogs/OpenInExternalProgramDialog.hpp"

#include "ui/dockpanels/FileBrowser.hpp"
#include "ui/dockpanels/MessagesPanel.hpp"

#include "ui/options/OptionsDialog.hpp"

#include "utility/Utility.hpp"

const QString AssetPathName{QStringLiteral("AssetPath")};

MainWindow::MainWindow(AssetManager* application)
	: QMainWindow()
	, _application(application)
	, _assets(_application->GetAssets())
	, _undoGroup(new QUndoGroup(this))
{
	_ui.setupUi(this);

	this->setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);

	//This has to be a native window for there to be a window handle
	setAttribute(Qt::WidgetAttribute::WA_NativeWindow, true);

	//Without this going fullscreen will cause black flickering
	QWindowsWindowFunctions::setHasBorderInFullScreen(this->windowHandle(), true);

	this->setWindowIcon(QIcon{":/hlam.ico"});

	this->installEventFilter(_application->GetDragNDropEventFilter());

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

		for (auto provider : _application->GetAssetProviderRegistry()->GetAssetProviders())
		{
			if (auto menu = provider->CreateToolMenu(); menu)
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

	// Add Windows to View menu.

	_ui.MenuView->addSeparator();

	{
		auto fileBrowser = new FileBrowser(_application, this);

		connect(fileBrowser, &FileBrowser::FilesSelected, this, &MainWindow::MaybeOpenAll);

		_fileBrowserDock = new QDockWidget(this);

		_fileBrowserDock->setWidget(fileBrowser);
		_fileBrowserDock->setWindowTitle(fileBrowser->windowTitle());

		this->addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, _fileBrowserDock);

		_fileBrowserDock->hide();

		_ui.MenuView->addAction(_fileBrowserDock->toggleViewAction());

		connect(_fileBrowserDock, &QDockWidget::visibilityChanged, [this, fileBrowser](bool visible)
			{
				if (visible)
				{
					fileBrowser->Initialize();
				}
			});
	}

	{
		auto messagesPanel = new MessagesPanel(_application, this);
		auto dock = new QDockWidget(this);

		dock->setWidget(messagesPanel);
		dock->setWindowTitle(messagesPanel->windowTitle());
		dock->setAllowedAreas(Qt::DockWidgetArea::BottomDockWidgetArea);
		dock->hide();

		connect(dock, &QDockWidget::dockLocationChanged, this, [dock] { dock->resize(400, 100); });

		addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, dock);

		_ui.MenuView->addAction(dock->toggleViewAction());
	}

	_assetsWidget = new QWidget(this);

	{
		auto sizePolicy = _assetsWidget->sizePolicy();
		sizePolicy.setRetainSizeWhenHidden(true);
		_assetsWidget->setSizePolicy(sizePolicy);
	}

	_assetsLayout = new QGridLayout(_assetsWidget);

	_assetsLayout->setSpacing(0);
	_assetsLayout->setContentsMargins(0, 0, 0, 0);
	_assetsLayout->setRowStretch(1, 1);

	_assetsWidget->setLayout(_assetsLayout);

	_assetTabs = new QTabBar(_assetsWidget);

	_assetsLayout->addWidget(_assetTabs, 0, 0);

	//Eliminate the border on the sides so the scene widget takes up all horizontal space
	_assetTabs->setDocumentMode(true);
	_assetTabs->setExpanding(false);
	_assetTabs->setDrawBase(true);
	_assetTabs->setTabsClosable(true);
	_assetTabs->setElideMode(Qt::TextElideMode::ElideLeft);

	_assetsWidget->setFocusProxy(_assetTabs);

	setCentralWidget(_assetsWidget);

	_assetListMenu = new QMenu(_assetsWidget);
	_assetListMenu->setStyleSheet("QMenu { menu-scrollable: 1; }");

	_assetListButton = new QToolButton(_assetsWidget);
	_assetListButton->setEnabled(false);
	_assetListButton->setMenu(_assetListMenu);
	_assetListButton->setPopupMode(QToolButton::InstantPopup);

	_assetsLayout->addWidget(_assetListButton, 0, 1);

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

		int index = _application->GetApplicationSettings()->GetMSAALevel();

		// Won't match the actual setting but this lets the user override the level manually.
		if (index < 0 || index >= _msaaActionGroup->actions().size())
		{
			index = 0;
		}

		_msaaActionGroup->actions()[index]->setChecked(true);
	}

	connect(_ui.ActionLoad, &QAction::triggered, this, &MainWindow::OnOpenLoadAssetDialog);
	connect(_ui.ActionSave, &QAction::triggered, this, &MainWindow::OnSaveAsset);
	connect(_ui.ActionSaveAs, &QAction::triggered, this, &MainWindow::OnSaveAssetAs);
	connect(_ui.ActionClose, &QAction::triggered, this, [this] { _assets->TryClose(_assetTabs->currentIndex(), true); });
	connect(_ui.ActionCloseAll, &QAction::triggered, this, [this] { CloseAllButCount(0, true); });
	connect(_ui.ActionExit, &QAction::triggered, this, &MainWindow::close);

	connect(_ui.ActionFullscreen, &QAction::triggered, this, &MainWindow::OnToggleFullscreen);

	connect(_ui.ActionPlaySounds, &QAction::triggered, this,
		[this](bool value) { _application->GetApplicationSettings()->PlaySounds = value; });

	connect(_ui.ActionFramerateAffectsPitch, &QAction::triggered, this,
		[this](bool value) { _application->GetApplicationSettings()->FramerateAffectsPitch = value; });

	connect(_ui.ActionPowerOf2Textures, &QAction::toggled,
		_application->GetApplicationSettings(), &ApplicationSettings::SetResizeTexturesToPowerOf2);

	connect(_ui.ActionWaitForVerticalSync, &QAction::toggled,
		_application->GetApplicationSettings(), &ApplicationSettings::SetEnableVSync);

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
			_application->GetApplicationSettings()->SetMSAALevel(index);
		};

		for (auto action : _msaaActionGroup->actions())
		{
			connect(action, &QAction::triggered, this, lambda);
		}
	}

	connect(_ui.ActionTransparentScreenshots, &QAction::triggered, this,
		[this](bool value) { _application->GetApplicationSettings()->TransparentScreenshots = value; });

	connect(_ui.ActionTakeScreenshot, &QAction::triggered, this, [this]()
		{
			auto asset = _assets->GetCurrent();

			if (!asset)
			{
				return;
			}

			asset->TakeScreenshot();
		});

	connect(_ui.ActionRefresh, &QAction::triggered, this, [this] { _assets->RefreshCurrent(); });

	connect(_ui.ActionOptions, &QAction::triggered, this, &MainWindow::OnOpenOptionsDialog);

	connect(_ui.ActionOpenManual, &QAction::triggered, this, [this]
		{
			const QString manualLocation{QApplication::applicationDirPath() + "/manual/HalfLifeAssetManagerManual.pdf"};
			qt::LaunchDefaultProgram(manualLocation);
		});
	connect(_ui.ActionAbout, &QAction::triggered, this, [this] { ShowAboutDialog(this); });
	connect(_ui.ActionAboutQt, &QAction::triggered, this, &QApplication::aboutQt);

	connect(_application->GetApplicationSettings()->GetRecentFiles(), &RecentFilesSettings::RecentFilesChanged,
		this, &MainWindow::OnRecentFilesChanged);

	connect(_undoGroup, &QUndoGroup::cleanChanged, this, [this](bool clean) { setWindowModified(!clean); });

	connect(_assetTabs, &QTabBar::currentChanged, this, &MainWindow::OnAssetTabChanged);
	connect(_assetTabs, &QTabBar::tabCloseRequested, this, [this](int index) { _assets->TryClose(index, true); });

	connect(_assets, &AssetList::AssetAdded, this, &MainWindow::OnAssetAdded);
	connect(_assets, &AssetList::AboutToCloseAsset, this, &MainWindow::OnAboutToCloseAsset);
	connect(_assets, &AssetList::AboutToRemoveAsset, this, &MainWindow::OnAboutToRemoveAsset);
	connect(_assets, &AssetList::AssetRemoved, this, &MainWindow::OnAssetRemoved);
	connect(_assets, &AssetList::ActiveAssetChanged, this,
		[this](Asset* currentAsset)
		{
			_assetTabs->setCurrentIndex(_assets->IndexOf(currentAsset));
		});

	connect(_application, &AssetManager::SettingsChanged, this, &MainWindow::SyncSettings);

	{
		const bool isSoundAvailable = _application->GetSoundSystem()->IsSoundAvailable();

		_ui.ActionPlaySounds->setEnabled(isSoundAvailable);
		_ui.ActionFramerateAffectsPitch->setEnabled(isSoundAvailable);

		if (isSoundAvailable)
		{
			_ui.ActionPlaySounds->setChecked(_application->GetApplicationSettings()->PlaySounds);
			_ui.ActionFramerateAffectsPitch->setChecked(_application->GetApplicationSettings()->FramerateAffectsPitch);
		}
	}

	_assetsWidget->setVisible(false);

	OnRecentFilesChanged();

	setWindowTitle({});

	{
		//Construct the file filters used for loading and saving
		auto setupFileFilters = [this](ProviderFeature feature)
		{
			QStringList filters;

			for (auto provider : _application->GetAssetProviderRegistry()->GetAssetProviders())
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
		auto textureLoader = _application->GetTextureLoader();

		const auto graphicsContext = _application->GetGraphicsContext();

		graphicsContext->Begin();

		QOpenGLFunctions functions;

		functions.initializeOpenGLFunctions();

		if (!functions.hasOpenGLFeature(QOpenGLFunctions::OpenGLFeature::NPOTTextures))
		{
			textureLoader->SetResizeToPowerOf2(true);
			_ui.ActionPowerOf2Textures->setEnabled(false);
		}

		// Transparent screenshots depend on framebuffers.
		if (!functions.hasOpenGLFeature(QOpenGLFunctions::OpenGLFeature::Framebuffers))
		{
			_application->GetApplicationSettings()->TransparentScreenshots = false;
			_ui.ActionTransparentScreenshots->setEnabled(false);
		}

		graphicsContext->End();

		_ui.ActionPowerOf2Textures->setChecked(textureLoader->ShouldResizeToPowerOf2());
		_ui.ActionWaitForVerticalSync->setChecked(_application->GetApplicationSettings()->ShouldEnableVSync());
		_ui.MinFilterGroup->actions()[static_cast<int>(textureLoader->GetMinFilter())]->setChecked(true);
		_ui.MagFilterGroup->actions()[static_cast<int>(textureLoader->GetMagFilter())]->setChecked(true);
		_ui.MipmapFilterGroup->actions()[static_cast<int>(textureLoader->GetMipmapFilter())]->setChecked(true);
		_ui.ActionTransparentScreenshots->setChecked(_application->GetApplicationSettings()->TransparentScreenshots);
	}

	SyncSettings();
}

MainWindow::~MainWindow()
{
	auto screen = this->windowHandle()->screen();
	auto settings = _application->GetSettings();

	settings->beginGroup("MainWindow");
	settings->setValue("ScreenName", screen->name());
	settings->setValue("ScreenGeometry", saveGeometry());
	settings->endGroup();

	_application->OnMainWindowClosing();
}

void MainWindow::LoadSettings()
{
	auto settings = _application->GetSettings();

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

	// In case the program was in fullscreen mode when saved, restore to maximized instead.
	ExitFullscreen();
}

void MainWindow::LoadFile(const QString& fileName)
{
	if (isMaximized())
	{
		showMaximized();
	}
	else
	{
		showNormal();
	}

	activateWindow();

	MaybeOpenAll(QStringList{fileName});
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	// If the user is in fullscreen mode force them out of it.
	ExitFullscreen();

	//If the user cancels any close request cancel the window close event as well
	for (int i = 0; i < _assetTabs->count(); ++i)
	{
		const auto asset = _assets->Get(i);

		if (!_assets->VerifyNoUnsavedChanges(asset, true))
		{
			event->ignore();
			return;
		}
	}

	// Close each asset
	// Don't ask the user to save again
	CloseAllButCount(0, false);

	event->accept();
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event)
{
	if (watched == _assetTabs)
	{
		if (event->type() == QEvent::Type::MouseButtonPress)
		{
			auto mouseEvent = static_cast<QMouseEvent*>(event);

			if (mouseEvent->button() == Qt::MouseButton::MiddleButton)
			{
				auto tab = _assetTabs->tabAt(mouseEvent->pos());

				if (tab != -1)
				{
					_assets->TryClose(tab, true);
					return true;
				}
			}
		}
	}

	return QMainWindow::eventFilter(watched, event);
}

void MainWindow::UpdateTitle(const QString& fileName, bool hasUnsavedChanges)
{
	setWindowTitle(QString{"%1[*]"}.arg(fileName));
	setWindowModified(hasUnsavedChanges);
}

void MainWindow::MaybeOpenAll(const QStringList& fileNames)
{
	if (fileNames.isEmpty())
	{
		return;
	}

	// Set directory to first file. All files are in the same directory.
	_application->SetPath(AssetPathName, fileNames[0]);

	std::vector<ExternalProgramCommand> filesToLoadInExternalPrograms;

	{
		const auto externalPrograms = _application->GetApplicationSettings()->GetExternalPrograms();
		const std::shared_ptr<spdlog::logger> logger = _application->GetLogger();

		const TimerSuspender timerSuspender{_application};

		// Make the tab widget invisible to reduce overhead from updating it.
		_assetsWidget->setVisible(false);
		_modifyingTabs = true;

		for (int i = 0; const auto & fileName : fileNames)
		{
			auto loadResult = _assets->TryLoad(fileName);

			std::visit([&](auto&& result)
				{
					using T = std::decay_t<decltype(result)>;

					if constexpr (std::is_same_v<T, AssetLoadAction>)
					{
						if (result == AssetLoadAction::Success)
						{
							_activateNewTabs = false;
						}
					}
					else if constexpr (std::is_same_v<T, AssetLoadInExternalProgram>)
					{
						filesToLoadInExternalPrograms.emplace_back(
							fileName, std::move(result.ExternalProgramKey), result.PromptBeforeOpening);
					}
					else
					{
						static_assert(always_false_v<T>, "Unhandled Asset load return type");
					}
				}, loadResult);
		}

		_activateNewTabs = true;

		_modifyingTabs = false;
		_assetsWidget->setVisible(_assetTabs->count() > 0);
	}

	// Use the simplified dialog when there's only one.
	switch (filesToLoadInExternalPrograms.size())
	{
	case 0U: break;
	case 1U:
	{
		const auto& file = filesToLoadInExternalPrograms.front();
		TryLoadInExternalProgram(file.FileName, file.ExternalProgramKey, file.PromptBeforeOpening);
		break;
	}

	default:
	{
		OpenInExternalProgramDialog dialog{_application, this, filesToLoadInExternalPrograms};
		dialog.exec();
		break;
	}
	}
}

void MainWindow::TryLoadInExternalProgram(const QString& fileName, const QString& externalProgramKey, bool promptBeforeOpening)
{
	const auto externalPrograms = _application->GetApplicationSettings()->GetExternalPrograms();

	const auto programName = externalPrograms->GetName(externalProgramKey);

	const auto launchResult = _application->TryLaunchExternalProgram(
		externalProgramKey, QStringList(fileName),
		promptBeforeOpening ? std::optional{ QString{"This file has to be opened in %1."}.arg(programName) } : std::nullopt);

	switch (launchResult)
	{
	case LaunchExternalProgramResult::Success:
		_application->GetApplicationSettings()->GetRecentFiles()->Add(fileName);
		break;

	case LaunchExternalProgramResult::Failed:
		_application->GetLogger()->error(R"(Error loading asset "{0}":
File "{0}" has to be opened in {1}.
Set the {1} executable setting to open the file through that program instead.)", fileName, programName);
		break;
	}
}

void MainWindow::CloseAllButCount(int leaveOpenCount, bool verifyUnsavedChanges)
{
	assert(leaveOpenCount >= 0);

	int count = _assets->Count();

	if (count <= leaveOpenCount)
	{
		return;
	}

	const TimerSuspender timerSuspender{_application};

	// Switch to the first asset to reduce the overhead involved with constant tab switching.
	_assets->SetCurrent(_assets->Get(0));

	// Make the tab widget invisible to reduce overhead from updating it.
	_assetsWidget->setVisible(false);
	_modifyingTabs = true;

	while (count-- > leaveOpenCount)
	{
		// Never allow cancel, otherwise we'll end up in an infinite loop.
		_assets->TryClose(count, verifyUnsavedChanges, false);
	}

	_modifyingTabs = false;
	_assetsWidget->setVisible(_assetTabs->count() > 0);
}

void MainWindow::SyncSettings()
{
	if (_application->GetApplicationSettings()->ShouldAllowTabCloseWithMiddleClick())
	{
		_assetTabs->installEventFilter(this);
	}
	else
	{
		_assetTabs->removeEventFilter(this);
	}

	if (_application->GetApplicationSettings()->OneAssetAtATime)
	{
		CloseAllButCount(1, true);
	}
}

static QStringList GetOpenFileNames(QWidget* parent, const QString& dir, const QString& filter, bool allowMultiple)
{
	if (allowMultiple)
	{
		return QFileDialog::getOpenFileNames(parent, "Select asset", dir, filter);
	}

	return QStringList{} << QFileDialog::getOpenFileName(parent, "Select asset", dir, filter);
}

void MainWindow::OnOpenLoadAssetDialog()
{
	const auto fileNames = GetOpenFileNames(this, _application->GetPath(AssetPathName), _loadFileFilter,
		!_application->GetApplicationSettings()->OneAssetAtATime);

	MaybeOpenAll(fileNames);
}

void MainWindow::OnAssetTabChanged(int index)
{
	if (_assetMenu)
	{
		_assetMenu->menuAction()->setVisible(false);
	}

	const auto currentAsset = index != -1 ? _assets->Get(index) : nullptr;
	const bool success = currentAsset != nullptr;

	if (success)
	{
		_undoGroup->setActiveStack(currentAsset->GetUndoStack());

		UpdateTitle(currentAsset->GetFileName(), !_undoGroup->isClean());

		auto provider = currentAsset->GetProvider();

		auto menuIt = _assetMenus.find(provider);

		// Lazily create menus to avoid creating entire windows on startup.
		if (menuIt == _assetMenus.end())
		{
			auto menu = new QMenu("Asset", _ui.MenuBar);

			provider->PopulateAssetMenu(menu);

			menuIt = _assetMenus.insert(provider, menu);
			_ui.MenuBar->insertMenu(_ui.MenuTools->menuAction(), menu);
			menu->menuAction()->setVisible(false);
		}

		_assetMenu = *menuIt;
		_assetMenu->menuAction()->setVisible(true);
	}
	else
	{
		_undoGroup->setActiveStack(nullptr);
		setWindowTitle({});
		ExitFullscreen();
	}

	_ui.ActionSave->setEnabled(success);
	_ui.ActionSaveAs->setEnabled(success);
	_ui.ActionClose->setEnabled(success);
	_ui.ActionCloseAll->setEnabled(success);
	_ui.ActionFullscreen->setEnabled(success);
	_ui.ActionRefresh->setEnabled(success);
	_ui.ActionTakeScreenshot->setEnabled(currentAsset != nullptr && currentAsset->CanTakeScreenshot());
	_assetListButton->setEnabled(success);

	if (index != -1)
	{
		_assetListMenu->setDefaultAction(_assetListMenu->actions()[index]);
	}

	if (!_modifyingTabs)
	{
		_assetsWidget->setVisible(success);
	}

	if (auto previousAsset = _assets->GetCurrent(); previousAsset)
	{
		disconnect(previousAsset, &Asset::EditWidgetChanged, this, &MainWindow::UpdateAssetWidget);
	}

	_assets->SetCurrent(currentAsset);

	if (currentAsset)
	{
		connect(currentAsset, &Asset::EditWidgetChanged, this, &MainWindow::UpdateAssetWidget);
	}

	UpdateAssetWidget();
}

void MainWindow::UpdateAssetWidget()
{
	const auto currentAsset = _assets->GetCurrent();

	QWidget* const nextEditWidget = currentAsset ? currentAsset->GetEditWidget() : nullptr;

	if (_currentEditWidget != nextEditWidget)
	{
		_assetsLayout->removeWidget(_currentEditWidget);

		if (nextEditWidget)
		{
			nextEditWidget->setParent(_assetsWidget);
			_assetsLayout->addWidget(nextEditWidget, 1, 0, 1, 2);
			nextEditWidget->show();
		}

		_currentEditWidget = nextEditWidget;
	}
}

void MainWindow::OnAssetAdded(int index)
{
	auto asset = _assets->Get(index);

	connect(asset, &Asset::FileNameChanged, this, &MainWindow::OnAssetFileNameChanged);

	_undoGroup->addStack(asset->GetUndoStack());

	const QString fileName = asset->GetFileName();

	// Add the action before adding the tab so OnAssetTabChanged references the right action.
	_assetListMenu->addAction(fileName, this, &MainWindow::OnAssetActivated);

	const auto tabIndex = _assetTabs->addTab(fileName);

	assert(tabIndex == index);

	if (_activateNewTabs)
	{
		_assetTabs->setCurrentIndex(index);
	}
}

void MainWindow::OnAssetActivated()
{
	const auto action = static_cast<QAction*>(sender());
	const int index = _assetListMenu->actions().indexOf(action);
	_assetTabs->setCurrentIndex(index);
}

void MainWindow::OnAboutToCloseAsset(int index)
{
	// Exit the fullscreen window if we're getting a close request
	// The user needs to be able to see the main window and interact with it
	// If the window isn't fullscreen then the user can easily open the program window
	ExitFullscreen();
}

void MainWindow::OnAboutToRemoveAsset(int index)
{
	auto asset = _assets->Get(index);
	_undoGroup->removeStack(asset->GetUndoStack());
}

void MainWindow::OnAssetRemoved(int index)
{
	_assetTabs->removeTab(index);
	_assetListMenu->removeAction(_assetListMenu->actions()[index]);
}

void MainWindow::OnAssetFileNameChanged(Asset* asset)
{
	const QString fileName = asset->GetFileName();
	const int index = _assets->IndexOf(asset);

	_assetTabs->setTabText(index, asset->GetFileName());

	const auto action = _assetListMenu->actions()[index];
	action->setText(fileName);

	if (_assetTabs->currentIndex() == index)
	{
		UpdateTitle(asset->GetFileName(), !_undoGroup->isClean());
	}
}

QString MainWindow::GetCanonicalFileName(Asset* asset) const
{
	const auto provider = asset->GetProvider();

	const auto validExtensions = provider->GetFileTypes();

	QString fileName{asset->GetFileName()};

	const QFileInfo info{fileName};

	if (!validExtensions.contains(info.suffix()))
	{
		fileName = QString{"%1/%2.%3"}
		.arg(info.absolutePath(), info.completeBaseName(), provider->GetPreferredFileType());
	}

	return fileName;
}

bool MainWindow::ShouldContinueWithSave(Asset* asset)
{
	const auto fileName = asset->GetFileName();

	for (int i = 0; i < _assets->Count(); ++i)
	{
		const auto other = _assets->Get(i);

		if (asset == other)
		{
			continue;
		}

		if (fileName.compare(other->GetFileName(), Qt::CaseInsensitive) == 0)
		{
			const auto action = QMessageBox::question(this, "Resolve asset conflicts",
				R"(Attempting to save an asset with the same filename as another asset that is already open.
Discard the other asset and any changes that have been made to it or cancel this operation?)",
				QMessageBox::Discard | QMessageBox::Cancel);

			if (action != QMessageBox::Discard)
			{
				return false;
			}

			_assets->TryClose(i, false, false);
		}
	}

	return true;
}

void MainWindow::OnSaveAsset()
{
	auto asset = _assets->GetCurrent();

	if (auto canonicalFileName = GetCanonicalFileName(asset); canonicalFileName != asset->GetFileName())
	{
		_application->GetLogger()->info("Asset \"{}\" automatically renamed to \"{}\" due to unsupported extension",
			asset->GetFileName(), canonicalFileName);
		asset->SetFileName(std::move(canonicalFileName));
	}

	if (!ShouldContinueWithSave(asset))
	{
		return;
	}

	_assets->Save(asset);
}

void MainWindow::OnSaveAssetAs()
{
	const auto asset = _assets->GetCurrent();

	QString fileName = GetCanonicalFileName(asset);

	fileName = QFileDialog::getSaveFileName(this, {}, fileName, _saveFileFilter);

	if (!fileName.isEmpty())
	{
		if (!ShouldContinueWithSave(asset))
		{
			return;
		}

		//Also update the saved path when saving files
		_application->SetPath(AssetPathName, QFileInfo(fileName).absolutePath());
		asset->SetFileName(std::move(fileName));
		_assets->Save(asset);
	}
}

void MainWindow::OnRecentFilesChanged()
{
	const auto recentFiles = _application->GetApplicationSettings()->GetRecentFiles();

	_ui.MenuRecentFiles->clear();
	_ui.MenuRecentFiles->setEnabled(recentFiles->GetCount() > 0);

	for (int i = 0; i < recentFiles->GetCount(); ++i)
	{
		_ui.MenuRecentFiles->addAction(recentFiles->At(i), this, &MainWindow::OnOpenRecentFile);
	}
}

void MainWindow::OnOpenRecentFile()
{
	const auto action = static_cast<QAction*>(sender());
	MaybeOpenAll(QStringList{action->text()});
}

void MainWindow::OnToggleFullscreen()
{
	const bool goFullscreen = !isFullScreen();

	if (goFullscreen)
	{
		// Create shortcuts for actions so they still work when the menu bar is hidden.
		// Qt Creator passes the window as the parent for menus and actions
		// so we need to search for children of the window instead of the menu bar.
		for (auto action : this->findChildren<QAction*>())
		{
			auto keySequence = action->shortcut();

			if (!keySequence.isEmpty())
			{
				auto shortcut = std::make_unique<QShortcut>(keySequence, this);

				connect(shortcut.get(), &QShortcut::activated, action, &QAction::trigger);

				_fullscreenShortcuts.push_back(std::move(shortcut));
			}
		}

		_cachedWindowState = windowState() & ~(Qt::WindowMinimized | Qt::WindowFullScreen);

		setWindowState((windowState() & ~(Qt::WindowMinimized | Qt::WindowMaximized)) | Qt::WindowFullScreen);
	}
	else
	{
		// Need to clear the state first to force a proper change.
		setWindowState(Qt::WindowNoState);
		setWindowState(_cachedWindowState);

		_fullscreenShortcuts.clear();
	}

	_ui.MenuBar->setVisible(!goFullscreen);
	_assetTabs->setVisible(!goFullscreen);
	this->_assetListButton->setVisible(!goFullscreen);

	emit _application->FullscreenModeChanged();
}

void MainWindow::ExitFullscreen()
{
	if (isFullScreen())
	{
		_ui.ActionFullscreen->trigger();
	}
}

void MainWindow::OnTextureFiltersChanged()
{
	const auto currentIndex = [](QActionGroup* group)
	{
		return group->actions().indexOf(group->checkedAction());
	};

	_application->GetApplicationSettings()->SetTextureFilters(
		static_cast<graphics::TextureFilter>(currentIndex(_ui.MinFilterGroup)),
		static_cast<graphics::TextureFilter>(currentIndex(_ui.MagFilterGroup)),
		static_cast<graphics::MipmapFilter>(currentIndex(_ui.MipmapFilterGroup)));
}

void MainWindow::OnOpenOptionsDialog()
{
	OptionsDialog dialog{_application, this};
	dialog.exec();
}
