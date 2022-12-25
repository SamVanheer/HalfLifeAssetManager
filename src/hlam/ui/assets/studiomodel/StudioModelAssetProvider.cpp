#include <cassert>

#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QSignalBlocker>

#include "formats/sprite/SpriteRenderer.hpp"
#include "formats/studiomodel/StudioModelIO.hpp"
#include "formats/studiomodel/StudioModelRenderer.hpp"
#include "formats/studiomodel/StudioModelUtils.hpp"

#include "qt/QtLogSink.hpp"
#include "qt/QtUtilities.hpp"

#include "ui/EditorContext.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelAssetProvider.hpp"
#include "ui/assets/studiomodel/StudioModelEditWidget.hpp"

#include "ui/assets/studiomodel/compiler/StudioModelCompilerFrontEnd.hpp"
#include "ui/assets/studiomodel/compiler/StudioModelDecompilerFrontEnd.hpp"

#include "ui/settings/GeneralSettings.hpp"
#include "ui/settings/StudioModelSettings.hpp"

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

	menu->addAction("Compile Model...", [=]
		{
			StudioModelCompilerFrontEnd compiler{_editorContext};
	compiler.exec();
		});

	menu->addAction("Decompile Model...", [=]
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
			_editorContext->GetGeneralSettings()->GetXashModelViewerFileName(),
			QStringList(fileName),
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
