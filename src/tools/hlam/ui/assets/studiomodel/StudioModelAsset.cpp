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

#include <GL/glew.h>

#include "assets/AssetIO.hpp"

#include "engine/shared/studiomodel/DumpModelInfo.hpp"
#include "engine/shared/studiomodel/StudioModelIO.hpp"
#include "engine/shared/studiomodel/StudioModelUtils.hpp"
#include "entity/HLMVStudioModelEntity.hpp"
#include "game/entity/BaseEntity.hpp"
#include "game/entity/EntityList.hpp"

#include "graphics/Scene.hpp"
#include "graphics/TextureLoader.hpp"

#include "qt/QtUtilities.hpp"

#include "ui/EditorContext.hpp"
#include "ui/FullscreenWidget.hpp"
#include "ui/SceneWidget.hpp"
#include "ui/StateSnapshot.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelColors.hpp"
#include "ui/assets/studiomodel/StudioModelEditWidget.hpp"
#include "ui/assets/studiomodel/StudioModelUndoCommands.hpp"
#include "ui/assets/studiomodel/compiler/StudioModelCompilerFrontEnd.hpp"
#include "ui/assets/studiomodel/compiler/StudioModelDecompilerFrontEnd.hpp"

#include "ui/camera_operators/ArcBallCameraOperator.hpp"
#include "ui/camera_operators/CameraOperator.hpp"
#include "ui/camera_operators/CameraOperators.hpp"
#include "ui/camera_operators/FirstPersonCameraOperator.hpp"
#include "ui/camera_operators/FreeLookCameraOperator.hpp"

#include "ui/settings/ColorSettings.hpp"
#include "ui/settings/StudioModelSettings.hpp"

#include "utility/IOUtils.hpp"
#include "utility/mathlib.hpp"

namespace ui::assets::studiomodel
{
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
	, _textureLoader(std::make_unique<graphics::TextureLoader>())
	, _scene(std::make_unique<graphics::Scene>(_textureLoader.get(), editorContext->GetSoundSystem(), editorContext->GetWorldTime()))
	, _cameraOperators(new camera_operators::CameraOperators(this))
{
	PushInputSink(this);

	connect(_cameraOperators, &camera_operators::CameraOperators::CameraChanged, this, &StudioModelAsset::OnCameraChanged);

	UpdateColors();

	_scene->FloorLength = _provider->GetStudioModelSettings()->GetFloorLength();

	auto entity = _scene->GetEntityContext()->EntityList->Create<HLMVStudioModelEntity>()([this](auto entity)
		{
			entity->SetEntityContext(_scene->GetEntityContext());
			entity->SetEditableModel(GetEditableStudioModel());
		}).SpawnAndGetEntity();

	if (nullptr != entity)
	{
		_scene->SetEntity(entity);
	}

	auto arcBallCamera = new camera_operators::ArcBallCameraOperator(_editorContext->GetGeneralSettings());
	_firstPersonCamera = new camera_operators::FirstPersonCameraOperator(_editorContext->GetGeneralSettings());

	_cameraOperators->Add(arcBallCamera);
	_cameraOperators->Add(new camera_operators::FreeLookCameraOperator(_editorContext->GetGeneralSettings()));
	_cameraOperators->Add(_firstPersonCamera);

	if (nullptr != entity)
	{
		const auto [targetOrigin, cameraOrigin, pitch, yaw] = GetCenteredValues(*entity, Axis::X, true);

		for (int i = 0; i < _cameraOperators->Count(); ++i)
		{
			const auto cameraOperator = _cameraOperators->Get(i);
			cameraOperator->CenterView(targetOrigin, cameraOrigin, pitch, yaw);
			//Set initial restoration point to the initial camera view
			cameraOperator->SaveView();
		}
	}

	if (_provider->GetStudioModelSettings()->ShouldAutodetectViewmodels() && QFileInfo{GetFileName()}.fileName().startsWith("v_"))
	{
		_cameraOperators->SetCurrent(_firstPersonCamera);
	}
	else
	{
		_cameraOperators->SetCurrent(arcBallCamera);
	}

	connect(_editorContext, &EditorContext::Tick, this, &StudioModelAsset::OnTick);
	connect(_editorContext->GetColorSettings(), &settings::ColorSettings::ColorsChanged, this, &StudioModelAsset::UpdateColors);
	connect(_provider->GetStudioModelSettings(), &settings::StudioModelSettings::FloorLengthChanged, this, &StudioModelAsset::OnFloorLengthChanged);
}

StudioModelAsset::~StudioModelAsset()
{
	PopInputSink();

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

	_editWidget->connect(_editWidget->GetSceneWidget(), &SceneWidget::MouseEvent, this, &StudioModelAsset::OnSceneWidgetMouseEvent);
	_editWidget->connect(_editWidget->GetSceneWidget(), &SceneWidget::WheelEvent, this, &StudioModelAsset::OnSceneWidgetWheelEvent);

	return _editWidget;
}

void StudioModelAsset::SetupFullscreenWidget(FullscreenWidget* fullscreenWidget)
{
	const auto sceneWidget = new SceneWidget(GetScene(), fullscreenWidget);

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

		_editableStudioModel = std::make_unique<studiomdl::EditableStudioModel>(studiomdl::ConvertToEditable(*studioModel));

		GetUndoStack()->clear();

		auto entity = _scene->GetEntity();
		entity->SetEditableModel(GetEditableStudioModel());
		entity->Spawn();
	}
	catch (const ::assets::AssetException& e)
	{
		QMessageBox::critical(nullptr, "Error", QString{"An error occurred while reloading the model \"%1\":\n%2"}.arg(GetFileName()).arg(e.what()));
		return;
	}

