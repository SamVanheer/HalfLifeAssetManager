#include <cassert>

#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QSignalBlocker>

#include "formats/sprite/SpriteRenderer.hpp"
#include "formats/studiomodel/StudioModelIO.hpp"
#include "formats/studiomodel/StudioModelRenderer.hpp"
#include "formats/studiomodel/StudioModelUtils.hpp"

#include "plugins/halflife/studiomodel/StudioModelAsset.hpp"
#include "plugins/halflife/studiomodel/StudioModelAssetProvider.hpp"
#include "plugins/halflife/studiomodel/ui/StudioModelEditWidget.hpp"

#include "plugins/halflife/studiomodel/ui/compiler/StudioModelCompilerFrontEnd.hpp"
#include "plugins/halflife/studiomodel/ui/compiler/StudioModelDecompilerFrontEnd.hpp"
#include "plugins/halflife/studiomodel/ui/dialogs/QCDataDialog.hpp"

#include "qt/QtLogSink.hpp"
#include "qt/QtUtilities.hpp"

#include "settings/ApplicationSettings.hpp"
#include "settings/StudioModelSettings.hpp"

#include "ui/EditorContext.hpp"
#include "ui/camera_operators/CameraOperators.hpp"

namespace studiomodel
{
Q_LOGGING_CATEGORY(HLAMStudioModel, "hlam.studiomodel")

const QString WindowStateKey{QStringLiteral("Asset/StudioModel/WindowState")};

StudioModelAssetProvider::StudioModelAssetProvider(const std::shared_ptr<StudioModelSettings>& studioModelSettings)
	: _studioModelSettings(studioModelSettings)
{
}

StudioModelAssetProvider::~StudioModelAssetProvider()
{
	delete _editWidget;
}

void StudioModelAssetProvider::Initialize(EditorContext* editorContext)
{
	AssetProvider::Initialize(editorContext);

	_studioModelRenderer = std::make_unique<studiomdl::StudioModelRenderer>(
		CreateQtLoggerSt(logging::HLAMStudioModelRenderer()),
		_editorContext->GetOpenGLFunctions(), _editorContext->GetColorSettings());

	_spriteRenderer = std::make_unique<sprite::SpriteRenderer>(
		CreateQtLoggerSt(logging::HLAMSpriteRenderer()), _editorContext->GetWorldTime());

	_dummyAsset = std::make_unique<StudioModelAsset>(
		"", _editorContext, this, std::make_unique<studiomdl::EditableStudioModel>());

	_currentAsset = GetDummyAsset();

	connect(_editorContext, &EditorContext::Tick, this, &StudioModelAssetProvider::OnTick);
	connect(_editorContext, &EditorContext::ActiveAssetChanged, this, &StudioModelAssetProvider::OnActiveAssetChanged);
}

void StudioModelAssetProvider::Shutdown()
{
	if (_editWidget)
	{
		_editorContext->GetSettings()->setValue(WindowStateKey, _editWidget->SaveState());
	}
}

QMenu* StudioModelAssetProvider::CreateToolMenu()
{
	auto menu = new QMenu("StudioModel");

	menu->addAction("Compile Model...", [this]
		{
			StudioModelCompilerFrontEnd compiler{_editorContext};
	compiler.exec();
		});

	menu->addAction("Decompile Model...", [this]
		{
			StudioModelDecompilerFrontEnd decompiler{_editorContext};
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
		auto panelsMenu = menu->addMenu("Panels");

		for (auto dock : GetEditWidget()->GetDockWidgets())
		{
			panelsMenu->addAction(dock->toggleViewAction());
		}
	}

	menu->addAction("Reset dock widgets", this, [this]()
		{
			GetEditWidget()->ResetToInitialState();
		});

	const auto controlsbar = menu->addAction("Show Controls Bar", this, [this](bool checked)
		{
			GetEditWidget()->SetControlsBarVisible(checked);
		});

	{
		const QSignalBlocker blocker{controlsbar};
		controlsbar->setCheckable(true);
		controlsbar->setChecked(GetEditWidget()->IsControlsBarVisible());
	}

	const auto timeline = menu->addAction("Show Timeline", this, [this](bool checked)
		{
			GetEditWidget()->SetTimelineVisible(checked);
		});

	{
		const QSignalBlocker blocker{timeline};
		timeline->setCheckable(true);
		timeline->setChecked(GetEditWidget()->IsTimelineVisible());
	}

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
			});

