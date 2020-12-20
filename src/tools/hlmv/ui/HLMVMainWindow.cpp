#include <algorithm>

#include <QApplication>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>

#include "assets/AssetIO.hpp"

#include "filesystem/IFileSystem.hpp"

#include "ui/Credits.hpp"
#include "ui/EditorContext.hpp"
#include "ui/FullscreenWidget.hpp"
#include "ui/HLMVMainWindow.hpp"

#include "ui/assets/Assets.hpp"

#include "ui/options/OptionsDialog.hpp"

#include "ui/settings/GameConfigurationsSettings.hpp"
#include "ui/settings/RecentFilesSettings.hpp"

namespace ui
{
HLMVMainWindow::HLMVMainWindow(EditorContext* editorContext)
	: QMainWindow()
	, _editorContext(editorContext)
{
	_ui.setupUi(this);

	{
		auto undo = _undoGroup->createUndoAction(this);
		auto redo = _undoGroup->createRedoAction(this);

		undo->setShortcut(QKeySequence::StandardKey::Undo);
		redo->setShortcut(QKeySequence::StandardKey::Redo);

		_ui.MenuEdit->addAction(undo);
		_ui.MenuEdit->addAction(redo);
	}

	_assetTabs = new QTabWidget(this);

	//Eliminate the border on the sides so the scene widget takes up all horizontal space
	_assetTabs->setDocumentMode(true);

	_assetTabs->setTabsClosable(true);

	_assetTabs->setElideMode(Qt::TextElideMode::ElideLeft);

	setCentralWidget(_assetTabs);

	connect(_ui.ActionLoad, &QAction::triggered, this, &HLMVMainWindow::OnOpenLoadAssetDialog);
	connect(_ui.ActionSave, &QAction::triggered, this, &HLMVMainWindow::OnSaveAsset);
	connect(_ui.ActionSaveAs, &QAction::triggered, this, &HLMVMainWindow::OnSaveAssetAs);
	connect(_ui.ActionExit, &QAction::triggered, this, &HLMVMainWindow::OnExit);
	connect(_ui.ActionFullscreen, &QAction::triggered, this, &HLMVMainWindow::OnGoFullscreen);
	connect(_ui.ActionOptions, &QAction::triggered, this, &HLMVMainWindow::OnOpenOptionsDialog);
	connect(_ui.ActionAbout, &QAction::triggered, this, &HLMVMainWindow::OnShowAbout);

	connect(_editorContext->GetRecentFiles(), &settings::RecentFilesSettings::RecentFilesChanged, this, &HLMVMainWindow::OnRecentFilesChanged);

	connect(_undoGroup, &QUndoGroup::cleanChanged, this, &HLMVMainWindow::OnAssetCleanChanged);

	connect(_assetTabs, &QTabWidget::currentChanged, this, &HLMVMainWindow::OnAssetTabChanged);
	connect(_assetTabs, &QTabWidget::tabCloseRequested, this, &HLMVMainWindow::OnAssetTabCloseRequested);

	connect(_editorContext->GetGameConfigurations(), &settings::GameConfigurationsSettings::ActiveConfigurationChanged,
		this, &HLMVMainWindow::SetupFileSystem);

	_ui.ActionSave->setEnabled(false);
	_ui.ActionSaveAs->setEnabled(false);
	_ui.MenuAsset->setEnabled(false);
	_assetTabs->setVisible(false);

	_editorContext->StartTimer();

	OnRecentFilesChanged();
	OnActiveConfigurationChanged(_editorContext->GetGameConfigurations()->GetActiveConfiguration(), {});
}

HLMVMainWindow::~HLMVMainWindow()
{
	_editorContext->GetTimer()->stop();
}

bool HLMVMainWindow::TryLoadAsset(const QString& fileName)
{
	try
	{
		auto asset = _editorContext->GetAssetProviderRegistry()->Load(_editorContext, fileName);

		if (nullptr != asset)
		{
			connect(asset.get(), &assets::Asset::FileNameChanged, this, &HLMVMainWindow::OnAssetFileNameChanged);

			auto editWidget = asset->GetEditWidget();

			_undoGroup->addStack(asset->GetUndoStack());

			_editorContext->GetLoadedAssets().emplace_back(std::move(asset), editWidget);

			const auto index = _assetTabs->addTab(editWidget, fileName);

			_assetTabs->setCurrentIndex(index);

			_assetTabs->setVisible(true);
			_ui.ActionFullscreen->setEnabled(true);

			_editorContext->GetRecentFiles()->Add(fileName);

			return true;
		}
		else
		{
			QMessageBox::critical(this, "Error loading asset", QString{"Error loading asset:\nNull asset returned"});
		}
	}
	catch (const ::assets::AssetException& e)
	{
		QMessageBox::critical(this, "Error loading asset", QString{"Error loading asset:\n%1"}.arg(e.what()));
	}

	return false;
}

void HLMVMainWindow::closeEvent(QCloseEvent* event)
{
	//TODO: ask to save pending changes for each document
	//TODO: if the user cancels any close request cancel the window close event as well

	//Close each asset
	while (_assetTabs->count() > 0)
	{
		OnAssetTabCloseRequested(0);
	}

	event->accept();
}

void HLMVMainWindow::UpdateTitle(const QString& fileName, bool hasUnsavedChanges)
{
	setWindowTitle(QString{"%1[*]"}.arg(fileName));
	setWindowModified(hasUnsavedChanges);
}

void HLMVMainWindow::OnRecentFilesChanged()
{
	const auto recentFiles = _editorContext->GetRecentFiles();

	_ui.MenuRecentFiles->clear();

	for (int i = 0; i < recentFiles->GetCount(); ++i)
	{
		_ui.MenuRecentFiles->addAction(recentFiles->At(i), this, &HLMVMainWindow::OnOpenRecentFile);
	}

	_ui.MenuRecentFiles->setEnabled(recentFiles->GetCount() > 0);
}

void HLMVMainWindow::OnOpenRecentFile()
{
	const auto action = static_cast<QAction*>(sender());

	const QString fileName{action->text()};

	if (!TryLoadAsset(fileName))
	{
		_editorContext->GetRecentFiles()->Remove(fileName);
	}
}

void HLMVMainWindow::OnExit()
{
	this->close();
}

void HLMVMainWindow::OnAssetCleanChanged(bool clean)
{
	setWindowModified(!clean);
}

void HLMVMainWindow::OnAssetTabChanged(int index)
{
	_ui.MenuAsset->clear();

	bool success = false;

	if (index != -1)
	{
		const auto editWidget = _assetTabs->widget(index);

		auto& assets = _editorContext->GetLoadedAssets();

		if (auto it = std::find_if(assets.begin(), assets.end(), [&](const auto& candidate)
			{
				return candidate.GetEditWidget() == editWidget;
			}); it != assets.end())
		{
			if (_currentAsset)
			{
				_currentAsset->SetActive(false);
			}

			const auto asset = it->GetAsset();

			_undoGroup->setActiveStack(asset->GetUndoStack());

			UpdateTitle(asset->GetFileName(), !_undoGroup->isClean());
			asset->PopulateAssetMenu(_ui.MenuAsset);

			_currentAsset = asset;

			asset->SetActive(true);

			success = true;
		}
		else
		{
			QMessageBox::critical(this, "Internal Error", "Asset not found in loaded assets list");
		}
	}

	if (!success)
	{
		_undoGroup->setActiveStack(nullptr);
		setWindowTitle({});
		_currentAsset = nullptr;
	}

	_ui.ActionSave->setEnabled(success);
	_ui.ActionSaveAs->setEnabled(success);
	_ui.MenuAsset->setEnabled(success);
}

void HLMVMainWindow::OnAssetTabCloseRequested(int index)
{
	if (_fullscreenWidget)
	{
		//Always exit the fullscreen window if we're getting a close request
		//The user needs to be able to see the main window and interact with it,
		//and the fullscreen window may be holding a reference to the asset being closed
		_fullscreenWidget->ExitFullscreen();
	}

	//TODO: ask to save, etc

	auto editWidget = _assetTabs->widget(index);

	_assetTabs->removeTab(index);

	auto& assets = _editorContext->GetLoadedAssets();

	if (auto it = std::find_if(assets.begin(), assets.end(), [&](const auto& asset)
		{
			return asset.GetEditWidget() == editWidget;
		}); it != assets.end())
	{
		_undoGroup->removeStack(it->GetAsset()->GetUndoStack());
		assets.erase(it);
	}
	else
	{
		QMessageBox::critical(this, "Internal Error", "Asset not found in loaded assets list");
	}

	const bool hasOpenAssets = _assetTabs->count() > 0;

	_assetTabs->setVisible(hasOpenAssets);
	_ui.ActionFullscreen->setEnabled(hasOpenAssets);
}

void HLMVMainWindow::OnAssetFileNameChanged(const QString& fileName)
{
	auto asset = static_cast<assets::Asset*>(sender());

	auto& assets = _editorContext->GetLoadedAssets();

	if (auto it = std::find_if(assets.begin(), assets.end(), [&](const auto& candidate)
		{
			return candidate.GetAsset() == asset;
		}); it != assets.end())
	{
		const int index = _assetTabs->indexOf(it->GetEditWidget());

		if (index != -1)
		{
			_assetTabs->setTabText(index, fileName);

			_editorContext->GetRecentFiles()->Add(fileName);

			if (_assetTabs->currentWidget() == it->GetEditWidget())
			{
				UpdateTitle(it->GetAsset()->GetFileName(), !_undoGroup->isClean());
			}
		}
		else
		{
			QMessageBox::critical(this, "Internal Error", "Asset index not found in assets tab widget");
		}
	}
	else
	{
		QMessageBox::critical(this, "Internal Error", "Asset not found in loaded assets list");
	}
}

void HLMVMainWindow::OnOpenLoadAssetDialog()
{
	//TODO: compute filter based on available asset providers
	if (const auto fileName = QFileDialog::getOpenFileName(this, "Select asset", {}, "Half-Life 1 Model Files (*.mdl *.dol);;All Files (*.*)");
		!fileName.isEmpty())
	{
		TryLoadAsset(fileName);
	}
}

void HLMVMainWindow::OnSaveAsset()
{
	auto& assets = _editorContext->GetLoadedAssets();

	auto& currentAsset = assets[_assetTabs->currentIndex()];

	auto asset = currentAsset.GetAsset();

	try
	{
		asset->Save(asset->GetFileName());
	}
	catch (const ::assets::AssetException& e)
	{
		QMessageBox::critical(this, "Error saving asset", QString{"Error saving asset:\n%1"}.arg(e.what()));
		return;
	}

	auto undoStack = asset->GetUndoStack();

	undoStack->setClean();
}

void HLMVMainWindow::OnSaveAssetAs()
{
	auto& assets = _editorContext->GetLoadedAssets();

	auto& currentAsset = assets[_assetTabs->currentIndex()];

	//TODO: compute filter based on available asset providers
	QString fileName{QFileDialog::getSaveFileName(
		this, {}, currentAsset.GetAsset()->GetFileName(), "Half-Life 1 Model Files (*.mdl *.dol);;All Files (*.*)")};

	if (!fileName.isEmpty())
	{
		currentAsset.GetAsset()->SetFileName(std::move(fileName));

		OnSaveAsset();
	}
}

void HLMVMainWindow::OnGoFullscreen()
{
	auto& assets = _editorContext->GetLoadedAssets();

	auto& currentAsset = assets[_assetTabs->currentIndex()];

	if (!_fullscreenWidget)
	{
		//Note: creating this window as a child of the main window causes problems with OpenGL rendering
		//This must be created with no parent to function properly
		_fullscreenWidget = std::make_unique<FullscreenWidget>();
	}

	currentAsset.GetAsset()->SetupFullscreenWidget(_fullscreenWidget.get());

	_fullscreenWidget->raise();
	_fullscreenWidget->showFullScreen();
	_fullscreenWidget->activateWindow();
}

void HLMVMainWindow::OnOpenOptionsDialog()
{
	options::OptionsDialog dialog{_editorContext, this};

	dialog.exec();
}

void HLMVMainWindow::OnShowAbout()
{
	QMessageBox::information(this, "About Half-Life Model Viewer",
		QString::fromUtf8(
			u8"Half-Life Model Viewer 3.0\n"
			u8"2020 Sam Vanheer\n\n"
			u8"Email:    sam.vanheer@outlook.com\n\n"
			u8"Based on Jed's Half-Life Model Viewer v1.3 © 2004 Neil \'Jed\' Jedrzejewski\n"
			u8"Email:    jed@wunderboy.org\n"
			u8"Web:      http://www.wunderboy.org/\n\n"
			u8"Also based on Half-Life Model Viewer v1.25 © 2002 Mete Ciragan\n"
			u8"Email:    mete@swissquake.ch\n"
			u8"Web:      http://www.milkshape3d.com/\n\n"
			u8"%1")
			.arg(QString::fromUtf8(GetSharedCredits().c_str()))
	);
}

void HLMVMainWindow::SetupFileSystem(std::pair<settings::GameEnvironment*, settings::GameConfiguration*> activeConfiguration)
{
	auto fileSystem = _editorContext->GetFileSystem();

	fileSystem->RemoveAllSearchPaths();

	const auto environment = activeConfiguration.first;
	const auto configuration = activeConfiguration.second;
	const auto defaultGameConfiguration = environment->GetGameConfigurationById(environment->GetDefaultModId());

	fileSystem->SetBasePath(environment->GetInstallationPath().toStdString().c_str());

	const char* const* ppszDirectoryExts;

	//TODO: rework this stuff to use std::string and separate out steampipe stuff
	const size_t uiNumExts = fileSystem->GetSteamPipeDirectoryExtensions(ppszDirectoryExts);

	const auto gameDir{defaultGameConfiguration->GetDirectory().toStdString()};
	const auto modDir{configuration->GetDirectory().toStdString()};

	//Add mod dirs first since they override game dirs
	if (gameDir != modDir)
	{
		for (size_t uiIndex = 0; uiIndex < uiNumExts; ++uiIndex)
		{
			fileSystem->AddSearchPath((modDir + ppszDirectoryExts[uiIndex]).c_str());
		}
	}

	for (size_t uiIndex = 0; uiIndex < uiNumExts; ++uiIndex)
	{
		fileSystem->AddSearchPath((gameDir + ppszDirectoryExts[uiIndex]).c_str());
	}
}

void HLMVMainWindow::OnActiveConfigurationChanged(std::pair<settings::GameEnvironment*, settings::GameConfiguration*> current,
	std::pair<settings::GameEnvironment*, settings::GameConfiguration*> previous)
{
	if (previous.second)
	{
		disconnect(previous.second, &settings::GameConfiguration::DirectoryChanged, this, &HLMVMainWindow::OnGameConfigurationDirectoryChanged);
	}

	if (current.second)
	{
		connect(current.second, &settings::GameConfiguration::DirectoryChanged, this, &HLMVMainWindow::OnGameConfigurationDirectoryChanged);

		SetupFileSystem(current);
	}
	else
	{
		auto fileSystem = _editorContext->GetFileSystem();

		fileSystem->RemoveAllSearchPaths();
	}
}

void HLMVMainWindow::OnGameConfigurationDirectoryChanged()
{
	SetupFileSystem(_editorContext->GetGameConfigurations()->GetActiveConfiguration());
}
}
