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

#include "formats/studiomodel/DumpModelInfo.hpp"
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

#include "settings/ColorSettings.hpp"
#include "settings/StudioModelSettings.hpp"

#include "soundsystem/SoundSystem.hpp"

#include "ui/EditorContext.hpp"
#include "ui/FullscreenWidget.hpp"
#include "ui/SceneWidget.hpp"
#include "ui/StateSnapshot.hpp"

#include "ui/camera_operators/ArcBallCameraOperator.hpp"
#include "ui/camera_operators/CameraOperator.hpp"
#include "ui/camera_operators/CameraOperators.hpp"
#include "ui/camera_operators/FirstPersonCameraOperator.hpp"
#include "ui/camera_operators/FreeLookCameraOperator.hpp"
#include "ui/camera_operators/TextureCameraOperator.hpp"

#include "utility/IOUtils.hpp"

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
	, _soundSystem(std::make_unique<SoundSystemWrapper>(
		_editorContext->GetSoundSystem(), _editorContext->GetFileSystem()))
	, _entityContext(std::make_unique<EntityContext>(this,
		_editorContext->GetWorldTime(),
		_provider->GetStudioModelRenderer(),
		_provider->GetSpriteRenderer(),
		_soundSystem.get(),
		_editorContext->GetApplicationSettings(),
		_provider->GetStudioModelSettings()))
	, _cameraOperators(new CameraOperators(this))
{
	{
		_editWidget = new QWidget();
		auto layout = new SingleWidgetLayout(_editWidget);
		layout->setContentsMargins(0, 0, 0, 0);
		layout->setSpacing(0);
		_editWidget->setLayout(layout);
	}

	connect(_cameraOperators, &CameraOperators::CameraChanged, this, &StudioModelAsset::OnCameraChanged);

	CreateMainScene();
	CreateTextureScene();

	auto arcBallCamera = new ArcBallCameraOperator(_editorContext->GetApplicationSettings());
	_firstPersonCamera = new FirstPersonCameraOperator(_editorContext->GetApplicationSettings());

	_cameraOperators->Add(arcBallCamera);
	_cameraOperators->Add(new FreeLookCameraOperator(_editorContext->GetApplicationSettings()));
	_cameraOperators->Add(_firstPersonCamera);

	const auto [targetOrigin, cameraOrigin, pitch, yaw] = GetCenteredValues(*_modelEntity, Axis::X, true);

	for (int i = 0; i < _cameraOperators->Count(); ++i)
	{
		const auto cameraOperator = _cameraOperators->Get(i);
		cameraOperator->CenterView(targetOrigin, cameraOrigin, pitch, yaw);
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

	connect(this, &StudioModelAsset::IsActiveChanged, this, &StudioModelAsset::OnIsActiveChanged);
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

QWidget* StudioModelAsset::GetEditWidget()
{
	return _editWidget;
}

void StudioModelAsset::EnterFullscreen(FullscreenWidget* fullscreenWidget)
{
	_provider->GetEditWidget()->DetachSceneWidget();
	fullscreenWidget->SetWidget(_editorContext->GetSceneWidget()->GetContainer());
}

void StudioModelAsset::ExitFullscreen(FullscreenWidget* fullscreenWidget)
{
	OnSceneWidgetRecreated();
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

		_modelEntity->CreateDeviceObjects(sc);

		context->End();
	}
	catch (const AssetException& e)
	{
		QMessageBox::critical(nullptr, "Error", QString{"An error occurred while reloading the model \"%1\":\n%2"}.arg(GetFileName()).arg(e.what()));
		return false;
	}

	LoadEntityFromSnapshot(snapshot.get());

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

	if (IsActive())
	{
		auto editWidget = _provider->GetEditWidget();
		editWidget->SetSceneIndex(index);
		_editorContext->GetSceneWidget()->SetScene(_currentScene);
	}
}

bool StudioModelAsset::CameraIsFirstPerson() const
{
	return _cameraOperators->GetCurrent() == _firstPersonCamera;
}

void StudioModelAsset::OnActivated()
{
	auto editWidget = _provider->GetEditWidget();

	{
		const QSignalBlocker editBlocker{editWidget};
		editWidget->SetAsset(this);
	}

	connect(editWidget, &StudioModelEditWidget::SceneIndexChanged, this, &StudioModelAsset::OnSceneIndexChanged);
	connect(editWidget, &StudioModelEditWidget::PoseChanged, this, &StudioModelAsset::SetPose);

	OnSceneWidgetRecreated();

	emit LoadSnapshot(&_snapshot);
}

void StudioModelAsset::OnDeactivated()
{
	emit SaveSnapshot(&_snapshot);

	auto editWidget = _provider->GetEditWidget();
	auto sceneWidget = _editorContext->GetSceneWidget();

	editWidget->disconnect(this);
	sceneWidget->disconnect(this);
	editWidget->SetAsset(_provider->GetDummyAsset());

	auto item = _editWidget->layout()->takeAt(0);
	_editWidget->layout()->removeItem(item);
	delete item;
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

void StudioModelAsset::OnTick()
{
	//TODO: update asset-local world time
	for (auto scene : _scenes)
	{
		scene->Tick();
	}

	emit Tick();
}

void StudioModelAsset::OnIsActiveChanged(bool value)
{
	// Only update if we're active.
	if (value)
	{
		connect(_provider, &StudioModelAssetProvider::Tick, this, &StudioModelAsset::OnTick, Qt::UniqueConnection);
		connect(_editorContext, &EditorContext::SceneWidgetRecreated, this, &StudioModelAsset::OnSceneWidgetRecreated,
			Qt::UniqueConnection);
	}
	else
	{
		disconnect(_provider, &StudioModelAssetProvider::Tick, this, &StudioModelAsset::OnTick);
		disconnect(_editorContext, &EditorContext::SceneWidgetRecreated,
			this, &StudioModelAsset::OnSceneWidgetRecreated);
	}
}

void StudioModelAsset::OnSceneWidgetRecreated()
{
	if (auto fullscreenWidget = _editorContext->GetFullscreenWidget(); fullscreenWidget)
	{
		fullscreenWidget->SetWidget(_editorContext->GetSceneWidget()->GetContainer());
	}
	else if (_provider->AreEditControlsVisible())
	{
		auto editWidget = _provider->GetEditWidget();

		editWidget->AttachSceneWidget();
		_editWidget->layout()->addWidget(editWidget);
		editWidget->setParent(_editWidget);
	}
	else
	{
		_editWidget->layout()->addWidget(_editorContext->GetSceneWidget()->GetContainer());
	}

	auto sceneWidget = _editorContext->GetSceneWidget();
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

	const QImage screenshot = _editorContext->GetSceneWidget()->grabFramebuffer();

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