		const QSignalBlocker blocker{_editControlsVisibleAction};
		_editControlsVisibleAction->setCheckable(true);
		_editControlsVisibleAction->setChecked(true);
	}

	menu->addSeparator();

	menu->addAction("Previous Camera", this,
		[this]()
		{
			GetCurrentAsset()->OnPreviousCamera();
		},
		QKeySequence{Qt::CTRL + static_cast<int>(Qt::Key::Key_U)});

	menu->addAction("Next Camera", this,
		[this]()
		{
			GetCurrentAsset()->OnNextCamera();
		},
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
			auto cameraOperators = GetCurrentAsset()->GetCameraOperators();

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
			auto cameraOperators = GetCurrentAsset()->GetCameraOperators();

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
			QCDataDialog dialog{this, _editorContext->GetMainWindow()};
			dialog.exec();
		});

	menu->addAction("Dump Model Info...", this, [this]() { GetCurrentAsset()->OnDumpModelInfo(); });

	menu->addSeparator();

	menu->addAction("Take Screenshot...", this, [this]() { GetCurrentAsset()->OnTakeScreenshot(); });
}

bool StudioModelAssetProvider::CanLoad(const QString& fileName, FILE* file) const
{
	return studiomdl::IsStudioModel(file);
}

std::variant<std::unique_ptr<Asset>, AssetLoadInExternalProgram> StudioModelAssetProvider::Load(
	const QString& fileName, FILE* file)
{
	qCDebug(HLAMStudioModel) << "Trying to load model" << fileName;

	const auto filePath = std::filesystem::u8path(fileName.toStdString());
	auto studioModel = studiomdl::LoadStudioModel(filePath, file);

	if (studiomdl::IsXashModel(*studioModel))
	{
		qCDebug(HLAMStudioModel) << "Model" << fileName << "is a Xash model";

		const auto result = _editorContext->TryLaunchExternalProgram(
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

	QString updatedFileName = fileName;

	QFileInfo fileInfo{fileName};

	//Automatically change the name of dol files to mdl to reflect the conversion
	if (fileInfo.suffix() == StudioModelPS2Extension)
	{
		updatedFileName = fileInfo.path() + '/' + fileInfo.completeBaseName() + '.' + StudioModelExtension;
	}

	qCDebug(HLAMStudioModel) << "Loaded model" << fileName << "as" << updatedFileName;

	return std::make_unique<StudioModelAsset>(std::move(updatedFileName), _editorContext, this,
		std::make_unique<studiomdl::EditableStudioModel>(std::move(editableStudioModel)));
}

bool StudioModelAssetProvider::AreEditControlsVisible() const
{
	return _editControlsVisibleAction->isChecked();
}

StudioModelEditWidget* StudioModelAssetProvider::GetEditWidget()
{
	if (!_editWidget)
	{
		assert(_editorContext);
		_editWidget = new StudioModelEditWidget(_editorContext, this);
		_editWidget->RestoreState(_editorContext->GetSettings()->value(WindowStateKey).toByteArray());
	}

	return _editWidget;
}

void StudioModelAssetProvider::OnTick()
{
	_studioModelRenderer->RunFrame();

	emit Tick();
}

void StudioModelAssetProvider::OnActiveAssetChanged(Asset* asset)
{
	if (_currentAsset == GetDummyAsset())
	{
		// Don't let it overwrite the changes made by the new asset.
		const QSignalBlocker assetBlocker{_currentAsset};
		_currentAsset->SetActive(false);
	}

	if (_currentAsset)
	{
		_currentAsset->OnDeactivated();
	}

	_currentAsset = asset && asset->GetProvider() == this ? static_cast<StudioModelAsset*>(asset) : GetDummyAsset();

	if (_currentAsset == GetDummyAsset())
	{
		_currentAsset->SetActive(true);
	}

	emit AssetChanged(_currentAsset);

	_currentAsset->OnActivated();

	_editWidget->setEnabled(_currentAsset != GetDummyAsset());
	_editWidget->setVisible(_currentAsset != GetDummyAsset());
}
}
