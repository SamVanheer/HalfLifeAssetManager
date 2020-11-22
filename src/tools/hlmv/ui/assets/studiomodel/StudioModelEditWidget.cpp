#include <QBoxLayout>

#include "entity/CHLMVStudioModelEntity.h"

#include "game/entity/CBaseEntity.h"
#include "game/entity/CBaseEntityList.h"
#include "game/entity/CEntityManager.h"
#include "graphics/Scene.hpp"

#include "ui/EditorUIContext.hpp"
#include "ui/SceneWidget.hpp"
#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelEditWidget.hpp"

#include "ui/assets/studiomodel/dockpanels/StudioModelDisplayPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelSequencesPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/Timeline.hpp"

#include "ui/camera_operators/ArcBallCameraOperator.hpp"
#include "ui/camera_operators/CameraOperator.hpp"

namespace ui::assets::studiomodel
{
StudioModelEditWidget::StudioModelEditWidget(EditorUIContext* editorContext, StudioModelAsset* asset, QWidget* parent)
	: QWidget(parent)
	, _asset(asset)
	, _scene(std::make_unique<graphics::Scene>(editorContext->GetSoundSystem()))
	, _context(new StudioModelContext(editorContext, asset, _scene.get(), this))
{
	_scene->FloorLength = editorContext->GetFloorLength();

	auto entity = static_cast<CHLMVStudioModelEntity*>(_scene->GetEntityContext()->EntityManager->Create("studiomodel", _scene->GetEntityContext(),
		glm::vec3(), glm::vec3(), false));

	if (nullptr != entity)
	{
		entity->SetModel(_asset->GetStudioModel());

		entity->Spawn();

		_scene->SetEntity(entity);
	}

	_cameraOperator = std::make_unique<camera_operators::ArcBallCameraOperator>();

	_sceneWidget = new SceneWidget(_scene.get(), this);

	_controlAreaWidget = new QWidget(this);

	_dockPanels = new QTabWidget(_controlAreaWidget);

	_dockPanels->setStyleSheet("QTabWidget::pane { padding: 0px; }");

	_dockPanels->addTab(new StudioModelDisplayPanel(_context), "Model Display");
	_dockPanels->addTab(new StudioModelSequencesPanel(_context), "Sequences");

	_timeline = new Timeline(_context, _controlAreaWidget);

	auto layout = new QVBoxLayout(this);

	layout->setContentsMargins(0, 0, 0, 0);

	layout->addWidget(_sceneWidget->GetContainer(), 1);

	layout->addWidget(_controlAreaWidget);

	setLayout(layout);

	{
		auto controlAreaLayout = new QVBoxLayout(_controlAreaWidget);

		controlAreaLayout->addWidget(_dockPanels);
		controlAreaLayout->addWidget(_timeline);

		_controlAreaWidget->setLayout(controlAreaLayout);
	}

	//TODO: need to initialize the background color to its default value here, as specified in the options dialog
	_context->SetBackgroundColor({63, 127, 127});

	//Listen to the main timer to update as needed
	connect(editorContext, &EditorUIContext::Tick, this, &StudioModelEditWidget::OnTick);
	connect(_sceneWidget, &SceneWidget::MouseEvent, this, &StudioModelEditWidget::OnMouseEvent);
	connect(editorContext, &EditorUIContext::FloorLengthChanged, this, &StudioModelEditWidget::OnFloorLengthChanged);
}

StudioModelEditWidget::~StudioModelEditWidget() = default;

void StudioModelEditWidget::OnTick()
{
	_scene->Tick();

	_sceneWidget->requestUpdate();

	emit _context->Tick();
}

void StudioModelEditWidget::OnMouseEvent(QMouseEvent* event)
{
	_cameraOperator->MouseEvent(*_scene->GetCamera(), *event);
}

void StudioModelEditWidget::OnFloorLengthChanged(int length)
{
	_scene->FloorLength = length;
}
}
