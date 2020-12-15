#include <cstdio>
#include <stdexcept>

#include <QAction>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QMessageBox>

#include "engine/shared/studiomodel/DumpModelInfo.hpp"
#include "entity/CHLMVStudioModelEntity.h"
#include "game/entity/CBaseEntity.h"
#include "game/entity/CBaseEntityList.h"
#include "game/entity/CEntityManager.h"

#include "graphics/Scene.hpp"

#include "qt/QtUtilities.hpp"

#include "ui/EditorContext.hpp"
#include "ui/FullscreenWidget.hpp"
#include "ui/SceneWidget.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelEditWidget.hpp"

#include "ui/camera_operators/ArcBallCameraOperator.hpp"
#include "ui/camera_operators/CameraOperator.hpp"

#include "ui/settings/StudioModelSettings.hpp"

#include "utility/IOUtils.h"

namespace ui::assets::studiomodel
{
StudioModelAsset::StudioModelAsset(QString&& fileName,
	EditorContext* editorContext, const StudioModelAssetProvider* provider, std::unique_ptr<studiomdl::CStudioModel>&& studioModel)
	: Asset(std::move(fileName))
	, _editorContext(editorContext)
	, _provider(provider)
	, _studioModel(std::move(studioModel))
	, _scene(std::make_unique<graphics::Scene>(editorContext->GetSoundSystem()))
{
	PushInputSink(this);

	//TODO: need to initialize the background color to its default value here, as specified in the options dialog
	SetBackgroundColor({63, 127, 127});
	_scene->FloorLength = _provider->GetSettings()->GetFloorLength();

	auto entity = static_cast<CHLMVStudioModelEntity*>(_scene->GetEntityContext()->EntityManager->Create("studiomodel", _scene->GetEntityContext(),
		glm::vec3(), glm::vec3(), false));

	if (nullptr != entity)
	{
		entity->SetModel(GetStudioModel());

		entity->Spawn();

		_scene->SetEntity(entity);
	}

	_cameraOperator = std::make_unique<camera_operators::ArcBallCameraOperator>();

	connect(_provider->GetSettings(), &settings::StudioModelSettings::FloorLengthChanged, this, &StudioModelAsset::OnFloorLengthChanged);
}

StudioModelAsset::~StudioModelAsset()
{
	PopInputSink();

	delete _editWidget;
}

void StudioModelAsset::PopulateAssetMenu(QMenu* menu)
{
	menu->addAction("Dump Model Info...", this, &StudioModelAsset::OnDumpModelInfo);

	menu->addAction("Edit QC File...", []
		{
			const QString fileName{QFileDialog::getOpenFileName(nullptr, "Select QC File", {}, "QC files (*.qc);;All Files (*.*)")};

			if (!fileName.isEmpty())
			{
				qt::LaunchDefaultProgram(fileName);
			}
		});

	menu->addAction("Take Screenshot...", this, &StudioModelAsset::OnTakeScreenshot);
}

QWidget* StudioModelAsset::GetEditWidget()
{
	if (_editWidget)
	{
		return _editWidget;
	}

	_editWidget = new StudioModelEditWidget(_editorContext, this);

	_editWidget->connect(_editWidget->GetSceneWidget(), &SceneWidget::MouseEvent, this, &StudioModelAsset::OnSceneWidgetMouseEvent);

	return _editWidget;
}

void StudioModelAsset::SetupFullscreenWidget(EditorContext* editorContext, FullscreenWidget* fullscreenWidget)
{
	const auto sceneWidget = new SceneWidget(GetScene(), fullscreenWidget);

	fullscreenWidget->setCentralWidget(sceneWidget->GetContainer());

	sceneWidget->connect(editorContext, &EditorContext::Tick, sceneWidget, &SceneWidget::requestUpdate);
	sceneWidget->connect(sceneWidget, &SceneWidget::MouseEvent, this, &StudioModelAsset::OnSceneWidgetMouseEvent);

	//Filter key events on the scene widget so we can capture exit even if it has focus
	sceneWidget->installEventFilter(fullscreenWidget);
}

void StudioModelAsset::Save(const QString& fileName)
{
	_provider->Save(fileName, *this);
}

void StudioModelAsset::OnMouseEvent(QMouseEvent* event)
{
	_cameraOperator->MouseEvent(*_editorContext->GetGeneralSettings(), *_scene->GetCamera(), *event);
}

void StudioModelAsset::OnSceneWidgetMouseEvent(QMouseEvent* event)
{
	if (!_inputSinks.empty())
	{
		_inputSinks.top()->OnMouseEvent(event);
	}
}

void StudioModelAsset::OnFloorLengthChanged(int length)
{
	_scene->FloorLength = length;
}

void StudioModelAsset::OnDumpModelInfo()
{
	const QFileInfo fileInfo{GetFileName()};

	const auto suggestedFileName{QString{"%1%2%3_modelinfo.txt"}.arg(fileInfo.path()).arg(QDir::separator()).arg(fileInfo.completeBaseName())};

	const QString fileName{QFileDialog::getSaveFileName(nullptr, {}, suggestedFileName, "Text Files (*.txt);;All Files (*.*)")};

	if (!fileName.isEmpty())
	{
		if (FILE* file = utf8_fopen(fileName.toStdString().c_str(), "w"); file)
		{
			studiomdl::DumpModelInfo(file, *_studioModel);

			fclose(file);

			qt::LaunchDefaultProgram(fileName);
		}
		else
		{
			QMessageBox::critical(nullptr, "Error", QString{"Could not open file \"%1\" for writing"}.arg(fileName));
		}
	}
}

void StudioModelAsset::OnTakeScreenshot()
{
	//Ensure the edit widget exists
	//Should always be the case since the screenshot action is only available if the edit widget is open
	GetEditWidget();

	const QImage screenshot = _editWidget->GetSceneWidget()->grabFramebuffer();

	const QString fileName{QFileDialog::getSaveFileName(nullptr, {}, {}, qt::GetImagesFileFilter())};

	if (!fileName.isEmpty())
	{
		if (!screenshot.save(fileName))
		{
			QMessageBox::critical(nullptr, "Error", "An error occurred while saving screenshot");
		}
	}
}

bool StudioModelAssetProvider::CanLoad(const QString& fileName) const
{
	//TODO:
	return true;
}

std::unique_ptr<Asset> StudioModelAssetProvider::Load(EditorContext* editorContext, const QString& fileName) const
{
	//TODO: this throws specific exceptions. They need to be generalized so the caller can handle them
	auto studioModel = studiomdl::LoadStudioModel(fileName.toStdString().c_str());

	return std::make_unique<StudioModelAsset>(QString{fileName}, editorContext, this, std::move(studioModel));
}

void StudioModelAssetProvider::Save(const QString& fileName, StudioModelAsset& asset) const
{
	//TODO: add setting to correct groups
	studiomdl::SaveStudioModel(fileName.toStdString().c_str(), *asset.GetStudioModel(), false);
}
}
