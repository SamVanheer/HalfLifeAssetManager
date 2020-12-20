#include <algorithm>
#include <cstdio>
#include <stdexcept>

#include <QAction>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QImage>
#include <QMenu>
#include <QMessageBox>

#include <GL/glew.h>

#include "engine/shared/studiomodel/DumpModelInfo.hpp"
#include "entity/CHLMVStudioModelEntity.hpp"
#include "game/entity/CBaseEntity.hpp"
#include "game/entity/CBaseEntityList.hpp"
#include "game/entity/CEntityManager.hpp"

#include "graphics/Scene.hpp"
#include "graphics/TextureLoader.hpp"

#include "qt/QtUtilities.hpp"

#include "ui/EditorContext.hpp"
#include "ui/FullscreenWidget.hpp"
#include "ui/SceneWidget.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelEditWidget.hpp"

#include "ui/camera_operators/ArcBallCameraOperator.hpp"
#include "ui/camera_operators/CameraOperator.hpp"
#include "ui/camera_operators/FirstPersonCameraOperator.hpp"
#include "ui/camera_operators/FreeLookCameraOperator.hpp"

#include "ui/settings/StudioModelSettings.hpp"

#include "utility/IOUtils.hpp"

namespace ui::assets::studiomodel
{
StudioModelAsset::StudioModelAsset(QString&& fileName,
	EditorContext* editorContext, const StudioModelAssetProvider* provider, std::unique_ptr<studiomdl::CStudioModel>&& studioModel)
	: Asset(std::move(fileName))
	, _editorContext(editorContext)
	, _provider(provider)
	, _studioModel(std::move(studioModel))
	, _textureLoader(std::make_unique<graphics::TextureLoader>())
	, _scene(std::make_unique<graphics::Scene>(_textureLoader.get(), editorContext->GetSoundSystem(), editorContext->GetWorldTime()))
{
	PushInputSink(this);

	//TODO: need to be able to update arcball camera without having to do this directly
	auto arcBallCameraOperator = std::make_unique<camera_operators::ArcBallCameraOperator>(_editorContext->GetGeneralSettings());
	auto freeLookCameraOperator = std::make_unique<camera_operators::FreeLookCameraOperator>(_editorContext->GetGeneralSettings());

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

		glm::vec3 min, max;
		entity->ExtractBbox(min, max);

		//Clamp the values to a reasonable range
		for (int i = 0; i < 3; ++i)
		{
			//Use different limits for min and max so centering won't end up setting origin to 0 0 0
			min[i] = clamp(min[i], -2000.f, 2000.f);
			max[i] = clamp(max[i], -1000.f, 1000.f);
		}

		float dx = max[0] - min[0];
		float dy = max[1] - min[1];
		float dz = max[2] - min[2];

		float d = dx;

		if (dy > d)
			d = dy;
		if (dz > d)
			d = dz;

		glm::vec3 trans;

		trans[2] = 0;
		trans[0] = -(min[2] + dz / 2);
		trans[1] = d * 1.0f;

		arcBallCameraOperator->GetCamera()->SetOrigin(trans);
		freeLookCameraOperator->GetCamera()->SetOrigin(trans);
	}

	AddCameraOperator(std::move(arcBallCameraOperator));
	AddCameraOperator(std::move(freeLookCameraOperator));
	AddCameraOperator(std::make_unique<camera_operators::FirstPersonCameraOperator>());

	//TODO: need to use the actual camera objects instead of indices
	if (_provider->GetSettings()->ShouldAutodetectViewmodels() && QFileInfo{GetFileName()}.fileName().startsWith("v_"))
	{
		SetCurrentCameraOperator(GetCameraOperator(2));
	}
	else
	{
		SetCurrentCameraOperator(GetCameraOperator(0));
	}

	connect(_editorContext, &EditorContext::Tick, this, &StudioModelAsset::OnTick);
	connect(_provider->GetSettings(), &settings::StudioModelSettings::FloorLengthChanged, this, &StudioModelAsset::OnFloorLengthChanged);
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

	menu->addAction("Load Ground Texture...", this, &StudioModelAsset::OnLoadGroundTexture);
	menu->addAction("Unload Ground Texture", this, &StudioModelAsset::OnUnloadGroundTexture);

	menu->addAction("Load Background Texture...", this, &StudioModelAsset::OnLoadBackgroundTexture);
	menu->addAction("Unload Background Texture", this, &StudioModelAsset::OnUnloadBackgroundTexture);

	menu->addSeparator();

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

void StudioModelAsset::SetupFullscreenWidget(FullscreenWidget* fullscreenWidget)
{
	const auto sceneWidget = new SceneWidget(GetScene(), fullscreenWidget);

	fullscreenWidget->setCentralWidget(sceneWidget->GetContainer());

	sceneWidget->connect(this, &StudioModelAsset::Draw, sceneWidget, &SceneWidget::requestUpdate);
	sceneWidget->connect(sceneWidget, &SceneWidget::MouseEvent, this, &StudioModelAsset::OnSceneWidgetMouseEvent);

	//Filter key events on the scene widget so we can capture exit even if it has focus
	sceneWidget->installEventFilter(fullscreenWidget);
}

void StudioModelAsset::Save(const QString& fileName)
{
	_provider->Save(fileName, *this);
}

camera_operators::CameraOperator* StudioModelAsset::GetCameraOperator(int index) const
{
	return _cameraOperators[index].get();
}

void StudioModelAsset::AddCameraOperator(std::unique_ptr<camera_operators::CameraOperator>&& cameraOperator)
{
	assert(cameraOperator);

	_cameraOperators.emplace_back(std::move(cameraOperator));
}

void StudioModelAsset::SetCurrentCameraOperator(camera_operators::CameraOperator* cameraOperator)
{
	if (_cameraOperator != cameraOperator)
	{
		_cameraOperator = cameraOperator;
		_scene->SetCurrentCamera(_cameraOperator != nullptr ? _cameraOperator->GetCamera() : nullptr);

		emit CameraChanged(_cameraOperator);
	}
}

void StudioModelAsset::ChangeCamera(bool next)
{
	int index;

	if (_cameraOperators.empty())
	{
		index = -1;
	}
	else if (!_cameraOperator)
	{
		index = next ? 0 : _cameraOperators.size() - 1;
	}
	else
	{
		auto it = std::find_if(_cameraOperators.begin(), _cameraOperators.end(), [=](const auto& candidate)
			{
				return candidate.get() == _cameraOperator;
			});

		index = it - _cameraOperators.begin();

		if (next)
		{
			index += 1;
		}
		else
		{
			index -= 1;
		}

		if (index < 0)
		{
			index = _cameraOperators.size() - 1;
		}
		else if (index >= _cameraOperators.size())
		{
			index = 0;
		}
	}

	SetCurrentCameraOperator(index != -1 ? _cameraOperators[index].get() : nullptr);
}

void StudioModelAsset::OnTick()
{
	//TODO: update asset-local world time
	//TODO: pause all updates while not active
	_scene->Tick();

	emit Tick();

	if (IsActive())
	{
		emit Draw();
	}
}

void StudioModelAsset::OnMouseEvent(QMouseEvent* event)
{
	if (_cameraOperator)
	{
		_cameraOperator->MouseEvent(*event);
	}
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

void StudioModelAsset::OnPreviousCamera()
{
	ChangeCamera(false);
}

void StudioModelAsset::OnNextCamera()
{
	ChangeCamera(true);
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
