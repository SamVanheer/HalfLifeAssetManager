#include <algorithm>
#include <tuple>
#include <vector>

#include <QBoxLayout>
#include <QDir>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QImage>
#include <QMenu>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QWidget>

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

#include "settings/GameConfigurationsSettings.hpp"
#include "settings/StudioModelSettings.hpp"

#include "soundsystem/SoundSystem.hpp"

#include "application/EditorContext.hpp"
#include "ui/FullscreenWidget.hpp"
#include "ui/SceneWidget.hpp"

#include "ui/camera_operators/CameraOperators.hpp"
#include "ui/camera_operators/TextureCameraOperator.hpp"

namespace studiomodel
{
/**
*	@brief Layout that allows no more than one widget to be added.
*/
class SingleWidgetLayout final : public QVBoxLayout
{
public:
	using QVBoxLayout::QVBoxLayout;

	void addItem(QLayoutItem* item) override
	{
		delete takeAt(0);
		QVBoxLayout::addItem(item);
	}
};

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
	EditorContext* editorContext, StudioModelAssetProvider* provider,
	std::unique_ptr<studiomdl::EditableStudioModel>&& editableStudioModel)
	: Asset(std::move(fileName))
	, _editorContext(editorContext)
	, _provider(provider)
	, _editableStudioModel(std::move(editableStudioModel))
	, _modelData(new StudioModelData(_editableStudioModel.get(), this))
	, _fileSystem(_editorContext->GetApplicationSettings()->GetGameConfigurations()->CreateFileSystem(GetFileName()))
	, _soundSystem(std::make_unique<SoundSystemWrapper>(_editorContext->GetSoundSystem(), _fileSystem.get()))
	, _entityContext(std::make_unique<EntityContext>(this,
		_editorContext->GetWorldTime(),
		_provider->GetStudioModelRenderer(),
		_provider->GetSpriteRenderer(),
		_soundSystem.get(),
		_editorContext->GetApplicationSettings(),
		_provider->GetStudioModelSettings()))
{
	{
		_editWidget = new QWidget();
		auto layout = new SingleWidgetLayout(_editWidget);
		layout->setContentsMargins(0, 0, 0, 0);
		layout->setSpacing(0);
		_editWidget->setLayout(layout);
	}

	CreateMainScene();
	CreateTextureScene();

	connect(_editorContext->GetApplicationSettings(), &ApplicationSettings::ResizeTexturesToPowerOf2Changed,
		this, &StudioModelAsset::OnResizeTexturesToPowerOf2Changed);
	connect(_editorContext->GetApplicationSettings(), &ApplicationSettings::TextureFiltersChanged,
		this, &StudioModelAsset::OnTextureFiltersChanged);

	// Initialize graphics resources.
	{
		graphics::SceneContext sc{_editorContext->GetOpenGLFunctions(), _editorContext->GetTextureLoader()};

		auto context = _editorContext->GetGraphicsContext();
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
		graphics::SceneContext sc{_editorContext->GetOpenGLFunctions(), _editorContext->GetTextureLoader()};

		auto context = _editorContext->GetGraphicsContext();

		context->Begin();

		for (auto it = _scenes.rbegin(), end = _scenes.rend(); it != end; ++it)
		{
			(*it)->DestroyDeviceObjects(sc);
		}

		context->End();
	}

	delete _editWidget;
}

void StudioModelAsset::Save()
{
	const auto filePath = std::filesystem::u8path(GetFileName().toStdString());
	auto result = studiomdl::ConvertFromEditable(filePath, *_editableStudioModel);

	studiomdl::SaveStudioModel(filePath, result, false);
}

