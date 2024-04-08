#include <algorithm>
#include <tuple>
#include <vector>

#include <QDir>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QOpenGLFramebufferObject>
#include <QImage>
#include <QMenu>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QWidget>

#include "application/AssetIO.hpp"

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

#include "filesystem/IFileSystem.hpp"

#include "formats/studiomodel/StudioModelIO.hpp"
#include "formats/studiomodel/StudioModelUtils.hpp"

#include "graphics/IGraphicsContext.hpp"
#include "graphics/Scene.hpp"
#include "graphics/SceneContext.hpp"

#include "plugins/halflife/studiomodel/StudioModelAsset.hpp"
#include "plugins/halflife/studiomodel/StudioModelAssetProvider.hpp"
#include "plugins/halflife/studiomodel/StudioModelColors.hpp"
#include "plugins/halflife/studiomodel/ui/StudioModelData.hpp"
#include "plugins/halflife/studiomodel/ui/StudioModelEditWidget.hpp"
#include "plugins/halflife/studiomodel/ui/StudioModelUndoCommands.hpp"

#include "qt/QtLogSink.hpp"
#include "qt/QtUtilities.hpp"

#include "settings/StudioModelSettings.hpp"

#include "soundsystem/SoundSystem.hpp"

#include "application/AssetManager.hpp"
#include "ui/MainWindow.hpp"
#include "ui/SceneWidget.hpp"

#include "ui/camera_operators/CameraOperators.hpp"
#include "ui/camera_operators/TextureCameraOperator.hpp"

