#include <algorithm>
#include <cstdio>
#include <stdexcept>

#include <QAction>
#include <QColor>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QImage>
#include <QMenu>
#include <QMessageBox>

#include <GL/glew.h>

#include "engine/shared/studiomodel/DumpModelInfo.hpp"
#include "engine/shared/studiomodel/StudioModelIO.hpp"
#include "engine/shared/studiomodel/StudioModelUtils.hpp"
#include "entity/HLMVStudioModelEntity.hpp"
#include "game/entity/BaseEntity.hpp"
#include "game/entity/BaseEntityList.hpp"
#include "game/entity/EntityManager.hpp"

#include "graphics/Scene.hpp"
#include "graphics/TextureLoader.hpp"

#include "qt/QtUtilities.hpp"

#include "ui/EditorContext.hpp"
#include "ui/FullscreenWidget.hpp"
#include "ui/SceneWidget.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelColors.hpp"
#include "ui/assets/studiomodel/StudioModelEditWidget.hpp"
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

namespace ui::assets::studiomodel
{
const QString StudioModelExtension{QStringLiteral("mdl")};
const QString StudioModelPS2Extension{QStringLiteral("dol")};

const float InitialCameraYaw{180};

static std::pair<float, float> GetCenteredValues(HLMVStudioModelEntity* entity)
{
	glm::vec3 min, max;
	entity->ExtractBbox(min, max);

	//Clamp the values to a reasonable range
	for (int i = 0; i < 3; ++i)
	{
		//Use different limits for min and max so centering won't end up setting origin to 0 0 0
		min[i] = std::clamp(min[i], -2000.f, 2000.f);
		max[i] = std::clamp(max[i], -1000.f, 1000.f);
	}

	const float dx = max[0] - min[0];
	const float dy = max[1] - min[1];
	const float dz = max[2] - min[2];

	const float distance{std::max({dx, dy, dz})};

	const float height{min[2] + (dz / 2)};

	return {height, distance};
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

	auto entity = static_cast<HLMVStudioModelEntity*>(_scene->GetEntityContext()->EntityManager->Create("studiomodel", _scene->GetEntityContext(),
		glm::vec3(), glm::vec3(), false));

	if (nullptr != entity)
	{
		entity->SetEditableModel(GetEditableStudioModel());
		entity->Spawn();
		_scene->SetEntity(entity);
	}

	auto arcBallCamera = new camera_operators::ArcBallCameraOperator(_editorContext->GetGeneralSettings());
	_firstPersonCamera = new camera_operators::FirstPersonCameraOperator(_editorContext->GetGeneralSettings());

	_cameraOperators->Add(arcBallCamera);
	_cameraOperators->Add(new camera_operators::FreeLookCameraOperator(_editorContext->GetGeneralSettings()));
	_cameraOperators->Add(_firstPersonCamera);

	if (nullptr != entity)
	{
		const auto [height, distance] = GetCenteredValues(entity);

		for (int i = 0; i < _cameraOperators->Count(); ++i)
		{
			const auto cameraOperator = _cameraOperators->Get(i);
			cameraOperator->CenterView(height, distance, InitialCameraYaw);
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
	menu->addAction("Previous Camera", this, &StudioModelAsset::OnPreviousCamera, QKeySequence{Qt::CTRL + Qt::Key::Key_U});
	menu->addAction("Next Camera", this, &StudioModelAsset::OnNextCamera, QKeySequence{Qt::CTRL + Qt::Key::Key_I});

	menu->addSeparator();

	menu->addAction("Center View", this, &StudioModelAsset::OnCenterView);
	menu->addAction("Save View", this, &StudioModelAsset::OnSaveView);
	menu->addAction("Restore View", this, &StudioModelAsset::OnRestoreView);

	menu->addSeparator();

	menu->addAction("Load Ground Texture...", this, &StudioModelAsset::OnLoadGroundTexture);
	menu->addAction("Unload Ground Texture", this, &StudioModelAsset::OnUnloadGroundTexture);

	menu->addAction("Load Background Texture...", this, &StudioModelAsset::OnLoadBackgroundTexture);
	menu->addAction("Unload Background Texture", this, &StudioModelAsset::OnUnloadBackgroundTexture);

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
	//TODO: add setting to correct groups
	const auto filePath = std::filesystem::u8path(GetFileName().toStdString());
	auto result = studiomdl::ConvertFromEditable(filePath, *_editableStudioModel);

	studiomdl::SaveStudioModel(filePath, result, false);
}

void StudioModelAsset::OnTick()
{
	//TODO: update asset-local world time
	//TODO: pause all updates while not active
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

void StudioModelAsset::OnCenterView()
{
	if (auto cameraOperator = _cameraOperators->GetCurrent(); cameraOperator)
	{
		const auto [height, distance] = GetCenteredValues(_scene->GetEntity());

		cameraOperator->CenterView(height, distance, InitialCameraYaw);
	}
}

void StudioModelAsset::OnSaveView()
{
	if (auto cameraOperator = _cameraOperators->GetCurrent(); cameraOperator)
	{
		const auto [height, distance] = GetCenteredValues(_scene->GetEntity());

		cameraOperator->SaveView();
	}
}

void StudioModelAsset::OnRestoreView()
{
	if (auto cameraOperator = _cameraOperators->GetCurrent(); cameraOperator)
	{
		const auto [height, distance] = GetCenteredValues(_scene->GetEntity());

		cameraOperator->RestoreView();
	}
}

void StudioModelAsset::OnLoadGroundTexture()
{
	const QString fileName{QFileDialog::getOpenFileName(nullptr, {}, {}, qt::GetImagesFileFilter())};

	if (!fileName.isEmpty())
	{
		QImage image{fileName};

		if (image.isNull())
		{
			QMessageBox::critical(nullptr, "Error", QString{"An error occurred while loading the image \"%1\""}.arg(fileName));
			return;
		}

		image.convertTo(QImage::Format::Format_RGBA8888);

		_scene->GetGraphicsContext()->Begin();

		if (_scene->GroundTexture == 0)
		{
			glGenTextures(1, &_scene->GroundTexture);
		}

		glBindTexture(GL_TEXTURE_2D, _scene->GroundTexture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.constBits());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		_scene->GetGraphicsContext()->End();
	}
}

void StudioModelAsset::OnUnloadGroundTexture()
{
	if (_scene->GroundTexture != 0)
	{
		_scene->GetGraphicsContext()->Begin();
		glDeleteTextures(1, &_scene->GroundTexture);
		_scene->GroundTexture = 0;
		_scene->GetGraphicsContext()->End();
	}
}

void StudioModelAsset::OnLoadBackgroundTexture()
{
	const QString fileName{QFileDialog::getOpenFileName(nullptr, {}, {}, qt::GetImagesFileFilter())};

	if (!fileName.isEmpty())
	{
		QImage image{fileName};

		if (image.isNull())
		{
			QMessageBox::critical(nullptr, "Error", QString{"An error occurred while loading the image \"%1\""}.arg(fileName));
			return;
		}

		image.convertTo(QImage::Format::Format_RGBA8888);

		_scene->GetGraphicsContext()->Begin();

		if (_scene->BackgroundTexture == 0)
		{
			glGenTextures(1, &_scene->BackgroundTexture);
		}

		glBindTexture(GL_TEXTURE_2D, _scene->BackgroundTexture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.constBits());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		_scene->GetGraphicsContext()->End();
	}
}

void StudioModelAsset::OnUnloadBackgroundTexture()
{
	if (_scene->BackgroundTexture != 0)
	{
		_scene->GetGraphicsContext()->Begin();
		glDeleteTextures(1, &_scene->BackgroundTexture);
		_scene->BackgroundTexture = 0;
		_scene->GetGraphicsContext()->End();
	}
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

QString StudioModelAssetProvider::GetProviderName() const
{
	return "Studiomodel";
}

QStringList StudioModelAssetProvider::GetFileTypes() const
{
	return {StudioModelExtension, StudioModelPS2Extension};
}

QString StudioModelAssetProvider::GetPreferredFileType() const
{
	return StudioModelExtension;
}

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

bool StudioModelAssetProvider::CanLoad(const QString& fileName) const
{
	//TODO: race condition: the file could be modified between this call and a call to LoadStudioModel
	//Ideally the file is opened exactly once
	return studiomdl::IsStudioModel(fileName.toStdString());
}

std::unique_ptr<Asset> StudioModelAssetProvider::Load(EditorContext* editorContext, const QString& fileName) const
{
	const auto filePath = std::filesystem::u8path(fileName.toStdString());
	auto studioModel = studiomdl::LoadStudioModel(filePath);

	auto editableStudioModel = studiomdl::ConvertToEditable(*studioModel);

	return std::make_unique<StudioModelAsset>(QString{fileName}, editorContext, this,
		std::make_unique<studiomdl::EditableStudioModel>(std::move(editableStudioModel)));
}
}
