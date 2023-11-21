#include <algorithm>
#include <cassert>

#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QMenu>
#include <QMessageBox>
#include <QOpenGLFunctions_1_1>
#include <QSettings>
#include <QSignalBlocker>

#include "application/AssetList.hpp"
#include "application/AssetManager.hpp"

#include "filesystem/FileSystem.hpp"

#include "formats/sprite/SpriteRenderer.hpp"
#include "formats/studiomodel/DumpModelInfo.hpp"
#include "formats/studiomodel/StudioModelIO.hpp"
#include "formats/studiomodel/StudioModelRenderer.hpp"
#include "formats/studiomodel/StudioModelUtils.hpp"

#include "graphics/IGraphicsContext.hpp"
#include "graphics/Palette.hpp"

#include "plugins/halflife/studiomodel/StudioModelAsset.hpp"
#include "plugins/halflife/studiomodel/StudioModelAssetProvider.hpp"
#include "plugins/halflife/studiomodel/ui/StudioModelEditWidget.hpp"

#include "plugins/halflife/studiomodel/ui/dialogs/QCDataDialog.hpp"

#include "qt/QtLogging.hpp"
#include "qt/QtUtilities.hpp"

#include "settings/ApplicationSettings.hpp"
#include "settings/ExternalProgramSettings.hpp"
#include "settings/StudioModelSettings.hpp"

#include "ui/MainWindow.hpp"
#include "ui/SceneWidget.hpp"
#include "ui/camera_operators/ArcBallCameraOperator.hpp"
#include "ui/camera_operators/CameraOperators.hpp"
#include "ui/camera_operators/FirstPersonCameraOperator.hpp"
#include "ui/camera_operators/FreeLookCameraOperator.hpp"

#include "utility/IOUtils.hpp"

namespace studiomodel
{
Q_LOGGING_CATEGORY(HLAMStudioModel, "hlam.studiomodel")
Q_LOGGING_CATEGORY(HLAMSpriteRenderer, "hlam.spriterenderer")
Q_LOGGING_CATEGORY(HLAMStudioModelRenderer, "hlam.studiomodelrenderer")

const QString StudioModelSettingsGroup{QStringLiteral("Assets/StudioModel")};
const QString WindowStateKey{QStringLiteral("WindowState")};
const QString WindowGeometryKey{QStringLiteral("WindowGeometry")};

StudioModelAssetProvider::StudioModelAssetProvider(AssetManager* application,
	const std::shared_ptr<StudioModelSettings>& studioModelSettings)
	: AssetProvider(application)
	, _logger(CreateQtLoggerSt(HLAMStudioModel()))
	, _studioModelSettings(studioModelSettings)

	, _studioModelRenderer(std::make_unique<studiomdl::StudioModelRenderer>(
		CreateQtLoggerSt(HLAMStudioModelRenderer()),
		_application->GetOpenGLFunctions(), _application->GetColorSettings()))

	, _spriteRenderer(std::make_unique<sprite::SpriteRenderer>(
		CreateQtLoggerSt(HLAMSpriteRenderer()), _application->GetWorldTime()))

	, _dummyAsset(std::make_unique<StudioModelAsset>(
		"", _application, this, _settingsVersion,
		std::make_unique<studiomdl::EditableStudioModel>(),
		std::make_unique<FileSystem>()))

