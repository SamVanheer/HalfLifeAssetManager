#include <algorithm>
#include <cassert>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include <QActionGroup>
#include <QApplication>
#include <QCloseEvent>
#include <QDir>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QOpenGLFunctions>
#include <QProgressDialog>
#include <QScreen>
#include <QToolButton>
#include <QWindow>

#include "application/AssetIO.hpp"
#include "application/AssetList.hpp"
#include "application/AssetManager.hpp"
#include "application/Assets.hpp"

#include "graphics/IGraphicsContext.hpp"
#include "graphics/TextureLoader.hpp"

#include "qt/QtLogging.hpp"
#include "qt/QtUtilities.hpp"

#include "settings/ApplicationSettings.hpp"
#include "settings/GameConfigurationsSettings.hpp"
#include "settings/RecentFilesSettings.hpp"

#include "soundsystem/ISoundSystem.hpp"

#include "ui/AboutDialog.hpp"
#include "ui/DragNDropEventFilter.hpp"
#include "ui/FullscreenWidget.hpp"
#include "ui/MainWindow.hpp"
#include "ui/SceneWidget.hpp"

#include "ui/dockpanels/FileBrowser.hpp"
#include "ui/dockpanels/MessagesPanel.hpp"

#include "ui/options/OptionsDialog.hpp"

const QString AssetPathName{QStringLiteral("AssetPath")};

MainWindow::MainWindow(AssetManager* application)
	: QMainWindow()
	, _application(application)
	, _assets(_application->GetAssets())
{
	_ui.setupUi(this);

	this->setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);

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

	// Create and add asset menus for each provider.
	for (auto provider : _application->GetAssetProviderRegistry()->GetAssetProviders())
	{
		auto menu = new QMenu("Asset", _ui.MenuBar);

		provider->PopulateAssetMenu(menu);

		_assetMenus.insert(provider, menu);
		_ui.MenuBar->insertMenu(_ui.MenuTools->menuAction(), menu);
		menu->menuAction()->setVisible(false);
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

	_assetTabs = new QTabWidget(this);

	{
		auto sizePolicy = _assetTabs->sizePolicy();
		sizePolicy.setRetainSizeWhenHidden(true);
		_assetTabs->setSizePolicy(sizePolicy);
	}

	//Eliminate the border on the sides so the scene widget takes up all horizontal space
	_assetTabs->setDocumentMode(true);
	_assetTabs->setTabsClosable(true);
	_assetTabs->setElideMode(Qt::TextElideMode::ElideLeft);

	setCentralWidget(_assetTabs);

	_assetListMenu = new QMenu(_assetTabs);
	_assetListMenu->setStyleSheet("QMenu { menu-scrollable: 1; }");

	_assetListButton = new QToolButton(_assetTabs);
	_assetListButton->setEnabled(false);
	_assetListButton->setMenu(_assetListMenu);
	_assetListButton->setPopupMode(QToolButton::InstantPopup);

	_assetTabs->setCornerWidget(_assetListButton, Qt::TopRightCorner);

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

	connect(_ui.ActionFullscreen, &QAction::triggered, _application, &AssetManager::ToggleFullscreen);

	connect(_application, &AssetManager::FullscreenWidgetChanged, this,
		[this] { _ui.ActionFullscreen->setChecked(_application->GetFullscreenWidget() != nullptr); });

	connect(_ui.ActionPlaySounds, &QAction::triggered, this,
		[this](bool value) { _application->GetApplicationSettings()->PlaySounds = value; });

	connect(_ui.ActionFramerateAffectsPitch, &QAction::triggered, this,
		[this](bool value) { _application->GetApplicationSettings()->FramerateAffectsPitch = value; });

	connect(_ui.ActionPowerOf2Textures, &QAction::toggled,
		_application->GetApplicationSettings(), &ApplicationSettings::SetResizeTexturesToPowerOf2);

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

	connect(_ui.ActionRefresh, &QAction::triggered, this, [this] { _assets->RefreshCurrent(); });

	connect(_ui.ActionOptions, &QAction::triggered, this, &MainWindow::OnOpenOptionsDialog);

	connect(_ui.ActionOpenManual, &QAction::triggered, this, [this]
		{
			const QString manualLocation{QApplication::applicationDirPath() + "/../manual/HalfLifeAssetManagerManual.pdf"};
			qt::LaunchDefaultProgram(manualLocation);
		});
	connect(_ui.ActionAbout, &QAction::triggered, this, [this] { ShowAboutDialog(this); });
	connect(_ui.ActionAboutQt, &QAction::triggered, this, &QApplication::aboutQt);

	connect(_application->GetApplicationSettings()->GetRecentFiles(), &RecentFilesSettings::RecentFilesChanged,
		this, &MainWindow::OnRecentFilesChanged);

	connect(_undoGroup, &QUndoGroup::cleanChanged, this, [this](bool clean) { setWindowModified(!clean); });

	connect(_assetTabs, &QTabWidget::currentChanged, this, &MainWindow::OnAssetTabChanged);
	connect(_assetTabs, &QTabWidget::tabCloseRequested, this, [this](int index) { _assets->TryClose(index, true); });

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

	_assetTabs->setVisible(false);

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

		graphicsContext->End();

		_ui.ActionPowerOf2Textures->setChecked(textureLoader->ShouldResizeToPowerOf2());
		_ui.MinFilterGroup->actions()[static_cast<int>(textureLoader->GetMinFilter())]->setChecked(true);
		_ui.MagFilterGroup->actions()[static_cast<int>(textureLoader->GetMagFilter())]->setChecked(true);
		_ui.MipmapFilterGroup->actions()[static_cast<int>(textureLoader->GetMipmapFilter())]->setChecked(true);
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
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	// If the user is in fullscreen mode force them out of it.
	_application->ExitFullscreen();

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

	const TimerSuspender timerSuspender{_application};

	QProgressDialog progress(
		QString{"Opening %1 assets..."}.arg(fileNames.size()), "Abort Open", 0, fileNames.size(), this);
	progress.setWindowModality(Qt::WindowModal);

	// Always show immediately for 10 or more, wait only a second otherwise for slow loading assets.
	progress.setMinimumDuration(fileNames.size() < 10 ? 1000 : 0);

	// Make the tab widget invisible to reduce overhead from updating it.
	_assetTabs->setVisible(false);
	_modifyingTabs = true;

	for (int i = 0; const auto & fileName : fileNames)
	{
		progress.setValue(i++);

		if (progress.wasCanceled())
			break;

		if (_assets->TryLoad(fileName) == AssetLoadResult::Success)
		{
			_activateNewTabs = false;
		}
	}

	_activateNewTabs = true;

	progress.setValue(fileNames.size());

	_modifyingTabs = false;
	_assetTabs->setVisible(_assetTabs->count() > 0);
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

	QProgressDialog progress(QString{"Closing %1 assets..."}.arg(count), {}, 0, count - leaveOpenCount, this);
	progress.setWindowModality(Qt::WindowModal);

	// Switch to the first asset to reduce the overhead involved with constant tab switching.
	_assets->SetCurrent(_assets->Get(0));

	// Make the tab widget invisible to reduce overhead from updating it.
	_assetTabs->setVisible(false);
	_modifyingTabs = true;

	int i;

	for (i = 0; count-- > leaveOpenCount; ++i)
	{
		progress.setValue(i);

		// Never allow cancel, otherwise we'll end up in an infinite loop.
		_assets->TryClose(count, verifyUnsavedChanges, false);
	}

	progress.setValue(i);

	_modifyingTabs = false;
	_assetTabs->setVisible(_assetTabs->count() > 0);
}

