#include <QBoxLayout>

#include "graphics/Scene.hpp"

#include "ui/EditorContext.hpp"
#include "ui/SceneWidget.hpp"
#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelEditWidget.hpp"

#include "ui/assets/studiomodel/dockpanels/InfoBar.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelAttachmentsPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelBodyPartsPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelBonesPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelHitboxesPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelModelDataPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelModelDisplayPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelModelInfoPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelSequencesPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelTexturesPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/Timeline.hpp"

#include "ui/camera_operators/CameraOperator.hpp"

#include "ui/camera_operators/ArcBallCameraOperator.hpp"
#include "ui/camera_operators/FirstPersonCameraOperator.hpp"
#include "ui/camera_operators/dockpanels/CamerasPanel.hpp"

namespace ui::assets::studiomodel
{
StudioModelEditWidget::StudioModelEditWidget(
	EditorContext* editorContext, StudioModelAsset* asset, QWidget* parent)
	: QWidget(parent)
	, _asset(asset)
{
	const auto scene = _asset->GetScene();

	_sceneWidget = new SceneWidget(scene, this);

	scene->SetGraphicsContext(std::make_unique<OpenGLGraphicsContext>(_sceneWidget));

	_controlAreaWidget = new QWidget(this);

	_dockPanels = new QTabWidget(_controlAreaWidget);

	_dockPanels->setStyleSheet("QTabWidget::pane { padding: 0px; padding-left: 9px; }");

	_camerasPanel = new camera_operators::CamerasPanel();

	for (int i = 0; i < _asset->GetCameraOperatorCount(); ++i)
	{
		const auto cameraOperator = _asset->GetCameraOperator(i);
		_camerasPanel->AddCameraOperator(cameraOperator->GetName(), cameraOperator->CreateEditWidget());
	}

	auto modelDisplayPanel = new StudioModelModelDisplayPanel(_asset);
	auto texturesPanel = new StudioModelTexturesPanel(_asset);
	auto bonesPanel = new StudioModelBonesPanel(_asset);
	auto attachmentsPanel = new StudioModelAttachmentsPanel(_asset);
	auto hitboxesPanel = new StudioModelHitboxesPanel(_asset);

	_dockPanels->addTab(_camerasPanel, "Cameras");
	_dockPanels->addTab(new StudioModelModelInfoPanel(_asset), "Model Info");
	_dockPanels->addTab(modelDisplayPanel, "Model Display");
	_dockPanels->addTab(new StudioModelSequencesPanel(_asset), "Sequences");
	_dockPanels->addTab(new StudioModelBodyPartsPanel(_asset), "Body Parts");
	_dockPanels->addTab(texturesPanel, "Textures");
	_dockPanels->addTab(new StudioModelModelDataPanel(_asset), "Model Data");
	_dockPanels->addTab(bonesPanel, "Bones");
	_dockPanels->addTab(attachmentsPanel, "Attachments");
	_dockPanels->addTab(hitboxesPanel, "Hitboxes");

	_dockPanels->setCurrentWidget(modelDisplayPanel);

	const auto infoBar = new InfoBar(_asset, _controlAreaWidget);
	_timeline = new Timeline(_asset, _controlAreaWidget);

	auto layout = new QVBoxLayout(this);

	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);

	layout->addWidget(_sceneWidget->GetContainer(), 1);

	layout->addWidget(_controlAreaWidget);

	setLayout(layout);

	{
		auto controlAreaLayout = new QVBoxLayout(_controlAreaWidget);

		controlAreaLayout->setContentsMargins(0, 0, 0, 0);
		controlAreaLayout->setSpacing(0);

		controlAreaLayout->addWidget(infoBar);
		controlAreaLayout->addWidget(_dockPanels);
		controlAreaLayout->addWidget(_timeline);

		_controlAreaWidget->setLayout(controlAreaLayout);
	}

	connect(asset, &StudioModelAsset::Draw, _sceneWidget, &SceneWidget::requestUpdate);

	connect(_dockPanels, &QTabWidget::currentChanged, this, &StudioModelEditWidget::OnTabChanged);

	connect(asset, &StudioModelAsset::CameraChanged, this, &StudioModelEditWidget::OnAssetCameraChanged);
	connect(_camerasPanel, &camera_operators::CamerasPanel::CameraChanged, this, &StudioModelEditWidget::OnCameraChanged);

	connect(editorContext, &EditorContext::Tick, infoBar, &InfoBar::OnTick);
	connect(_sceneWidget, &SceneWidget::CreateDeviceResources, texturesPanel, &StudioModelTexturesPanel::OnCreateDeviceResources);
	connect(this, &StudioModelEditWidget::DockPanelChanged, texturesPanel, &StudioModelTexturesPanel::OnDockPanelChanged);

	connect(this, &StudioModelEditWidget::DockPanelChanged, bonesPanel, &StudioModelBonesPanel::OnDockPanelChanged);
	connect(this, &StudioModelEditWidget::DockPanelChanged, attachmentsPanel, &StudioModelAttachmentsPanel::OnDockPanelChanged);
	connect(this, &StudioModelEditWidget::DockPanelChanged, hitboxesPanel, &StudioModelHitboxesPanel::OnDockPanelChanged);
}

StudioModelEditWidget::~StudioModelEditWidget() = default;

void StudioModelEditWidget::OnTabChanged(int index)
{
	auto previous = _currentTab;
	_currentTab = _dockPanels->currentWidget();

	emit DockPanelChanged(_currentTab, previous);
}

void StudioModelEditWidget::OnAssetCameraChanged(camera_operators::CameraOperator* cameraOperator)
{
	int index = -1;

	for (int i = 0; i < _camerasPanel->GetCount(); ++i)
	{
		const auto widget = _camerasPanel->GetWidget(i);

		if (const auto candidate = widget->property(camera_operators::CameraOperatorPropertyKey.data()).value<camera_operators::CameraOperator*>();
			candidate == cameraOperator)
		{
			index = i;
			break;
		}
	}

	_camerasPanel->ChangeCamera(index);
}

void StudioModelEditWidget::OnCameraChanged(int index)
{
	auto widget = _camerasPanel->GetWidget(index);

	bool success{false};

	if (widget)
	{
		if (auto cameraOperator = widget->property(camera_operators::CameraOperatorPropertyKey.data()).value<camera_operators::CameraOperator*>();
			cameraOperator)
		{
			_asset->SetCurrentCameraOperator(cameraOperator);
			success = true;
		}
	}

	if (!success)
	{
		_asset->SetCurrentCameraOperator(nullptr);
	}
}
}