bool StudioModelAsset::TryRefresh()
{
	auto snapshot = std::make_unique<StateSnapshot>();

	SaveEntityToSnapshot(snapshot.get());

	emit SaveSnapshot(snapshot.get());

	auto oldModelData = _modelData;

	try
	{
		const auto filePath = std::filesystem::u8path(GetFileName().toStdString());
		auto studioModel = studiomdl::LoadStudioModel(filePath, nullptr);

		auto newModel = std::make_unique<studiomdl::EditableStudioModel>(studiomdl::ConvertToEditable(*studioModel));

		// Clear UI to null state so changes to the models don't trigger changes in UI slots.
		emit _provider->AssetChanged(_provider->GetDummyAsset());

		graphics::SceneContext sc{_editorContext->GetOpenGLFunctions(), _editorContext->GetTextureLoader()};

		auto context = _editorContext->GetGraphicsContext();
		context->Begin();

		//Clean up old model resources
		//TODO: needs to be handled better
		_modelEntity->DestroyDeviceObjects(sc);

		_editableStudioModel = std::move(newModel);

		_modelData = new StudioModelData(_editableStudioModel.get(), this);

		GetUndoStack()->clear();

		_modelEntity->SetEditableModel(GetEditableStudioModel());
		_modelEntity->Spawn();

		LoadEntityFromSnapshot(snapshot.get());

		_modelEntity->CreateDeviceObjects(sc);

		context->End();
	}
	catch (const AssetException& e)
	{
		QMessageBox::critical(nullptr, "Error",
			QString{"An error occurred while reloading the model \"%1\":\n%2"}.arg(GetFileName()).arg(e.what()));
		return false;
	}

	emit _provider->AssetChanged(this);

	// Delete the old data now that any remaining references have been cleared.
	delete oldModelData;

	emit LoadSnapshot(snapshot.get());

	return true;
}

graphics::IGraphicsContext* StudioModelAsset::GetGraphicsContext()
{
	return _editorContext->GetGraphicsContext();
}

graphics::TextureLoader* StudioModelAsset::GetTextureLoader()
{
	return _editorContext->GetTextureLoader();
}

studiomdl::IStudioModelRenderer* StudioModelAsset::GetStudioModelRenderer() const
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
		_editorContext->GetSceneWidget()->SetScene(_currentScene);
	}
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

	connect(_editorContext, &EditorContext::SceneWidgetRecreated, this, &StudioModelAsset::OnSceneWidgetRecreated);
	connect(_editorContext, &EditorContext::FullscreenWidgetChanged, this, &StudioModelAsset::OnSceneWidgetRecreated);
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

		if (studioModelSettings->ShouldAutodetectViewmodels() && QFileInfo{GetFileName()}.fileName().startsWith("v_"))
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
	auto sceneWidget = _editorContext->GetSceneWidget();

	editWidget->disconnect(this);
	sceneWidget->disconnect(this);

	disconnect(_editorContext, &EditorContext::SceneWidgetRecreated,
		this, &StudioModelAsset::OnSceneWidgetRecreated);
	disconnect(_editorContext, &EditorContext::FullscreenWidgetChanged,
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

	_modelEntity->SetMouth(snapshot->Value("entity.mouth").toFloat());

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
	const auto fullscreenWidget = _editorContext->GetFullscreenWidget();
	const auto editWidget = _provider->GetEditWidget();
	const auto sceneWidget = _editorContext->GetSceneWidget();

	editWidget->DetachSceneWidget();

	if (fullscreenWidget)
	{
		fullscreenWidget->SetWidget(sceneWidget->GetContainer());
	}

	if (_provider->AreEditControlsVisible())
	{
		if (!fullscreenWidget)
		{
			editWidget->AttachSceneWidget();
		}

		if (_editWidget->layout()->indexOf(editWidget) == -1)
		{
			_editWidget->layout()->addWidget(editWidget);
		}

		editWidget->setParent(_editWidget);
		editWidget->show();
	}
	else if (!fullscreenWidget)
	{
		_editWidget->layout()->addWidget(sceneWidget->GetContainer());
	}
	else
	{
		// If fullscreen is enabled the edit widget will be shown unless explicitly hidden.
		editWidget->hide();
	}

	sceneWidget->SetScene(GetCurrentScene());

	connect(sceneWidget, &SceneWidget::MouseEvent,
		this, &StudioModelAsset::OnSceneWidgetMouseEvent, Qt::UniqueConnection);
	connect(sceneWidget, &SceneWidget::WheelEvent,
		this, &StudioModelAsset::OnSceneWidgetWheelEvent, Qt::UniqueConnection);
}

void StudioModelAsset::OnResizeTexturesToPowerOf2Changed()
{
	auto context = _editorContext->GetGraphicsContext();

	context->Begin();
	_editableStudioModel->UpdateTextures(*GetTextureLoader());
	context->End();
}

void StudioModelAsset::OnTextureFiltersChanged()
{
	auto context = _editorContext->GetGraphicsContext();

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
