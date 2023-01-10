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

#include "plugins/halflife/studiomodel/ui/compiler/StudioModelCompilerFrontEnd.hpp"
#include "plugins/halflife/studiomodel/ui/compiler/StudioModelDecompilerFrontEnd.hpp"
#include "plugins/halflife/studiomodel/ui/dialogs/QCDataDialog.hpp"

#include "qt/QtLogging.hpp"
#include "qt/QtUtilities.hpp"

#include "settings/ApplicationSettings.hpp"
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

const QString WindowStateKey{QStringLiteral("Asset/StudioModel/WindowState")};

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
		"", _application, this, std::make_unique<studiomdl::EditableStudioModel>()))

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

	connect(_application, &AssetManager::Tick, this, &StudioModelAssetProvider::OnTick);
	connect(_application->GetAssets(), &AssetList::ActiveAssetChanged,
		this, &StudioModelAssetProvider::OnActiveAssetChanged);

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
		_application->GetSettings()->setValue(WindowStateKey, _editWidget->SaveState());
	}

	for (auto cameraOperator : _cameraOperators->GetAll())
	{
		_studioModelSettings->SetCameraFOV(cameraOperator->GetName(), cameraOperator->GetCamera()->GetFieldOfView());
	}
}

QMenu* StudioModelAssetProvider::CreateToolMenu()
{
	auto menu = new QMenu("StudioModel");

	menu->addAction("Compile Model...", [this]
		{
			StudioModelCompilerFrontEnd compiler{_application};
	compiler.exec();
		});

	menu->addAction("Decompile Model...", [this]
		{
			StudioModelDecompilerFrontEnd decompiler{_application};
	decompiler.exec();
		});

	menu->addAction("Edit QC File...", []
		{
			const QString fileName{QFileDialog::getOpenFileName(nullptr, "Select QC File", {}, "QC files (*.qc);;All Files (*.*)")};

	if (!fileName.isEmpty())
	{
		qt::LaunchDefaultProgram(fileName);
	}
		});

	return menu;
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

	const auto controlsbar = menu->addAction("Show Controls Bar", this, [this](bool checked)
		{
			GetEditWidget()->SetControlsBarVisible(checked);
			_studioModelSettings->SetControlsBarVisible(checked);
		});

	controlsbar->setCheckable(true);
	controlsbar->setChecked(GetEditWidget()->IsControlsBarVisible());

	const auto timeline = menu->addAction("Show Timeline", this, [this](bool checked)
		{
			GetEditWidget()->SetTimelineVisible(checked);
			_studioModelSettings->SetTimelineVisible(checked);
		});

	timeline->setCheckable(true);
	timeline->setChecked(GetEditWidget()->IsTimelineVisible());

	{
		_editControlsVisibleAction = menu->addAction("Show Edit Controls", this,
			[this, controlsbar, timeline](bool checked)
			{
				controlsbar->setEnabled(checked);
				timeline->setEnabled(checked);

				auto asset = GetCurrentAsset();

				if (asset != GetDummyAsset())
				{
					asset->OnDeactivated();
					asset->OnActivated();
				}

				_studioModelSettings->SetEditControlsVisible(checked);
			});

		_editControlsVisibleAction->setCheckable(true);
		_editControlsVisibleAction->setChecked(_studioModelSettings->AreEditControlsVisible());
	}

	controlsbar->setEnabled(_editControlsVisibleAction->isChecked());
	timeline->setEnabled(_editControlsVisibleAction->isChecked());

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

	menu->addAction("Take Screenshot...", this, &StudioModelAssetProvider::OnTakeScreenshot);
}

bool StudioModelAssetProvider::CanLoad(const QString& fileName, FILE* file) const
{
	return studiomdl::IsStudioModel(file);
}

std::variant<std::unique_ptr<Asset>, AssetLoadInExternalProgram> StudioModelAssetProvider::Load(
	const QString& fileName, FILE* file)
{
	_logger->trace("Trying to load model {}", fileName);

	const auto filePath = std::filesystem::u8path(fileName.toStdString());
	auto studioModel = studiomdl::LoadStudioModel(filePath, file);

	if (studiomdl::IsXashModel(*studioModel))
	{
		_logger->debug("Model {} is a Xash model", fileName);

		const auto result = _application->TryLaunchExternalProgram(
			XashModelViewerFileNameKey, QStringList(fileName),
			"This is a Xash model which requires it to be loaded in Xash Model Viewer.");

		if (result != LaunchExternalProgramResult::Failed)
		{
			return AssetLoadInExternalProgram{.Loaded = result == LaunchExternalProgramResult::Success};
		}
		
		throw AssetException(std::string{"File \""} + fileName.toStdString()
			+ "\" is a Xash model and cannot be opened by this program."
			+ "\nSet the Xash Model Viewer executable setting to open the model through that program instead.");
	}

	auto editableStudioModel = studiomdl::ConvertToEditable(*studioModel);

	return std::make_unique<StudioModelAsset>(QString{fileName}, _application, this,
		std::make_unique<studiomdl::EditableStudioModel>(std::move(editableStudioModel)));
}

bool StudioModelAssetProvider::IsCandidateForLoading(const QString& fileName, FILE* file) const
{
	return studiomdl::IsMainStudioModel(file);
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
		_editWidget->RestoreState(_application->GetSettings()->value(WindowStateKey).toByteArray());
		_editWidget->SetControlsBarVisible(_studioModelSettings->IsControlsBarVisible());
		_editWidget->SetTimelineVisible(_studioModelSettings->IsTimelineVisible());
	}

	return _editWidget;
}

bool StudioModelAssetProvider::CameraIsFirstPerson() const
{
	return _cameraOperators->GetCurrent() == _firstPersonCamera;
}

void StudioModelAssetProvider::OnTick()
{
	_studioModelRenderer->RunFrame();

	GetCurrentAsset()->Tick();

	_editWidget->Tick();
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
			studiomdl::DumpModelInfo(file, *asset->GetEditableStudioModel());

			fclose(file);

			qt::LaunchDefaultProgram(fileName);
		}
		else
		{
			QMessageBox::critical(nullptr, "Error", QString{"Could not open file \"%1\" for writing"}.arg(fileName));
		}
	}
}

void StudioModelAssetProvider::OnTakeScreenshot()
{
	const QImage screenshot = _application->GetSceneWidget()->grabFramebuffer();

	const QString fileName{QFileDialog::getSaveFileName(nullptr, {}, {}, qt::GetImagesFileFilter())};

	if (!fileName.isEmpty())
	{
		if (!screenshot.save(fileName))
		{
			QMessageBox::critical(nullptr, "Error", "An error occurred while saving screenshot");
		}
	}
}
}