	LoadEntityFromSnapshot(snapshot.get());

	emit LoadSnapshot(snapshot.get());
}

void StudioModelAsset::SaveEntityToSnapshot(StateSnapshot* snapshot)
{
	auto entity = _scene->GetEntity();
	auto model = entity->GetEditableModel();

	const int sequenceIndex = entity->GetSequence();

	if (sequenceIndex >= 0 && sequenceIndex < model->Sequences.size())
	{
		snapshot->SetValue("entity.sequence", QVariant::fromValue(QString::fromStdString(model->Sequences[sequenceIndex]->Label)));
		snapshot->SetValue("entity.frame", QVariant::fromValue(entity->GetFrame()));
	}

	snapshot->SetValue("entity.skin", QVariant::fromValue(entity->GetSkin()));
	snapshot->SetValue("entity.body", QVariant::fromValue(entity->GetBodygroup()));

	for (int i = 0; i < STUDIO_MAX_CONTROLLERS; ++i)
	{
		snapshot->SetValue(QString{"entity.bonecontroller%1"}.arg(i), QVariant::fromValue(entity->GetControllerValue(i)));
	}

	snapshot->SetValue("entity.mouth", QVariant::fromValue(entity->GetMouth()));

	for (int i = 0; i < STUDIO_MAX_BLENDERS; ++i)
	{
		snapshot->SetValue(QString{"entity.blender%1"}.arg(i), QVariant::fromValue(entity->GetBlendingValue(i)));
	}
}

void StudioModelAsset::LoadEntityFromSnapshot(StateSnapshot* snapshot)
{
	auto entity = _scene->GetEntity();
	auto model = entity->GetEditableModel();

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

			entity->SetSequence(static_cast<int>(index));

			//Only set these if we were able to find the sequence
			entity->SetFrame(snapshot->Value("entity.frame").toFloat());

			foundSequence = true;
		}
	}

	if (!foundSequence)
	{
		//Reset to default
		entity->SetSequence(0);
	}

	entity->SetSkin(snapshot->Value("entity.skin").toInt());
	entity->SetCompoundBodyValue(snapshot->Value("entity.body").toInt());

	for (int i = 0; i < STUDIO_MAX_CONTROLLERS; ++i)
	{
		entity->SetController(i, snapshot->Value(QString{"entity.bonecontroller%1"}.arg(i)).toFloat());
	}

	entity->SetMouth(snapshot->Value("entity.mouth").toFloat());

	for (int i = 0; i < STUDIO_MAX_BLENDERS; ++i)
	{
		entity->SetBlending(i, snapshot->Value(QString{"entity.blender%1"}.arg(i)).toFloat());
	}
}

void StudioModelAsset::OnTick()
{
	//TODO: update asset-local world time
	_scene->Tick();

	emit Tick();
}

void StudioModelAsset::OnMouseEvent(QMouseEvent* event)
{
	if (auto cameraOperator = _cameraOperators->GetCurrent(); cameraOperator)
	{
		cameraOperator->MouseEvent(*event);
	}
}

void StudioModelAsset::OnWheelEvent(QWheelEvent* event)
{
	if (auto cameraOperator = _cameraOperators->GetCurrent(); cameraOperator)
	{
		cameraOperator->WheelEvent(*event);
	}
}

void StudioModelAsset::OnSceneWidgetMouseEvent(QMouseEvent* event)
{
	if (!_inputSinks.empty())
	{
		_inputSinks.top()->OnMouseEvent(event);
	}
}

void StudioModelAsset::OnSceneWidgetWheelEvent(QWheelEvent* event)
{
	if (!_inputSinks.empty())
	{
		_inputSinks.top()->OnWheelEvent(event);
	}
}

void StudioModelAsset::OnCameraChanged(camera_operators::CameraOperator* previous, camera_operators::CameraOperator* current)
{
	_scene->SetCurrentCamera(current != nullptr ? current->GetCamera() : nullptr);
	_scene->CameraIsFirstPerson = current == _firstPersonCamera;
}

static glm::vec3 ColorToVector(const QColor& color)
{
	return {color.redF(), color.greenF(), color.blueF()};
}

void StudioModelAsset::UpdateColors()
{
	const auto colorSettings = _editorContext->GetColorSettings();

	_scene->GroundColor = ColorToVector(colorSettings->GetColor(GroundColor.Name));
	_scene->BackgroundColor = ColorToVector(colorSettings->GetColor(BackgroundColor.Name));
	_scene->CrosshairColor = ColorToVector(colorSettings->GetColor(CrosshairColor.Name));
	_scene->SetLightColor(ColorToVector(colorSettings->GetColor(LightColor.Name)));
	_scene->SetWireframeColor(ColorToVector(colorSettings->GetColor(WireframeColor.Name)));
}

void StudioModelAsset::OnFloorLengthChanged(int length)
{
	_scene->FloorLength = length;
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
		const auto [targetOrigin, cameraOrigin, pitch, yaw] = GetCenteredValues(*_scene->GetEntity(), axis, positive);

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

	auto model = GetScene()->GetEntity()->GetEditableModel();

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

	return std::make_unique<StudioModelAsset>(std::move(updatedFileName), editorContext, this,
		std::make_unique<studiomdl::EditableStudioModel>(std::move(editableStudioModel)));
}
}