	, _cameraOperators(new CameraOperators(this))
	, _arcBallCamera(new ArcBallCameraOperator(application->GetApplicationSettings()))
	, _firstPersonCamera(new FirstPersonCameraOperator(application->GetApplicationSettings()))
{
	_cameraOperators->Add(_arcBallCamera);
	_cameraOperators->Add(new FreeLookCameraOperator(application->GetApplicationSettings()));
	_cameraOperators->Add(_firstPersonCamera);

	for (auto cameraOperator : _cameraOperators->GetAll())
	{
		auto camera = cameraOperator->GetCamera();
		camera->SetFieldOfView(_studioModelSettings->GetCameraFOV(cameraOperator->GetName(), camera->GetFieldOfView()));
	}

	_currentAsset = GetDummyAsset();

	connect(_application->GetApplicationSettings(), &ApplicationSettings::SettingsLoaded,
		this, &StudioModelAssetProvider::UpdateSettingsState);
	connect(_application->GetApplicationSettings(), &ApplicationSettings::SettingsSaved,
		this, &StudioModelAssetProvider::UpdateSettingsState);

	connect(_application->GetAssets(), &AssetList::ActiveAssetChanged,
		this, &StudioModelAssetProvider::OnActiveAssetChanged);

	CreateToolMenu();
	UpdateSettingsState();

	{
		const auto graphicsContext = _application->GetGraphicsContext();
		const auto openglFunctions = _application->GetOpenGLFunctions();

		graphicsContext->Begin();
		openglFunctions->glGenTextures(1, &_defaultGroundTexture);
		openglFunctions->glBindTexture(GL_TEXTURE_2D, _defaultGroundTexture);

		const int side = 64;
		const int pixelCount = side * side;

		graphics::RGB24 pixels[pixelCount];

		// Paint texture as darkish gray with black lines in the center (based on Source's dev_measuregeneric01b).
		// Gray/white lines don't look so good due to transparency.
		const graphics::RGB24 groundColor{90, 90, 90};
		const graphics::RGB24 lineColor{0, 0, 0};

		std::fill(std::begin(pixels), std::end(pixels), groundColor);

		for (int edge = 0; edge < 2; ++edge)
		{
			const int middle = (side / 2) - edge;

			for (int i = 0; i < side; ++i)
			{
				pixels[(side * middle) + i] = lineColor;
			}

			for (int i = 0; i < side; ++i)
			{
				pixels[middle + (i * side)] = lineColor;
			}
		}

		openglFunctions->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, side, side, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
		openglFunctions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		openglFunctions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		graphicsContext->End();
	}
}

StudioModelAssetProvider::~StudioModelAssetProvider()
{
	delete _editWidget;
}

void StudioModelAssetProvider::Shutdown()
{
	{
		const auto graphicsContext = _application->GetGraphicsContext();
		const auto openglFunctions = _application->GetOpenGLFunctions();

		graphicsContext->Begin();
		openglFunctions->glDeleteTextures(1, &_defaultGroundTexture);
		_defaultGroundTexture = 0;
		graphicsContext->End();
	}

	if (_editWidget)
	{
		auto settings = _application->GetSettings();

		settings->beginGroup(StudioModelSettingsGroup);
		settings->setValue(WindowGeometryKey, _editWidget->saveGeometry());
		settings->setValue(WindowStateKey, _editWidget->SaveState());
		settings->endGroup();
	}

	for (auto cameraOperator : _cameraOperators->GetAll())
	{
		_studioModelSettings->SetCameraFOV(cameraOperator->GetName(), cameraOperator->GetCamera()->GetFieldOfView());
	}
}

void StudioModelAssetProvider::Tick()
{
	_studioModelRenderer->RunFrame();

	GetCurrentAsset()->Tick();

	_editWidget->Tick();
}

QMenu* StudioModelAssetProvider::CreateToolMenu()
{
	if (!_toolMenu)
	{
		_toolMenu = new QMenu("StudioModel");

		_launchCrowbarAction = _toolMenu->addAction("Launch Crowbar",
			[this] { _application->TryLaunchExternalProgram(CrowbarFileNameKey, QStringList{}); });

		_toolMenu->addAction("Edit QC File...", []
			{
				const QString fileName{QFileDialog::getOpenFileName(
					nullptr, "Select QC File", {}, "QC files (*.qc);;All Files (*.*)")};

				if (!fileName.isEmpty())
				{
					qt::LaunchDefaultProgram(fileName);
				}
			});
	}

	return _toolMenu;
}

void StudioModelAssetProvider::PopulateAssetMenu(QMenu* menu)
{
	{
		auto dockWidgetsMenu = menu->addMenu("Dock Widgets");

		for (auto dock : GetEditWidget()->GetDockWidgets())
		{
			dockWidgetsMenu->addAction(dock->toggleViewAction());
		}
	}

	menu->addAction("Reset Dock Widgets", this, [this]()
		{
			GetEditWidget()->ResetToInitialState();
		});

	_controlsBarVisibleAction = menu->addAction("Show Controls Bar", this, [this](bool checked)
		{
			GetEditWidget()->SetControlsBarVisible(checked);
			_studioModelSettings->SetControlsBarVisible(checked);
		});

	_controlsBarVisibleAction->setCheckable(true);
	_controlsBarVisibleAction->setChecked(GetEditWidget()->IsControlsBarVisible());

	_timelineVisibleAction = menu->addAction("Show Timeline", this, [this](bool checked)
		{
			GetEditWidget()->SetTimelineVisible(checked);
			_studioModelSettings->SetTimelineVisible(checked);
		});

	_timelineVisibleAction->setCheckable(true);
	_timelineVisibleAction->setChecked(GetEditWidget()->IsTimelineVisible());

	{
		_editControlsVisibleAction = menu->addAction("Show Edit Controls", this,
			[this](bool checked)
			{
				_controlsBarVisibleAction->setEnabled(checked);
				_timelineVisibleAction->setEnabled(checked);

				auto asset = GetCurrentAsset();

				asset->OnSceneWidgetRecreated();

				_studioModelSettings->SetEditControlsVisible(checked);
			});

		_editControlsVisibleAction->setCheckable(true);
		_editControlsVisibleAction->setChecked(_studioModelSettings->AreEditControlsVisible());
	}

	_controlsBarVisibleAction->setEnabled(_editControlsVisibleAction->isChecked());
	_timelineVisibleAction->setEnabled(_editControlsVisibleAction->isChecked());

	menu->addSeparator();

	menu->addAction("Previous Camera", GetCameraOperators(), &CameraOperators::PreviousCamera,
		QKeySequence{Qt::CTRL + static_cast<int>(Qt::Key::Key_U)});

	menu->addAction("Next Camera", GetCameraOperators(), &CameraOperators::NextCamera,
		QKeySequence{Qt::CTRL + static_cast<int>(Qt::Key::Key_I)});

	menu->addSeparator();

	{
		auto centerMenu = menu->addMenu("Center View");

		centerMenu->addAction("Center On Positive X", [this]() { GetCurrentAsset()->OnCenterView(Axis::X, true); });
		centerMenu->addAction("Center On Negative X", [this]() { GetCurrentAsset()->OnCenterView(Axis::X, false); });

		//TODO: camera position doesn't match what's shown by "Draw Axes" on the Y axis
		centerMenu->addAction("Center On Positive Y", [this]() { GetCurrentAsset()->OnCenterView(Axis::Y, false); });
		centerMenu->addAction("Center On Negative Y", [this]() { GetCurrentAsset()->OnCenterView(Axis::Y, true); });

		centerMenu->addAction("Center On Positive Z", [this]() { GetCurrentAsset()->OnCenterView(Axis::Z, true); });
		centerMenu->addAction("Center On Negative Z", [this]() { GetCurrentAsset()->OnCenterView(Axis::Z, false); });
	}

	menu->addAction("Save View", this, [this]()
		{
			auto cameraOperators = GetCameraOperators();

			if (auto cameraOperator = cameraOperators->GetCurrent(); cameraOperator)
			{
				StateSnapshot snapshot;

				if (cameraOperator->SaveView(&snapshot))
				{
					_cameraSnapshot = std::move(snapshot);
					_cameraSnapshot.SetValue("CameraIndex", cameraOperators->IndexOf(cameraOperator));
					_restoreViewAction->setEnabled(true);
				}
			}
		});

	_restoreViewAction = menu->addAction("Restore View", this, [this]()
		{
			auto cameraOperators = GetCameraOperators();

			const int index = _cameraSnapshot.Value("CameraIndex", -1).toInt();

			if (index == -1)
			{
				return;
			}

			if (const auto cameraOperator = cameraOperators->Get(index); cameraOperator)
			{
				cameraOperators->SetCurrent(cameraOperator);
				cameraOperator->RestoreView(&_cameraSnapshot);
			}
		});

	_restoreViewAction->setEnabled(false);

	menu->addSeparator();

	menu->addAction("Flip Normals", this, [this]() { GetCurrentAsset()->OnFlipNormals(); });

	menu->addSeparator();

	menu->addAction("Show QC Data", this, [this]
		{
			QCDataDialog dialog{this, _application->GetMainWindow()};
			dialog.exec();
		});

	menu->addAction("Dump Model Info...", this, &StudioModelAssetProvider::OnDumpModelInfo);

	menu->addSeparator();
}

bool StudioModelAssetProvider::CanLoad(const QString& fileName, FILE* file) const
{
	return studiomdl::IsStudioModel(file);
}

AssetLoadData StudioModelAssetProvider::Load(const QString& fileName, FILE* file)
{
	const auto filePath = std::filesystem::u8path(fileName.toStdString());

	auto fileSystem = std::make_unique<FileSystem>();
	_application->InitializeFileSystem(*fileSystem, fileName);

	auto studioModel = studiomdl::LoadStudioModel(filePath, file, *fileSystem);

	if (studiomdl::IsXashModel(*studioModel))
	{
		_logger->debug("Model {} is a Xash model", fileName);

		const XashOpenMode mode = _studioModelSettings->GetXashOpenMode();

		if (mode != XashOpenMode::Never)
		{
			bool loadInXashModelViewer = true;

			if (mode == XashOpenMode::Ask)
			{
				const auto action = QMessageBox::question(_application->GetMainWindow(),
					"Attempting to load Xash model", R"(This model was created using Xash's model compiler.

Load in Xash Model Viewer?)", QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

				loadInXashModelViewer = action == QMessageBox::Yes;
			}

			if (loadInXashModelViewer)
			{
				return AssetLoadInExternalProgram{
					.ExternalProgramKey = XashModelViewerFileNameKey,
					.PromptBeforeOpening = false
				};
			}
		}
	}

	auto editableStudioModel = studiomdl::ConvertToEditable(*studioModel);

	if (studioModel->GetSeqGroupCount() > 0)
	{
		_logger->info("Merged {} sequence group files into main file \"{}\"", studioModel->GetSeqGroupCount(), fileName);
	}

	if (studioModel->HasSeparateTextureHeader())
	{
		_logger->info("Merged texture file into main file \"{}\"", fileName);
	}
	
	auto asset = std::make_unique<StudioModelAsset>(QString{fileName}, _application, this, _settingsVersion,
		std::make_unique<studiomdl::EditableStudioModel>(std::move(editableStudioModel)),
		std::move(fileSystem));

	return asset;
}

bool StudioModelAssetProvider::IsCandidateForLoading(const QString& fileName, FILE* file) const
{
	return studiomdl::IsMainStudioModel(file);
}

bool StudioModelAssetProvider::IsControlsBarVisible() const
{
	return _controlsBarVisibleAction->isChecked();
}

bool StudioModelAssetProvider::IsTimelineVisible() const
{
	return _timelineVisibleAction->isChecked();
}

bool StudioModelAssetProvider::AreEditControlsVisible() const
{
	return _editControlsVisibleAction->isChecked();
}

StudioModelEditWidget* StudioModelAssetProvider::GetEditWidget()
{
	if (!_editWidget)
	{
		assert(_application);
		_editWidget = new StudioModelEditWidget(_application, this);

		auto settings = _application->GetSettings();

		settings->beginGroup(StudioModelSettingsGroup);
		_editWidget->restoreGeometry(settings->value(WindowGeometryKey).toByteArray());
		_editWidget->RestoreState(settings->value(WindowStateKey).toByteArray());
		settings->endGroup();
		
		_editWidget->SetControlsBarVisible(_studioModelSettings->IsControlsBarVisible());
		_editWidget->SetTimelineVisible(_studioModelSettings->IsTimelineVisible());
	}

	return _editWidget;
}

bool StudioModelAssetProvider::CameraIsFirstPerson() const
{
	return _cameraOperators->GetCurrent() == _firstPersonCamera;
}

void StudioModelAssetProvider::UpdateSettingsState()
{
	const auto externalPrograms = _application->GetApplicationSettings()->GetExternalPrograms();

	if (_launchCrowbarAction)
	{
		_launchCrowbarAction->setEnabled(!externalPrograms->GetProgram(CrowbarFileNameKey).isEmpty());
	}

	++_settingsVersion;

	UpdateActiveAssetSettingsState();
}

void StudioModelAssetProvider::UpdateActiveAssetSettingsState()
{
	if (_currentAsset != GetDummyAsset() && _settingsVersion != _currentAsset->_settingsVersion)
	{
		_currentAsset->_settingsVersion = _settingsVersion;
		_currentAsset->UpdateSettingsState();
	}
}

void StudioModelAssetProvider::OnActiveAssetChanged(Asset* asset)
{
	if (_currentAsset)
	{
		_currentAsset->OnDeactivated();
	}

	_currentAsset = asset && asset->GetProvider() == this ? static_cast<StudioModelAsset*>(asset) : GetDummyAsset();

	emit AssetChanged(_currentAsset);

	_currentAsset->OnActivated();

	UpdateActiveAssetSettingsState();
}

void StudioModelAssetProvider::OnDumpModelInfo()
{
	const auto asset = GetCurrentAsset();

	const QFileInfo fileInfo{asset->GetFileName()};

	const auto suggestedFileName{QString{"%1%2%3_modelinfo.txt"}.arg(fileInfo.path()).arg(QDir::separator()).arg(fileInfo.completeBaseName())};

	const QString fileName{QFileDialog::getSaveFileName(nullptr, {}, suggestedFileName, "Text Files (*.txt);;All Files (*.*)")};

	if (!fileName.isEmpty())
	{
		if (FILE* file = utf8_fopen(fileName.toStdString().c_str(), "w"); file)
		{
			studiomdl::DumpModelInfo(file, asset->GetFileName().toStdString(), *asset->GetEditableStudioModel());

			fclose(file);

			qt::LaunchDefaultProgram(fileName);
		}
		else
		{
			QMessageBox::critical(nullptr, "Error", QString{"Could not open file \"%1\" for writing"}.arg(fileName));
		}
	}
}
}