namespace studiomodel
{
static std::tuple<glm::vec3, glm::vec3, float, float> GetCenteredValues(
	const HLMVStudioModelEntity& entity, Axis axis, bool positive)
{
	glm::vec3 min, max;
	entity.ExtractBbox(min, max);

	const glm::vec3 size = max - min;

	const glm::vec3 targetOrigin{0, 0, min.z + (size.z / 2)};

	//Set origin to distance on axis, negate if needed
	//Offset camera origin to be relative to target
	glm::vec3 cameraOrigin{targetOrigin};

	const float distance{std::clamp(std::max({size.x, size.y, size.z}), -1000.f, 1000.f)};

	cameraOrigin[static_cast<int>(axis)] += distance * (positive ? 1 : -1);

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
	AssetManager* application, StudioModelAssetProvider* provider, unsigned int settingsVersion,
	std::unique_ptr<studiomdl::EditableStudioModel>&& editableStudioModel,
	std::unique_ptr<IFileSystem> fileSystem)
	: Asset(std::move(fileName))
	, _application(application)
	, _provider(provider)
	, _editableStudioModel(std::move(editableStudioModel))
	, _modelData(new StudioModelData(_editableStudioModel.get(), this))
	, _fileSystem(std::move(fileSystem))
	, _soundSystem(std::make_unique<SoundSystemWrapper>(_application->GetSoundSystem(), _fileSystem.get()))
	, _entityContext(std::make_unique<EntityContext>(this,
		_application->GetWorldTime(),
		_provider->GetStudioModelRenderer(),
		_provider->GetSpriteRenderer(),
		_soundSystem.get(),
		_application->GetApplicationSettings(),
		_provider->GetStudioModelSettings()))
	, _settingsVersion(settingsVersion)
{
	CreateMainScene();
	CreateTextureScene();

	connect(this, &StudioModelAsset::FileNameChanged, this, &StudioModelAsset::UpdateFileSystem);

	connect(_application->GetApplicationSettings(), &ApplicationSettings::ResizeTexturesToPowerOf2Changed,
		this, &StudioModelAsset::OnResizeTexturesToPowerOf2Changed);
	connect(_application->GetApplicationSettings(), &ApplicationSettings::TextureFiltersChanged,
		this, &StudioModelAsset::OnTextureFiltersChanged);

	// Initialize graphics resources.
	{
		graphics::SceneContext sc{_application->GetOpenGLFunctions(), _application->GetTextureLoader()};

		auto context = _application->GetGraphicsContext();
		context->Begin();

		for (auto scene : _scenes)
		{
			scene->CreateDeviceObjects(sc);
		}

		context->End();
	}

	SetCurrentScene(GetScene());
}

StudioModelAsset::~StudioModelAsset()
{
	{
		graphics::SceneContext sc{_application->GetOpenGLFunctions(), _application->GetTextureLoader()};

		auto context = _application->GetGraphicsContext();

		context->Begin();

		for (auto it = _scenes.rbegin(), end = _scenes.rend(); it != end; ++it)
		{
			(*it)->DestroyDeviceObjects(sc);
		}

		context->End();
	}
}

QWidget* StudioModelAsset::GetEditWidget()
{
	return _provider->GetEditWidget();
}

void StudioModelAsset::Save()
{
	if (_editableStudioModel->IsXashModel)
	{
		const auto action = QMessageBox::question(_application->GetMainWindow(),
			"Possible loss of data", R"(This model was created using Xash's model compiler.
Saving it will remove Xash-specific data.

Continue?)", QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

		if (action != QMessageBox::Yes)
		{
			return;
		}

		// After saving Xash-specific data is lost so this is no longer true.
		_editableStudioModel->IsXashModel = false;
	}

	// Once saved this is never true.
	_editableStudioModel->HasExternalTextureFile = false;

	const auto filePath = std::filesystem::u8path(GetFileName().toStdString());
	auto result = studiomdl::ConvertFromEditable(filePath, *_editableStudioModel);

	studiomdl::SaveStudioModel(filePath, result);

	auto undoStack = GetUndoStack();

	undoStack->setClean();
}

std::unique_ptr<studiomdl::EditableStudioModel> TryReloadModel(const QString& fileName, IFileSystem& fileSystem)
{
	try
	{
		const std::string filePathString = fileName.toStdString();
		const auto filePath = std::filesystem::u8path(filePathString);

		auto file = fileSystem.TryOpenAbsolute(filePathString, true, true);

		if (!file)
		{
			throw AssetException("Could not open asset: file no longer exists or is currently opened by another program");
		}

		auto studioModel = studiomdl::LoadStudioModel(filePath, file.get(), fileSystem);

		return std::make_unique<studiomdl::EditableStudioModel>(studiomdl::ConvertToEditable(*studioModel));
	}
	catch (const AssetException& e)
	{
		QMessageBox::critical(nullptr, "Error",
			QString{ "An error occurred while reloading the model \"%1\":\n%2" }.arg(fileName).arg(e.what()));
		return {};
	}
}

bool StudioModelAsset::TryRefresh()
{
	auto newModel = TryReloadModel(GetFileName(), *_fileSystem);

	if (!newModel)
	{
		return false;
	}

	auto snapshot = std::make_unique<StateSnapshot>();

	SaveEntityToSnapshot(snapshot.get());

	emit SaveSnapshot(snapshot.get());

	// Clear UI to null state so changes to the models don't trigger changes in UI slots.
	emit _provider->AssetChanged(_provider->GetDummyAsset());

	graphics::SceneContext sc{_application->GetOpenGLFunctions(), _application->GetTextureLoader()};

	auto context = _application->GetGraphicsContext();
	context->Begin();

	//Clean up old model resources
	//TODO: needs to be handled better
	_modelEntity->DestroyDeviceObjects(sc);

	_editableStudioModel = std::move(newModel);

	auto oldModelData = _modelData;

	_modelData = new StudioModelData(_editableStudioModel.get(), this);

	GetUndoStack()->clear();

	_modelEntity->SetEditableModel(GetEditableStudioModel());
	_modelEntity->Spawn();

	LoadEntityFromSnapshot(snapshot.get());

	_modelEntity->CreateDeviceObjects(sc);

	context->End();

	emit _provider->AssetChanged(this);

	// Delete the old data now that any remaining references have been cleared.
	delete oldModelData;

	emit LoadSnapshot(snapshot.get());

	return true;
}

bool StudioModelAsset::CanTakeScreenshot() const
{
	return true;
}

void StudioModelAsset::TakeScreenshot()
{
	auto graphicsContext = _application->GetGraphicsContext();

	graphicsContext->Begin();

	QImage fboImage;
	QImage screenshot;
	
	bool grabScreenshotFromMainScene = true;
	bool fbIsValid = true;

	if (_application->GetApplicationSettings()->TransparentScreenshots)
	{
		auto sceneWidget = _application->GetSceneWidget();

		// Create a temporary framebuffer to render the scene with transparency.
		QOpenGLFramebufferObjectFormat format;

		format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
		format.setSamples(sceneWidget->format().samples());

		auto buffer = std::make_unique<QOpenGLFramebufferObject>(sceneWidget->size(), format);

		fbIsValid = buffer->isValid() && buffer->bind();

		if (fbIsValid)
		{
			GetScene()->Draw(*sceneWidget->GetSceneContext(), glm::vec4{ 0, 0, 0, 0 });

			// Create a wrapper to handle alpha channel correctly
			// See https://doc.qt.io/qt-5/qopenglframebufferobject.html#toImage-1 for more information
			// 'screenshot' does not copy image data so 'fboImage' must live at least as long!
			fboImage = QImage(buffer->toImage());
			screenshot = QImage(fboImage.constBits(), fboImage.width(), fboImage.height(), QImage::Format_ARGB32);

			buffer->release();

			grabScreenshotFromMainScene = false;
		}
	}

	if (grabScreenshotFromMainScene)
	{
		screenshot = _application->GetSceneWidget()->grabFramebuffer();
	}

	// Show error after taking screenshot to avoid scene changing during messagebox display.
	if (!fbIsValid)
	{
		QMessageBox::critical(nullptr, "Error", R"(Error creating OpenGL framebuffer to render transparent screenshot

The screenshot will be rendered without transparency)");
	}

	graphicsContext->End();

	const QString fileName{ QFileDialog::getSaveFileName(nullptr, {}, {}, qt::GetImagesFileFilter()) };

	if (!fileName.isEmpty())
	{
		if (!screenshot.save(fileName))
		{
			QMessageBox::critical(nullptr, "Error", "An error occurred while saving screenshot");
		}
	}
}

graphics::IGraphicsContext* StudioModelAsset::GetGraphicsContext()
{
	return _application->GetGraphicsContext();
}

graphics::TextureLoader* StudioModelAsset::GetTextureLoader()
{
	return _application->GetTextureLoader();
}

studiomdl::StudioModelRenderer* StudioModelAsset::GetStudioModelRenderer() const
{
	return GetProvider()->GetStudioModelRenderer();
}

void StudioModelAsset::SetCurrentScene(graphics::Scene* scene)
{
	const auto it = std::find(_scenes.begin(), _scenes.end(), scene);

	assert(it != _scenes.end());

	const int index = it != _scenes.end() ? it - _scenes.begin() : 0;

	_currentScene = _scenes[index];

	if (_provider->GetCurrentAsset() == this)
	{
		auto editWidget = _provider->GetEditWidget();
		editWidget->SetSceneIndex(index);
		_application->GetSceneWidget()->SetScene(_currentScene);
	}
}

void StudioModelAsset::UpdateSettingsState()
{
	UpdateFileSystem();
}

void StudioModelAsset::UpdateFileSystem()
{
	_application->InitializeFileSystem(*_fileSystem, GetFileName());
}

bool StudioModelAsset::CameraIsFirstPerson() const
{
	return _provider->CameraIsFirstPerson();
}

void StudioModelAsset::OnActivated()
{
	auto editWidget = _provider->GetEditWidget();

	{
		const QSignalBlocker editBlocker{editWidget};
		editWidget->SetAsset(this);
	}

	connect(_application, &AssetManager::SceneWidgetRecreated, this, &StudioModelAsset::OnSceneWidgetRecreated);
	connect(_application, &AssetManager::FullscreenModeChanged, this, &StudioModelAsset::OnSceneWidgetRecreated);
	connect(editWidget, &StudioModelEditWidget::SceneIndexChanged, this, &StudioModelAsset::OnSceneIndexChanged);
	connect(editWidget, &StudioModelEditWidget::PoseChanged, this, &StudioModelAsset::SetPose);

	OnSceneWidgetRecreated();

	emit LoadSnapshot(&_snapshot);

	connect(_provider->GetCameraOperators(), &CameraOperators::CameraChanged,
		this, &StudioModelAsset::OnCameraChanged, Qt::UniqueConnection);

	if (_isInitialized)
	{
		_provider->GetCameraOperators()->RestoreViews(_cameraViewStates);
		_provider->GetCameraOperators()->SetCurrent(_provider->GetCameraOperators()->Get(_activeCamera));
	}
	else
	{
		_isInitialized = true;

		const auto [targetOrigin, cameraOrigin, pitch, yaw] = GetCenteredValues(*_modelEntity, Axis::X, true);

		auto cameraOperators = _provider->GetCameraOperators();

		for (int i = 0; i < cameraOperators->Count(); ++i)
		{
			const auto cameraOperator = cameraOperators->Get(i);
			cameraOperator->CenterView(targetOrigin, cameraOrigin, pitch, yaw);
		}

		auto studioModelSettings = _provider->GetStudioModelSettings();

		if (studioModelSettings->ShouldAutodetectViewmodels()
			&& QFileInfo{GetFileName()}.fileName().startsWith("v_", Qt::CaseInsensitive))
		{
			cameraOperators->SetCurrent(_provider->GetFirstPersonCameraOperator());
		}
		else
		{
			cameraOperators->SetCurrent(_provider->GetArcBallCameraOperator());
		}

		_activeCamera = _provider->GetCameraOperators()->IndexOf(_provider->GetCameraOperators()->GetCurrent());
	}

	OnCameraChanged(nullptr, _provider->GetCameraOperators()->GetCurrent());
}

void StudioModelAsset::OnDeactivated()
{
	emit SaveSnapshot(&_snapshot);

	_cameraViewStates = _provider->GetCameraOperators()->SaveViews();
	_activeCamera = _provider->GetCameraOperators()->IndexOf(_provider->GetCameraOperators()->GetCurrent());

	disconnect(_provider->GetCameraOperators(), &CameraOperators::CameraChanged,
		this, &StudioModelAsset::OnCameraChanged);

	auto editWidget = _provider->GetEditWidget();
	auto sceneWidget = _application->GetSceneWidget();

	editWidget->disconnect(this);
	sceneWidget->disconnect(this);

	disconnect(_application, &AssetManager::SceneWidgetRecreated,
		this, &StudioModelAsset::OnSceneWidgetRecreated);
	disconnect(_application, &AssetManager::FullscreenModeChanged,
		this, &StudioModelAsset::OnSceneWidgetRecreated);
}

void StudioModelAsset::CreateMainScene()
{
	_scene = std::make_unique<graphics::Scene>("Scene", _entityContext.get());

	_scenes.push_back(_scene.get());

	_backgroundEntity = _scene->GetEntityList()->Create<BackgroundEntity>();
	_axesEntity = _scene->GetEntityList()->Create<AxesEntity>();
	_modelEntity = _scene->GetEntityList()->Create<HLMVStudioModelEntity>(GetEditableStudioModel());
	_groundEntity = _scene->GetEntityList()->Create<GroundEntity>();
	_playerHitboxEntity = _scene->GetEntityList()->Create<PlayerHitboxEntity>();
	_boundingBoxEntity = _scene->GetEntityList()->Create<BoundingBoxEntity>();
	_clippingBoxEntity = _scene->GetEntityList()->Create<ClippingBoxEntity>();
	_crosshairEntity = _scene->GetEntityList()->Create<CrosshairEntity>();
	_guidelinesEntity = _scene->GetEntityList()->Create<GuidelinesEntity>();
}

void StudioModelAsset::CreateTextureScene()
{
	_textureScene = std::make_unique<graphics::Scene>("Texture", _entityContext.get());

	_scenes.push_back(_textureScene.get());

	// The order that entities are added matters for now since there's no sorting done.
	_textureEntity = _textureScene->GetEntityList()->Create<TextureEntity>();

	_textureCameraOperator = std::make_unique<TextureCameraOperator>(_textureEntity);

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

	snapshot->SetValue("entity.model.topcolor", model->TopColor);
	snapshot->SetValue("entity.model.bottomcolor", model->BottomColor);
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

	_modelEntity->SetMouth(snapshot->Value("entity.mouth").toInt());

	for (int i = 0; i < STUDIO_MAX_BLENDERS; ++i)
	{
		_modelEntity->SetBlending(i, snapshot->Value(QString{"entity.blender%1"}.arg(i)).toFloat());
	}

	model->TopColor = snapshot->Value("entity.model.topcolor", 0).toInt();
	model->BottomColor = snapshot->Value("entity.model.bottomcolor", 0).toInt();
}

bool StudioModelAsset::HandleMouseInput(QMouseEvent* event)
{
	if (GetCurrentScene() != GetScene())
	{
		return false;
	}

	if (event->modifiers() & Qt::KeyboardModifier::ControlModifier
		&& event->buttons() & Qt::MouseButton::LeftButton)
	{
		if (event->button() == Qt::MouseButton::LeftButton)
		{
			_lightVectorCoordinates.x = event->x();
			_lightVectorCoordinates.y = event->y();
		}
		else
		{
			glm::vec3 direction = GetScene()->SkyLight.Direction;

			const float DELTA = 0.05f;

			if (event->modifiers() & Qt::KeyboardModifier::ShiftModifier)
			{
				if (_lightVectorCoordinates.x <= event->x())
				{
					direction.z += DELTA;
				}
				else
				{
					direction.z -= DELTA;
				}

				_lightVectorCoordinates.x = event->x();

				direction.z = std::clamp(direction.z, -1.0f, 1.0f);
			}
			else
			{
				if (_lightVectorCoordinates.x <= event->x())
				{
					direction.x += DELTA;
				}
				else
				{
					direction.x -= DELTA;
				}

				if (_lightVectorCoordinates.y <= event->y())
				{
					direction.y += DELTA;
				}
				else
				{
					direction.y -= DELTA;
				}

				_lightVectorCoordinates.x = event->x();
				_lightVectorCoordinates.y = event->y();

				direction.x = std::clamp(direction.x, -1.0f, 1.0f);
				direction.y = std::clamp(direction.y, -1.0f, 1.0f);
			}

			GetScene()->SkyLight.Direction = direction;

			emit GetModelData()->SkyLightChanged();
		}

		return true;
	}

	return false;
}

void StudioModelAsset::Tick()
{
	//TODO: update asset-local world time
	for (auto scene : _scenes)
	{
		scene->Tick();
	}
}

void StudioModelAsset::OnSceneWidgetRecreated()
{
	const bool isFullscreen = _application->IsInFullscreenMode();
	const auto editWidget = _provider->GetEditWidget();
	const auto sceneWidget = _application->GetSceneWidget();

	editWidget->AttachSceneWidget();

	const bool editControlsVisible = _provider->AreEditControlsVisible();

	editWidget->SetControlsBarVisible(!isFullscreen && _provider->IsControlsBarVisible() && editControlsVisible);
	editWidget->SetTimelineVisible(!isFullscreen && _provider->IsTimelineVisible() && editControlsVisible);
	editWidget->SetDockWidgetsVisible(!isFullscreen && editControlsVisible);

	sceneWidget->SetScene(GetCurrentScene());

	connect(sceneWidget, &SceneWidget::MouseEvent,
		this, &StudioModelAsset::OnSceneWidgetMouseEvent, Qt::UniqueConnection);
	connect(sceneWidget, &SceneWidget::WheelEvent,
		this, &StudioModelAsset::OnSceneWidgetWheelEvent, Qt::UniqueConnection);

	emit EditWidgetChanged();
}

void StudioModelAsset::OnResizeTexturesToPowerOf2Changed()
{
	auto context = _application->GetGraphicsContext();

	context->Begin();
	_editableStudioModel->UpdateTextures(*GetTextureLoader());
	context->End();
}

void StudioModelAsset::OnTextureFiltersChanged()
{
	auto context = _application->GetGraphicsContext();

	context->Begin();
	_editableStudioModel->UpdateFilters(*GetTextureLoader());
	context->End();
}

void StudioModelAsset::OnSceneIndexChanged(int index)
{
	SetCurrentScene(index != -1 ? GetScenes()[index] : nullptr);
}

void StudioModelAsset::OnSceneWidgetMouseEvent(QMouseEvent* event)
{
	// Check if we handle this locally first.
	if (HandleMouseInput(event))
	{
		return;
	}

	if (_currentScene)
	{
		if (auto cameraOperator = _currentScene->GetCurrentCamera(); cameraOperator)
		{
			cameraOperator->MouseEvent(*event);
		}
	}
}

void StudioModelAsset::OnSceneWidgetWheelEvent(QWheelEvent* event)
{
	if (_currentScene)
	{
		if (auto cameraOperator = _currentScene->GetCurrentCamera(); cameraOperator)
		{
			cameraOperator->WheelEvent(*event);
		}
	}
}

void StudioModelAsset::OnCameraChanged(SceneCameraOperator* previous, SceneCameraOperator* current)
{
	_scene->SetCurrentCamera(current);
}

void StudioModelAsset::OnCenterView(Axis axis, bool positive)
{
	if (auto cameraOperator = _provider->GetCameraOperators()->GetCurrent(); cameraOperator)
	{
		const auto [targetOrigin, cameraOrigin, pitch, yaw] = GetCenteredValues(*_modelEntity, axis, positive);

		cameraOperator->CenterView(targetOrigin, cameraOrigin, pitch, yaw);
	}
}

void StudioModelAsset::OnFlipNormals()
{
	AddUndoCommand(new FlipNormalsCommand(this));
}
}