void MainWindow::SyncSettings()
{
	if (_application->GetApplicationSettings()->ShouldAllowTabCloseWithMiddleClick())
	{
		_assetTabs->tabBar()->installEventFilter(this);
	}
	else
	{
		_assetTabs->tabBar()->removeEventFilter(this);
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

		if (auto menu = _assetMenus.find(currentAsset->GetProvider()); menu != _assetMenus.end())
		{
			_assetMenu = *menu;
			_assetMenu->menuAction()->setVisible(true);
		}
		else
		{
			_assetMenu = nullptr;
		}
	}
	else
	{
		_undoGroup->setActiveStack(nullptr);
		setWindowTitle({});
		_application->ExitFullscreen();
	}

	_ui.ActionSave->setEnabled(success);
	_ui.ActionSaveAs->setEnabled(success);
	_ui.ActionClose->setEnabled(success);
	_ui.ActionCloseAll->setEnabled(success);
	_ui.ActionFullscreen->setEnabled(success);
	_ui.ActionRefresh->setEnabled(success);
	_assetListButton->setEnabled(success);

	if (index != -1)
	{
		_assetListMenu->setDefaultAction(_assetListMenu->actions()[index]);
	}

	if (!_modifyingTabs)
	{
		_assetTabs->setVisible(success);
	}

	_assets->SetCurrent(currentAsset);
}

void MainWindow::OnAssetAdded(int index)
{
	auto asset = _assets->Get(index);

	connect(asset, &Asset::FileNameChanged, this, &MainWindow::OnAssetFileNameChanged);

	_undoGroup->addStack(asset->GetUndoStack());

	const QString fileName = asset->GetFileName();

	// Add the action before adding the tab so OnAssetTabChanged references the right action.
	_assetListMenu->addAction(fileName, this, &MainWindow::OnAssetActivated);

	const auto tabIndex = _assetTabs->addTab(asset->GetEditWidget(), fileName);

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
	if (_fullscreenWidget && _fullscreenWidget->isFullScreen())
	{
		// Exit the fullscreen window if we're getting a close request
		// The user needs to be able to see the main window and interact with it
		// If the window isn't fullscreen then the user can easily open the program window
		_fullscreenWidget->ExitFullscreen();
	}
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

	if (_assetTabs->currentWidget() == asset->GetEditWidget())
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
	_assets->TryLoad(action->text());
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
