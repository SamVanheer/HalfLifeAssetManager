#include <algorithm>

#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>

#include "Credits.hpp"

#include "ui/EditorContext.hpp"
#include "ui/FullscreenWidget.hpp"
#include "ui/HLMVMainWindow.hpp"
#include "ui/assets/Assets.hpp"
#include "ui/options/OptionsDialog.hpp"

#include "ui/assets/studiomodel/StudioModelEditWidget.hpp"

namespace ui
{
HLMVMainWindow::HLMVMainWindow(EditorContext* editorContext)
	: QMainWindow()
	, _editorContext(editorContext)
{
	_ui.setupUi(this);

	_assetTabs = new QTabWidget(this);

	//Eliminate the border on the sides so the scene widget takes up all horizontal space
	_assetTabs->setDocumentMode(true);

	_assetTabs->setTabsClosable(true);

	_assetTabs->setElideMode(Qt::TextElideMode::ElideLeft);

	setCentralWidget(_assetTabs);

	connect(_ui.ActionLoad, &QAction::triggered, this, &HLMVMainWindow::OnOpenLoadAssetDialog);
	connect(_ui.ActionFullscreen, &QAction::triggered, this, &HLMVMainWindow::OnGoFullscreen);
	connect(_ui.ActionOptions, &QAction::triggered, this, &HLMVMainWindow::OnOpenOptionsDialog);
	connect(_ui.ActionAbout, &QAction::triggered, this, &HLMVMainWindow::OnShowAbout);

	connect(_assetTabs, &QTabWidget::tabCloseRequested, this, &HLMVMainWindow::OnAssetTabCloseRequested);

	_assetTabs->setVisible(false);

	_editorContext->GetTimer()->start(0);
}

HLMVMainWindow::~HLMVMainWindow() = default;

void HLMVMainWindow::OnOpenLoadAssetDialog()
{
	//TODO: compute filter based on available asset providers
	if (const auto fileName = QFileDialog::getOpenFileName(this, "Select asset", {}, "Half-Life 1 Model Files (*.mdl *.dol);;All Files (*.*)");
		!fileName.isEmpty())
	{
		LoadAsset(fileName);
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

	currentAsset.GetAsset()->SetupFullscreenWidget(_editorContext, _fullscreenWidget.get());

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
			.arg(QString::fromUtf8(tools::GetSharedCredits().c_str()))
	);
}

void HLMVMainWindow::LoadAsset(const QString& fileName)
{
	auto asset = _editorContext->GetAssetProviderRegistry()->Load(_editorContext, fileName.toStdString());

	if (nullptr != asset)
	{
		auto editWidget = asset->CreateEditWidget(_editorContext);

		_editorContext->GetLoadedAssets().emplace_back(std::move(asset), editWidget);

		const auto index = _assetTabs->addTab(editWidget, fileName);

		_assetTabs->setCurrentIndex(index);

		_assetTabs->setVisible(true);
		_ui.ActionFullscreen->setEnabled(true);
	}
}

void HLMVMainWindow::OnAssetTabCloseRequested(int index)
{
	//Always exit the fullscreen window if we're getting a close request
	//The user needs to be able to see the main window and interact with it,
	//and the fullscreen window may be holding a reference to the asset being closed
	_fullscreenWidget->ExitFullscreen();

	//TODO: ask to save, etc

	auto editWidget = _assetTabs->widget(index);

	delete editWidget;

	_assetTabs->removeTab(index);

	auto& assets = _editorContext->GetLoadedAssets();

	if (auto it = std::find_if(assets.begin(), assets.end(), [&](const auto& asset)
		{
			return asset.GetEditWidget() == editWidget;
		}); it != assets.end())
	{
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
}
