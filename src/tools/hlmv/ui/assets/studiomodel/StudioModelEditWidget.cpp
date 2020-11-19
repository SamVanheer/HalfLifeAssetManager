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

namespace ui::assets::studiomodel
{
StudioModelEditWidget::StudioModelEditWidget(EditorUIContext* editorContext, StudioModelAsset* asset, QWidget* parent)
	: QWidget(parent)
	, _asset(asset)
	, _scene(std::make_unique<graphics::Scene>())
	, _context(new StudioModelContext(asset, _scene.get(), this))
{
	//TODO: set up asset in scene
	auto entity = static_cast<CHLMVStudioModelEntity*>(_scene->GetEntityContext()->EntityManager->Create("studiomodel", _scene->GetEntityContext(),
		glm::vec3(), glm::vec3(), false));

	if (nullptr != entity)
	{
		entity->SetModel(_asset->GetStudioModel());

		entity->Spawn();

		_scene->SetEntity(entity);
	}

	_sceneWidget = new SceneWidget(_scene.get(), this);

	_dockPanels = new QTabWidget(this);

	_dockPanels->setStyleSheet("QTabWidget::pane { padding: 0px; }");

	_dockPanels->addTab(new StudioModelDisplayPanel(_context), "Model Display");

	auto layout = new QVBoxLayout(this);

	layout->setContentsMargins(0, 0, 0, 0);

	layout->addWidget(_sceneWidget->GetContainer(), 1);

	layout->addWidget(_dockPanels);

	setLayout(layout);

	//TODO: need to initialize the background color to its default value here, as specified in the options dialog
	_context->SetBackgroundColor({63, 127, 127});

	//Listen to the main timer to update as needed
	connect(editorContext, &EditorUIContext::Tick, this, &StudioModelEditWidget::OnTick);
}

StudioModelEditWidget::~StudioModelEditWidget() = default;

void StudioModelEditWidget::OnTick()
{
	_scene->Tick();

	_sceneWidget->requestUpdate();
}
}
