#include <QBoxLayout>
#include <QDockWidget>
#include <QMainWindow>
#include <QMap>

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
#include "ui/assets/studiomodel/dockpanels/StudioModelLightingPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelModelDataPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelModelDisplayPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelModelInfoPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelSequencesPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelTexturesPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/Timeline.hpp"

#include "ui/camera_operators/CameraOperator.hpp"
#include "ui/camera_operators/CameraOperators.hpp"

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

	_window = new QMainWindow(this);

	_centralWidget = new QWidget(_window);

	_sceneWidget = new SceneWidget(scene, _centralWidget);

	scene->SetGraphicsContext(std::make_unique<OpenGLGraphicsContext>(_sceneWidget));

	_window->setCentralWidget(_centralWidget);

	_window->setWindowFlags(Qt::WindowType::Widget);

	//Don't enable nested docks for now. The docks are so large they break the window's size and cause rendering problems
	_window->setDockOptions(QMainWindow::DockOption::AnimatedDocks | QMainWindow::DockOption::AllowTabbedDocks /* | QMainWindow::DockOption::AllowNestedDocks*/);

	_window->setTabPosition(Qt::DockWidgetArea::BottomDockWidgetArea, QTabWidget::TabPosition::North);

	_window->setDocumentMode(true);

	_camerasPanel = new camera_operators::CamerasPanel();

	auto cameraOperators = _asset->GetCameraOperators();

	for (int i = 0; i < cameraOperators->Count(); ++i)
	{
		const auto cameraOperator = cameraOperators->Get(i);
		_camerasPanel->AddCameraOperator(cameraOperator->GetName(), cameraOperator->CreateEditWidget());
	}

	OnAssetCameraChanged(nullptr, cameraOperators->GetCurrent());

	auto texturesPanel = new StudioModelTexturesPanel(_asset);

	auto addDockPanel = [&](QWidget* widget, const QString& label)
	{
		auto dock = new QDockWidget(label, _window);

		dock->setWidget(widget);
		dock->setObjectName(label);

		connect(dock, &QDockWidget::dockLocationChanged, this, &StudioModelEditWidget::OnDockLocationChanged);

		_window->addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, dock);

		return dock;
	};

	addDockPanel(_camerasPanel, "Cameras");
	addDockPanel(new StudioModelModelInfoPanel(_asset), "Model Info");
	auto modelDisplayDock = addDockPanel(new StudioModelModelDisplayPanel(_asset), "Model Display");
	addDockPanel(new StudioModelLightingPanel(_asset), "Lighting");
	addDockPanel(new StudioModelSequencesPanel(_asset), "Sequences");
	addDockPanel(new StudioModelBodyPartsPanel(_asset), "Body Parts");
	addDockPanel(texturesPanel, "Textures");
	addDockPanel(new StudioModelModelDataPanel(_asset), "Model Data");
	addDockPanel(new StudioModelBonesPanel(_asset), "Bones");
	addDockPanel(new StudioModelAttachmentsPanel(_asset), "Attachments");
	addDockPanel(new StudioModelHitboxesPanel(_asset), "Hitboxes");

	//Tabify all dock widgets
	{
		QMap<Qt::DockWidgetArea, QDockWidget*> firstDockWidgets;

		for (auto dock : _window->findChildren<QDockWidget*>())
		{
			const auto area = _window->dockWidgetArea(dock);

			if (auto it = firstDockWidgets.find(area); it != firstDockWidgets.end())
			{
				_window->tabifyDockWidget(it.value(), dock);
			}
			else
			{
				firstDockWidgets.insert(area, dock);
			}
		}
	}

	modelDisplayDock->raise();

	const auto infoBar = new InfoBar(_asset, _centralWidget);
	_timeline = new Timeline(_asset, this);

	auto layout = new QVBoxLayout(this);

	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);

	layout->addWidget(_window, 1);

	layout->addWidget(_timeline);

	setLayout(layout);

	{
		auto centralLayout = new QVBoxLayout(_centralWidget);

		centralLayout->setContentsMargins(0, 0, 0, 0);
		centralLayout->setSpacing(0);

		centralLayout->addWidget(_sceneWidget->GetContainer(), 1);
		centralLayout->addWidget(infoBar);

		_centralWidget->setLayout(centralLayout);
	}

	connect(cameraOperators, &camera_operators::CameraOperators::CameraChanged, this, &StudioModelEditWidget::OnAssetCameraChanged);
	connect(_camerasPanel, &camera_operators::CamerasPanel::CameraChanged, this, &StudioModelEditWidget::OnCameraChanged);

	connect(_sceneWidget, &SceneWidget::frameSwapped, infoBar, &InfoBar::OnDraw);
	connect(_sceneWidget, &SceneWidget::CreateDeviceResources, texturesPanel, &StudioModelTexturesPanel::OnCreateDeviceResources);
	//connect(this, &StudioModelEditWidget::DockPanelChanged, texturesPanel, &StudioModelTexturesPanel::OnDockPanelChanged);
}

StudioModelEditWidget::~StudioModelEditWidget() = default;

void StudioModelEditWidget::OnDockLocationChanged(Qt::DockWidgetArea area)
{
	auto dock = static_cast<QDockWidget*>(sender());

	auto widget = dock->widget();

	//Automatically change the layout for panels using a box layout
	if (auto layout = qobject_cast<QBoxLayout*>(widget->layout()); layout)
	{
		switch (area)
		{
		case Qt::DockWidgetArea::TopDockWidgetArea:
		case Qt::DockWidgetArea::BottomDockWidgetArea:
			layout->setDirection(QBoxLayout::Direction::LeftToRight);
			break;

		default:
			layout->setDirection(QBoxLayout::Direction::TopToBottom);
			break;
		}
	}
}

void StudioModelEditWidget::OnAssetCameraChanged(camera_operators::CameraOperator* previous, camera_operators::CameraOperator* current)
{
	int index = -1;

	for (int i = 0; i < _camerasPanel->GetCount(); ++i)
	{
		const auto widget = _camerasPanel->GetWidget(i);

		if (const auto candidate = widget->property(camera_operators::CameraOperatorPropertyKey.data()).value<camera_operators::CameraOperator*>();
			candidate == current)
		{
			index = i;
			break;
		}
	}

	_camerasPanel->ChangeCamera(index);
}

void StudioModelEditWidget::OnCameraChanged(int index)
{
	auto cameraOperators = _asset->GetCameraOperators();

	auto widget = _camerasPanel->GetWidget(index);

	camera_operators::CameraOperator* cameraOperator = nullptr;

	if (widget)
	{
		cameraOperator = widget->property(camera_operators::CameraOperatorPropertyKey.data()).value<camera_operators::CameraOperator*>();
	}
	
	cameraOperators->SetCurrent(cameraOperator);
}
}
