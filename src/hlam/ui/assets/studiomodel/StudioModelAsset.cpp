#include <algorithm>
#include <cstdio>
#include <stdexcept>
#include <tuple>
#include <vector>

#include <QAction>
#include <QColor>
#include <QDir>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QImage>
#include <QMenu>
#include <QMessageBox>

#include "assets/AssetIO.hpp"

#include "entity/AxesEntity.hpp"
#include "entity/BackgroundEntity.hpp"
#include "entity/BaseEntity.hpp"
#include "entity/BoundingBoxEntity.hpp"
#include "entity/ClippingBoxEntity.hpp"
#include "entity/CrosshairEntity.hpp"
#include "entity/EntityList.hpp"
#include "entity/GroundEntity.hpp"
#include "entity/GuidelinesEntity.hpp"
#include "entity/HLMVStudioModelEntity.hpp"
#include "entity/PlayerHitboxEntity.hpp"
#include "entity/TextureEntity.hpp"

#include "formats/sprite/SpriteRenderer.hpp"
#include "formats/studiomodel/DumpModelInfo.hpp"
#include "formats/studiomodel/IStudioModelRenderer.hpp"
#include "formats/studiomodel/StudioModelIO.hpp"
#include "formats/studiomodel/StudioModelRenderer.hpp"
#include "formats/studiomodel/StudioModelUtils.hpp"

#include "graphics/IGraphicsContext.hpp"
#include "graphics/Scene.hpp"

#include "qt/QtLogSink.hpp"
#include "qt/QtUtilities.hpp"

#include "ui/EditorContext.hpp"
#include "ui/FullscreenWidget.hpp"
#include "ui/SceneWidget.hpp"
#include "ui/StateSnapshot.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelColors.hpp"
#include "ui/assets/studiomodel/StudioModelData.hpp"
#include "ui/assets/studiomodel/StudioModelEditWidget.hpp"
#include "ui/assets/studiomodel/StudioModelUndoCommands.hpp"
#include "ui/assets/studiomodel/compiler/StudioModelCompilerFrontEnd.hpp"
#include "ui/assets/studiomodel/compiler/StudioModelDecompilerFrontEnd.hpp"

#include "ui/camera_operators/ArcBallCameraOperator.hpp"
#include "ui/camera_operators/CameraOperator.hpp"
#include "ui/camera_operators/CameraOperators.hpp"
#include "ui/camera_operators/FirstPersonCameraOperator.hpp"
#include "ui/camera_operators/FreeLookCameraOperator.hpp"
#include "ui/camera_operators/TextureCameraOperator.hpp"

#include "ui/settings/ColorSettings.hpp"
#include "ui/settings/StudioModelSettings.hpp"

#include "utility/IOUtils.hpp"
#include "utility/mathlib.hpp"

