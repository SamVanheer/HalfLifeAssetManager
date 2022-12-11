#include <QAction>
#include <QBoxLayout>
#include <QDockWidget>
#include <QMainWindow>
#include <QMap>

#include "graphics/Scene.hpp"

#include "qt/QtUtilities.hpp"

#include "ui/DragNDropEventFilter.hpp"
#include "ui/EditorContext.hpp"
#include "ui/SceneWidget.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelEditWidget.hpp"
#include "ui/assets/studiomodel/StudioModelView.hpp"

#include "ui/assets/studiomodel/dockpanels/InfoBar.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelAttachmentsPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelBodyPartsPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelBonesPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelFlagsPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelHitboxesPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelLightingPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelModelDataPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelModelDisplayPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelModelInfoPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelScenePanel.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelSequencesPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelTexturesPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelTransformPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/Timeline.hpp"

#include "ui/camera_operators/dockpanels/CamerasPanel.hpp"

#include "ui/settings/StudioModelSettings.hpp"

namespace ui::assets::studiomodel
{
StudioModelEditWidget::StudioModelEditWidget(EditorContext* editorContext, StudioModelAsset* asset)
	: _editorContext(editorContext)
	, _asset(asset)
{
	_ui.setupUi(this);

	_view = new StudioModelView(_ui.Window);

	_sceneWidget = new SceneWidget(this);

	auto eventFilter = _editorContext->GetDragNDropEventFilter();

	//The filter needs to be installed on the main window (handles dropping on any child widget),
	//as well as the scene widget (has special behavior due to being OpenGL)
	_ui.Window->installEventFilter(eventFilter);
	_sceneWidget->installEventFilter(eventFilter);

	for (auto scene : _asset->GetScenes())
	{
		_view->AddScene(QString::fromStdString(scene->GetName()));
	}

	_view->SetWidget(_sceneWidget->GetContainer());

	_ui.Window->setCentralWidget(_view);

	//Needed so the window will actually show up
	_ui.Window->setWindowFlags(Qt::WindowType::Widget);

	//Don't enable nested docks for now. The docks are so large they break the window's size and cause rendering problems
	_ui.Window->setDockOptions(QMainWindow::DockOption::AnimatedDocks | QMainWindow::DockOption::AllowTabbedDocks /* | QMainWindow::DockOption::AllowNestedDocks*/);

	_ui.Window->setTabPosition(Qt::DockWidgetArea::BottomDockWidgetArea, QTabWidget::TabPosition::North);

	_ui.Window->setDocumentMode(true);

	auto camerasPanel = new camera_operators::CamerasPanel(_asset->GetCameraOperators());

	auto scenePanel = new StudioModelScenePanel(_asset);
	auto bodyPartsPanel = new StudioModelBodyPartsPanel(_asset);
	auto texturesPanel = new StudioModelTexturesPanel(_asset);
	auto modelDataPanel = new StudioModelModelDataPanel(_asset);
	auto transformPanel = new StudioModelTransformPanel(_asset);

	auto addDockPanel = [&](QWidget* widget, const QString& label, Qt::DockWidgetArea area = Qt::DockWidgetArea::BottomDockWidgetArea)
	{
		auto dock = new QDockWidget(label, _ui.Window);

		dock->setWidget(widget);
		dock->setObjectName(label);

		connect(dock, &QDockWidget::dockLocationChanged, this, &StudioModelEditWidget::OnDockLocationChanged);

		_ui.Window->addDockWidget(area, dock);

		_dockWidgets.append(dock);

		return dock;
	};

	auto camerasDock = addDockPanel(camerasPanel, "Cameras");
	auto sceneDock = addDockPanel(scenePanel, "Scene");
	addDockPanel(new StudioModelModelInfoPanel(_asset), "Model Info");
	auto modelDisplayDock = addDockPanel(new StudioModelModelDisplayPanel(_asset), "Model Display");
	addDockPanel(new StudioModelLightingPanel(_asset), "Lighting");
	addDockPanel(new StudioModelSequencesPanel(_asset), "Sequences");
	auto bodyPartsDock = addDockPanel(bodyPartsPanel, "Body Parts");
	auto texturesDock = addDockPanel(texturesPanel, "Textures");
	auto modelDataDock = addDockPanel(modelDataPanel, "Model Data");
	auto flagsDock = addDockPanel(new StudioModelFlagsPanel(_asset), "Model Flags");
	addDockPanel(new StudioModelBonesPanel(_asset), "Bones");
	addDockPanel(new StudioModelAttachmentsPanel(_asset), "Attachments");
	addDockPanel(new StudioModelHitboxesPanel(_asset), "Hitboxes");
	auto transformDock = addDockPanel(transformPanel, "Transformation", Qt::DockWidgetArea::LeftDockWidgetArea);

	//Tabify all dock widgets except floating ones
	{
		QMap<Qt::DockWidgetArea, QDockWidget*> firstDockWidgets;

		for (auto dock : _dockWidgets)
		{
			const auto area = _ui.Window->dockWidgetArea(dock);

			if (area != Qt::DockWidgetArea::NoDockWidgetArea)
			{
				if (auto it = firstDockWidgets.find(area); it != firstDockWidgets.end())
				{
					_ui.Window->tabifyDockWidget(it.value(), dock);
				}
				else
				{
					firstDockWidgets.insert(area, dock);
				}
			}
		}
	}

	modelDisplayDock->raise();

	//Hidden by default
	flagsDock->setVisible(false);
	transformDock->setVisible(false);

	transformDock->toggleViewAction()->setShortcut(QKeySequence{Qt::CTRL + Qt::Key::Key_M});

	_view->GetInfoBar()->SetAsset(_asset);

	_ui.Timeline->SetAsset(_asset);

	connect(_view, &StudioModelView::SceneChanged, this, &StudioModelEditWidget::SetSceneIndex);

	connect(_view, &StudioModelView::PoseChanged, [this](int index)
		{
			_asset->SetPose(static_cast<Pose>(index));
		});
	connect(_sceneWidget, &SceneWidget::frameSwapped, _view->GetInfoBar(), &InfoBar::OnDraw);
	connect(_editorContext, &EditorContext::Tick, _view->GetInfoBar(), &InfoBar::OnTick);

	connect(camerasDock, &QDockWidget::dockLocationChanged, [camerasPanel, this](Qt::DockWidgetArea area)
		{
			camerasPanel->OnLayoutDirectionChanged(qt::GetDirectionForDockArea(area));
		});

	connect(sceneDock, &QDockWidget::dockLocationChanged, scenePanel, &StudioModelScenePanel::OnLayoutDirectionChanged);
	connect(bodyPartsDock, &QDockWidget::dockLocationChanged, bodyPartsPanel, &StudioModelBodyPartsPanel::OnLayoutDirectionChanged);
	connect(texturesDock, &QDockWidget::visibilityChanged, this, &StudioModelEditWidget::OnTexturesDockVisibilityChanged);
	connect(modelDataDock, &QDockWidget::dockLocationChanged, modelDataPanel, &StudioModelModelDataPanel::OnLayoutDirectionChanged);
	connect(transformDock, &QDockWidget::visibilityChanged, transformPanel, &StudioModelTransformPanel::ResetValues);
}

StudioModelEditWidget::~StudioModelEditWidget() = default;

graphics::Scene* StudioModelEditWidget::GetCurrentScene()
{
	const int index = _view->GetSceneIndex();

	if (index == -1)
	{
		return nullptr;
	}

	return _asset->GetScenes()[index];
}

void StudioModelEditWidget::SetSceneIndex(int index)
{
	_view->SetSceneIndex(index);
	_sceneWidget->SetScene(_asset->GetScenes()[index]);
}

void StudioModelEditWidget::OnDockLocationChanged(Qt::DockWidgetArea area)
{
	auto dock = static_cast<QDockWidget*>(sender());

	auto widget = dock->widget();

	//Automatically change the layout for panels using a box layout
	qt::TrySetBoxLayoutDirection(widget, qt::GetDirectionForDockArea(area));

	if (area != Qt::DockWidgetArea::NoDockWidgetArea)
	{
		//Force the window to resize the dock area to fit to the new set of dock widgets
		_ui.Window->resizeDocks({dock}, {0}, qt::GetOrientationForDockArea(area));
	}
}

void StudioModelEditWidget::OnTexturesDockVisibilityChanged(bool visible)
{
	if (_asset->GetProvider()->GetStudioModelSettings()->ShouldActivateTextureViewWhenTexturesPanelOpened())
	{
		auto scene = visible ? _asset->GetTextureScene() : _asset->GetScene();

		SetSceneIndex(_asset->GetScenes().indexOf(scene));
	}
}
}