namespace ui::assets::studiomodel
{
Q_LOGGING_CATEGORY(HLAMStudioModel, "hlam.studiomodel")

static std::tuple<glm::vec3, glm::vec3, float, float> GetCenteredValues(const HLMVStudioModelEntity& entity, Axis axis, bool positive)
{
	glm::vec3 min, max;
	entity.ExtractBbox(min, max);

	//Clamp the values to a reasonable range
	for (int i = 0; i < 3; ++i)
	{
		//Use different limits for min and max so centering won't end up setting origin to 0 0 0
		min[i] = std::clamp(min[i], -2000.f, 2000.f);
		max[i] = std::clamp(max[i], -1000.f, 1000.f);
	}

	const glm::vec3 size = max - min;

	const float distance{std::max({size.x, size.y, size.z})};

	const int axisIndex = static_cast<int>(axis);

	const int positiveValue = positive ? 1 : -1;

	const glm::vec3 targetOrigin{0, 0, min.z + (size.z / 2)};

	//Set origin to distance on axis, negate if needed
	//Offset camera origin to be relative to target
	glm::vec3 cameraOrigin{targetOrigin};

	cameraOrigin[axisIndex] += distance * positiveValue;

	//Look back down the axis
	float pitch = 0;
	float yaw = 0;

	switch (axis)
	{
	case Axis::X:
		yaw = positive ? 180 : 0;
		break;

	case Axis::Y:
		yaw = positive ? 90 : 270;
		break;

	case Axis::Z:
		pitch = positive ? -90 : 90;
		yaw = 180;
		break;
	}

	return {targetOrigin, cameraOrigin, pitch, yaw};
}

StudioModelAsset::StudioModelAsset(QString&& fileName,
	EditorContext* editorContext, const StudioModelAssetProvider* provider,
	std::unique_ptr<studiomdl::EditableStudioModel>&& editableStudioModel)
	: Asset(std::move(fileName))
	, _editorContext(editorContext)
	, _provider(provider)
	, _editableStudioModel(std::move(editableStudioModel))
	, _modelData(new StudioModelData(this))
	, _textureLoader(_editorContext->GetTextureLoader())
	, _studioModelRenderer(std::make_unique<studiomdl::StudioModelRenderer>(
		CreateQtLoggerSt(logging::HLAMStudioModelRenderer()), _editorContext->GetOpenGLFunctions(), _editorContext->GetColorSettings()))
	, _spriteRenderer(std::make_unique<sprite::SpriteRenderer>(
		CreateQtLoggerSt(logging::HLAMSpriteRenderer()), _editorContext->GetWorldTime()))
	, _entityContext(std::make_unique<EntityContext>(this,
		_editorContext->GetWorldTime(),
		_studioModelRenderer.get(),
		_spriteRenderer.get(),
		_editorContext->GetSoundSystem(),
		_editorContext->GetGeneralSettings(),
		_provider->GetStudioModelSettings()))
	, _cameraOperators(new camera_operators::CameraOperators(this))
{
	_modelData->Initialize(_editableStudioModel.get());

	connect(_cameraOperators, &camera_operators::CameraOperators::CameraChanged, this, &StudioModelAsset::OnCameraChanged);

	CreateMainScene();
	CreateTextureScene();

	auto arcBallCamera = new camera_operators::ArcBallCameraOperator(_editorContext->GetGeneralSettings());
	_firstPersonCamera = new camera_operators::FirstPersonCameraOperator(_editorContext->GetGeneralSettings());

	_cameraOperators->Add(arcBallCamera);
	_cameraOperators->Add(new camera_operators::FreeLookCameraOperator(_editorContext->GetGeneralSettings()));
	_cameraOperators->Add(_firstPersonCamera);

	const auto [targetOrigin, cameraOrigin, pitch, yaw] = GetCenteredValues(*_modelEntity, Axis::X, true);

	for (int i = 0; i < _cameraOperators->Count(); ++i)
	{
		const auto cameraOperator = _cameraOperators->Get(i);
		cameraOperator->CenterView(targetOrigin, cameraOrigin, pitch, yaw);
		//Set initial restoration point to the initial camera view
		cameraOperator->SaveView();
	}

	auto studioModelSettings = _provider->GetStudioModelSettings();

	if (studioModelSettings->ShouldAutodetectViewmodels() && QFileInfo{GetFileName()}.fileName().startsWith("v_"))
	{
		_cameraOperators->SetCurrent(_firstPersonCamera);
	}
	else
	{
		_cameraOperators->SetCurrent(arcBallCamera);
	}

	connect(_editorContext, &EditorContext::Tick, this, &StudioModelAsset::OnTick);

	connect(_editorContext->GetGeneralSettings(), &ui::settings::GeneralSettings::ResizeTexturesToPowerOf2Changed,
		this, &StudioModelAsset::OnResizeTexturesToPowerOf2Changed);
	connect(_editorContext->GetGeneralSettings(), &ui::settings::GeneralSettings::TextureFiltersChanged,
		this, &StudioModelAsset::OnTextureFiltersChanged);

	// Initialize graphics resources.
	auto context = _editorContext->GetGraphicsContext();
	context->Begin();

	for (auto scene : _scenes)
	{
		scene->CreateDeviceObjects();
	}

	context->End();
}

StudioModelAsset::~StudioModelAsset()
{
	auto context = _editorContext->GetGraphicsContext();

	context->Begin();

	for (auto it = _scenes.rbegin(), end = _scenes.rend(); it != end; ++it)
	{
		(*it)->DestroyDeviceObjects();
	}

	context->End();

	delete _editWidget;
}

void StudioModelAsset::PopulateAssetMenu(QMenu* menu)
{
	{
		auto panelsMenu = menu->addMenu("Panels");

		for (auto dock : _editWidget->GetDockWidgets())
		{
			panelsMenu->addAction(dock->toggleViewAction());
		}
	}

	menu->addAction("Previous Camera", this, &StudioModelAsset::OnPreviousCamera, QKeySequence{Qt::CTRL + Qt::Key::Key_U});
	menu->addAction("Next Camera", this, &StudioModelAsset::OnNextCamera, QKeySequence{Qt::CTRL + Qt::Key::Key_I});

	menu->addSeparator();

	{
		auto centerMenu = menu->addMenu("Center View");

		centerMenu->addAction("Center On Positive X", [this]() { OnCenterView(Axis::X, true); });
		centerMenu->addAction("Center On Negative X", [this]() { OnCenterView(Axis::X, false); });

		//TODO: camera position doesn't match what's shown by "Draw Axes" on the Y axis
		centerMenu->addAction("Center On Positive Y", [this]() { OnCenterView(Axis::Y, false); });
		centerMenu->addAction("Center On Negative Y", [this]() { OnCenterView(Axis::Y, true); });

		centerMenu->addAction("Center On Positive Z", [this]() { OnCenterView(Axis::Z, true); });
		centerMenu->addAction("Center On Negative Z", [this]() { OnCenterView(Axis::Z, false); });
	}

	menu->addAction("Save View", this, &StudioModelAsset::OnSaveView);
	menu->addAction("Restore View", this, &StudioModelAsset::OnRestoreView);

	menu->addSeparator();

	menu->addAction("Flip Normals", this, &StudioModelAsset::OnFlipNormals);

	menu->addSeparator();

	menu->addAction("Dump Model Info...", this, &StudioModelAsset::OnDumpModelInfo);

	menu->addSeparator();

	menu->addAction("Take Screenshot...", this, &StudioModelAsset::OnTakeScreenshot);
}

QWidget* StudioModelAsset::GetEditWidget()
{
	if (_editWidget)
	{
		return _editWidget;
	}

	_editWidget = new StudioModelEditWidget(_editorContext, this);

	auto sceneWidget = _editWidget->GetSceneWidget();

	_editWidget->connect(sceneWidget, &SceneWidget::MouseEvent, this, &StudioModelAsset::OnSceneWidgetMouseEvent);
	_editWidget->connect(sceneWidget, &SceneWidget::WheelEvent, this, &StudioModelAsset::OnSceneWidgetWheelEvent);

	sceneWidget->SetScene(GetScene());

	// TODO: needs to be moved once single instance has been implemented.
	emit AssetChanged(this);

	return _editWidget;
}

void StudioModelAsset::SetupFullscreenWidget(FullscreenWidget* fullscreenWidget)
{
	const auto sceneWidget = new SceneWidget(fullscreenWidget);

	sceneWidget->SetScene(GetScene());

	fullscreenWidget->setCentralWidget(sceneWidget->GetContainer());

	//sceneWidget->connect(this, &StudioModelAsset::Draw, sceneWidget, &SceneWidget::requestUpdate);
	sceneWidget->connect(sceneWidget, &SceneWidget::MouseEvent, this, &StudioModelAsset::OnSceneWidgetMouseEvent);

	//Filter key events on the scene widget so we can capture exit even if it has focus
	sceneWidget->installEventFilter(fullscreenWidget);
}

void StudioModelAsset::Save()
{
	const auto filePath = std::filesystem::u8path(GetFileName().toStdString());
	auto result = studiomdl::ConvertFromEditable(filePath, *_editableStudioModel);

	studiomdl::SaveStudioModel(filePath, result, false);
}

void StudioModelAsset::TryRefresh()
{
	auto snapshot = std::make_unique<StateSnapshot>();

	SaveEntityToSnapshot(snapshot.get());

	emit SaveSnapshot(snapshot.get());

	try
	{
		const auto filePath = std::filesystem::u8path(GetFileName().toStdString());
		auto studioModel = studiomdl::LoadStudioModel(filePath, nullptr);

		auto newModel = std::make_unique<studiomdl::EditableStudioModel>(studiomdl::ConvertToEditable(*studioModel));

		// Clear UI to null state so changes to the models don't trigger changes in UI slots.
		emit AssetChanged(nullptr);

		auto context = _editorContext->GetGraphicsContext();
		context->Begin();

		//Clean up old model resources
		//TODO: needs to be handled better
		_modelEntity->DestroyDeviceObjects(_editorContext->GetOpenGLFunctions(), *_textureLoader);

		_editableStudioModel = std::move(newModel);

		_modelData->Initialize(_editableStudioModel.get());

		GetUndoStack()->clear();

		_modelEntity->SetEditableModel(GetEditableStudioModel());
		_modelEntity->Spawn();

		_modelEntity->CreateDeviceObjects(_editorContext->GetOpenGLFunctions(), *_textureLoader);

		context->End();

		emit AssetChanged(this);
	}
	catch (const ::assets::AssetException& e)
	{
		QMessageBox::critical(nullptr, "Error", QString{"An error occurred while reloading the model \"%1\":\n%2"}.arg(GetFileName()).arg(e.what()));
		return;
	}

	LoadEntityFromSnapshot(snapshot.get());

	emit LoadSnapshot(snapshot.get());
}

void StudioModelAsset::SetCurrentScene(graphics::Scene* scene)
{
	assert(_scenes.contains(scene));

	_editWidget->SetSceneIndex(_scenes.indexOf(scene));
}

soundsystem::ISoundSystem* StudioModelAsset::GetSoundSystem()
{
	return _editorContext->GetSoundSystem();
}

void StudioModelAsset::CreateMainScene()
{
	_scene = std::make_unique<graphics::Scene>("Scene", _editorContext->GetGraphicsContext(), _editorContext->GetOpenGLFunctions(),
		_textureLoader, _entityContext.get());

	_scenes.push_back(_scene.get());

	// The order that entities are added matters for now since there's no sorting done.
	_backgroundEntity = _scene->GetEntityList()->Create<BackgroundEntity>();
	_scene->GetEntityList()->Create<AxesEntity>();
	_modelEntity = _scene->GetEntityList()->Create<HLMVStudioModelEntity>(GetEditableStudioModel());
	_groundEntity = _scene->GetEntityList()->Create<GroundEntity>();
	_scene->GetEntityList()->Create<PlayerHitboxEntity>();
	_scene->GetEntityList()->Create<BoundingBoxEntity>();
	_scene->GetEntityList()->Create<ClippingBoxEntity>();
	_scene->GetEntityList()->Create<CrosshairEntity>();
	_scene->GetEntityList()->Create<GuidelinesEntity>();
}

void StudioModelAsset::CreateTextureScene()
{
	_textureScene = std::make_unique<graphics::Scene>("Texture", _editorContext->GetGraphicsContext(), _editorContext->GetOpenGLFunctions(),
		_textureLoader, _entityContext.get());

	_scenes.push_back(_textureScene.get());

	// The order that entities are added matters for now since there's no sorting done.
	_textureEntity = _textureScene->GetEntityList()->Create<TextureEntity>();

	_textureCameraOperator = std::make_unique<camera_operators::TextureCameraOperator>(_textureEntity);

	_textureScene->SetCurrentCamera(_textureCameraOperator.get());
}

void StudioModelAsset::SaveEntityToSnapshot(StateSnapshot* snapshot)
{
	auto model = _modelEntity->GetEditableModel();

	const int sequenceIndex = _modelEntity->GetSequence();

	if (sequenceIndex >= 0 && sequenceIndex < model->Sequences.size())
	{
		snapshot->SetValue("entity.sequence", QVariant::fromValue(QString::fromStdString(model->Sequences[sequenceIndex]->Label)));
		snapshot->SetValue("entity.frame", QVariant::fromValue(_modelEntity->GetFrame()));
	}

	snapshot->SetValue("entity.skin", QVariant::fromValue(_modelEntity->GetSkin()));
	snapshot->SetValue("entity.body", QVariant::fromValue(_modelEntity->GetBodygroup()));

	for (int i = 0; i < STUDIO_MAX_CONTROLLERS; ++i)
	{
		snapshot->SetValue(QString{"entity.bonecontroller%1"}.arg(i), QVariant::fromValue(_modelEntity->GetControllerValue(i)));
	}

	snapshot->SetValue("entity.mouth", QVariant::fromValue(_modelEntity->GetMouth()));

	for (int i = 0; i < STUDIO_MAX_BLENDERS; ++i)
	{
		snapshot->SetValue(QString{"entity.blender%1"}.arg(i), QVariant::fromValue(_modelEntity->GetBlendingValue(i)));
	}
}

void StudioModelAsset::LoadEntityFromSnapshot(StateSnapshot* snapshot)
{
	auto model = _modelEntity->GetEditableModel();

	bool foundSequence = false;

	if (auto sequence = snapshot->Value("entity.sequence"); sequence.isValid())
	{
		auto sequenceName = sequence.toString().toStdString();

		if (auto it = std::find_if(model->Sequences.begin(), model->Sequences.end(), [&](const auto& sequence)
			{
				return sequence->Label == sequenceName;
			}); it != model->Sequences.end())
		{
			const auto index = it - model->Sequences.begin();

			_modelEntity->SetSequence(static_cast<int>(index));

			//Only set these if we were able to find the sequence
			_modelEntity->SetFrame(snapshot->Value("entity.frame").toFloat());

			foundSequence = true;
		}
	}

	if (!foundSequence)
	{
		//Reset to default
		_modelEntity->SetSequence(0);
	}

	_modelEntity->SetSkin(snapshot->Value("entity.skin").toInt());
	_modelEntity->SetCompoundBodyValue(snapshot->Value("entity.body").toInt());

	for (int i = 0; i < STUDIO_MAX_CONTROLLERS; ++i)
	{
		_modelEntity->SetController(i, snapshot->Value(QString{"entity.bonecontroller%1"}.arg(i)).toFloat());
	}

	_modelEntity->SetMouth(snapshot->Value("entity.mouth").toFloat());

	for (int i = 0; i < STUDIO_MAX_BLENDERS; ++i)
	{
		_modelEntity->SetBlending(i, snapshot->Value(QString{"entity.blender%1"}.arg(i)).toFloat());
	}
}

void StudioModelAsset::OnTick()
{
	//TODO: update asset-local world time
	_studioModelRenderer->RunFrame();

	for (auto scene : _scenes)
	{
		scene->Tick();
	}

	emit Tick();
}

void StudioModelAsset::OnResizeTexturesToPowerOf2Changed()
{
	auto context = _editorContext->GetGraphicsContext();

	context->Begin();
	_editableStudioModel->UpdateTextures(*_textureLoader);
	context->End();
}

void StudioModelAsset::OnTextureFiltersChanged()
{
	auto context = _editorContext->GetGraphicsContext();

	context->Begin();
	_editableStudioModel->UpdateFilters(*_textureLoader);
	context->End();
}

void StudioModelAsset::OnSceneWidgetMouseEvent(QMouseEvent* event)
{
	if (auto scene = _editWidget->GetCurrentScene(); scene)
	{
		if (auto cameraOperator = scene->GetCurrentCamera(); cameraOperator)
		{
			cameraOperator->MouseEvent(*event);
		}
	}
}

void StudioModelAsset::OnSceneWidgetWheelEvent(QWheelEvent* event)
{
	if (auto scene = _editWidget->GetCurrentScene(); scene)
	{
		if (auto cameraOperator = scene->GetCurrentCamera(); cameraOperator)
		{
			cameraOperator->WheelEvent(*event);
		}
	}
}

void StudioModelAsset::OnCameraChanged(camera_operators::SceneCameraOperator* previous, camera_operators::SceneCameraOperator* current)
{
	_scene->SetCurrentCamera(current);
	GetProvider()->GetStudioModelSettings()->CameraIsFirstPerson = current == _firstPersonCamera;
}

void StudioModelAsset::OnPreviousCamera()
{
	_cameraOperators->PreviousCamera();
}

void StudioModelAsset::OnNextCamera()
{
	_cameraOperators->NextCamera();
}

void StudioModelAsset::OnCenterView(Axis axis, bool positive)
{
	if (auto cameraOperator = _cameraOperators->GetCurrent(); cameraOperator)
	{
		const auto [targetOrigin, cameraOrigin, pitch, yaw] = GetCenteredValues(*_modelEntity, axis, positive);

		cameraOperator->CenterView(targetOrigin, cameraOrigin, pitch, yaw);
	}
}

void StudioModelAsset::OnSaveView()
{
	if (auto cameraOperator = _cameraOperators->GetCurrent(); cameraOperator)
	{
		cameraOperator->SaveView();
	}
}

void StudioModelAsset::OnRestoreView()
{
	if (auto cameraOperator = _cameraOperators->GetCurrent(); cameraOperator)
	{
		cameraOperator->RestoreView();
	}
}

void StudioModelAsset::OnFlipNormals()
{
	std::vector<glm::vec3> oldNormals;
	std::vector<glm::vec3> newNormals;

	auto model = _modelEntity->GetEditableModel();

	for (auto& bodypart : model->Bodyparts)
	{
		for (auto& model : bodypart->Models)
		{
			oldNormals.reserve(oldNormals.size() + model.Normals.size());
			newNormals.reserve(newNormals.size() + model.Normals.size());

			for (auto& normal : model.Normals)
			{
				oldNormals.push_back(normal.Vertex);
				newNormals.push_back(-normal.Vertex);
			}
		}
	}

	AddUndoCommand(new FlipNormalsCommand(this, std::move(oldNormals), std::move(newNormals)));
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
			studiomdl::DumpModelInfo(file, *_editableStudioModel);

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

StudioModelAssetProvider::~StudioModelAssetProvider() = default;

QMenu* StudioModelAssetProvider::CreateToolMenu(EditorContext* editorContext)
{
	auto menu = new QMenu("StudioModel");

	menu->addAction("Compile Model...", [=]
		{
			StudioModelCompilerFrontEnd compiler{editorContext, GetStudioModelSettings()};
			compiler.exec();
		});

	menu->addAction("Decompile Model...", [=]
		{
			StudioModelDecompilerFrontEnd decompiler{editorContext, GetStudioModelSettings()};
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

std::unique_ptr<Asset> StudioModelAssetProvider::Load(EditorContext* editorContext, const QString& fileName, FILE* file) const
{
	qCDebug(HLAMStudioModel) << "Trying to load model" << fileName;

	const auto filePath = std::filesystem::u8path(fileName.toStdString());
	auto studioModel = studiomdl::LoadStudioModel(filePath, file);

	auto editableStudioModel = studiomdl::ConvertToEditable(*studioModel);

	QString updatedFileName = fileName;

	QFileInfo fileInfo{fileName};

	//Automatically change the name of dol files to mdl to reflect the conversion
	if (fileInfo.suffix() == StudioModelPS2Extension)
	{
		updatedFileName = fileInfo.path() + '/' + fileInfo.completeBaseName() + '.' + StudioModelExtension;
	}

	qCDebug(HLAMStudioModel) << "Loaded model" << fileName << "as" << updatedFileName;

	return std::make_unique<StudioModelAsset>(std::move(updatedFileName), editorContext, this,
		std::make_unique<studiomdl::EditableStudioModel>(std::move(editableStudioModel)));
}
}
