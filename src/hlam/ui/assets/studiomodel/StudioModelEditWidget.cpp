#include <algorithm>

#include <QAction>
#include <QDockWidget>
#include <QMainWindow>
#include <QMap>

#include "graphics/Scene.hpp"

#include "qt/QtUtilities.hpp"

#include "ui/DockableWidget.hpp"
#include "ui/DragNDropEventFilter.hpp"
#include "ui/EditorContext.hpp"
#include "ui/SceneWidget.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelEditWidget.hpp"
#include "ui/assets/studiomodel/StudioModelView.hpp"

#include "ui/assets/studiomodel/dockpanels/AttachmentsPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/BodyPartsPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/BonesPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/FlagsPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/HitboxesPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/InfoBar.hpp"
#include "ui/assets/studiomodel/dockpanels/LightingPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/ModelDataPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/ModelDisplayPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/ModelInfoPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/ScenePanel.hpp"
#include "ui/assets/studiomodel/dockpanels/SequencesPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/TexturesPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/Timeline.hpp"
#include "ui/assets/studiomodel/dockpanels/TransformPanel.hpp"

#include "ui/camera_operators/dockpanels/CamerasPanel.hpp"

namespace studiomodel
{
StudioModelEditWidget::StudioModelEditWidget(EditorContext* editorContext, StudioModelAssetProvider* provider)
	: _editorContext(editorContext)
	, _provider(provider)
{
	_ui.setupUi(this);

	_view = new StudioModelView(_ui.Window);

	_timeline = new Timeline(_provider, this);

	_ui.MainLayout->addWidget(_timeline);

	_sceneWidget = new SceneWidget(
		editorContext,
		editorContext->GetOpenGLFunctions(), editorContext->GetTextureLoader(),
		this);

	auto eventFilter = _editorContext->GetDragNDropEventFilter();

	//The filter needs to be installed on the main window (handles dropping on any child widget),
	//as well as the scene widget (has special behavior due to being OpenGL)
	_ui.Window->installEventFilter(eventFilter);
	_sceneWidget->installEventFilter(eventFilter);

	_view->SetWidget(_sceneWidget->GetContainer());

	_ui.Window->setCentralWidget(_view);

	//Needed so the window will actually show up
	_ui.Window->setWindowFlags(Qt::WindowType::Widget);

	//Don't enable nested docks for now. The docks are so large they break the window's size and cause rendering problems
	_ui.Window->setDockOptions(QMainWindow::DockOption::AnimatedDocks | QMainWindow::DockOption::AllowTabbedDocks /* | QMainWindow::DockOption::AllowNestedDocks*/);

	_ui.Window->setTabPosition(Qt::DockWidgetArea::BottomDockWidgetArea, QTabWidget::TabPosition::North);

	_ui.Window->setDocumentMode(true);

	auto addDockPanel = [&](DockableWidget* widget, const QString& label, Qt::DockWidgetArea area = Qt::DockWidgetArea::BottomDockWidgetArea)
	{
		auto dock = new QDockWidget(label, _ui.Window);

		dock->setWidget(widget);
		dock->setObjectName(label);

		connect(dock, &QDockWidget::dockLocationChanged, this, &StudioModelEditWidget::OnDockLocationChanged);
		connect(dock, &QDockWidget::visibilityChanged, this, &StudioModelEditWidget::OnDockVisibilityChanged);

		_ui.Window->addDockWidget(area, dock);

		_dockWidgets.append(dock);

		return dock;
	};

	_camerasPanel = new CamerasPanel();

	addDockPanel(_camerasPanel, "Cameras");
	addDockPanel(new ScenePanel(_provider), "Scene");
	addDockPanel(new ModelInfoPanel(_provider), "Model Info");
	auto modelDisplayDock = addDockPanel(new ModelDisplayPanel(_provider), "Model Display");
	addDockPanel(new LightingPanel(_provider), "Lighting");
	addDockPanel(new SequencesPanel(_provider), "Sequences");
	addDockPanel(new BodyPartsPanel(_provider), "Body Parts");
	addDockPanel(new TexturesPanel(_provider), "Textures");
	addDockPanel(new ModelDataPanel(_provider), "Model Data");
	auto flagsDock = addDockPanel(new FlagsPanel(_provider), "Model Flags");
	addDockPanel(new BonesPanel(_provider), "Bones");
	addDockPanel(new AttachmentsPanel(_provider), "Attachments");
	addDockPanel(new HitboxesPanel(_provider), "Hitboxes");
	auto transformDock = addDockPanel(new TransformPanel(_provider), "Transformation", Qt::DockWidgetArea::LeftDockWidgetArea);

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

	connect(_view, &StudioModelView::SceneIndexChanged, this, &StudioModelEditWidget::SceneIndexChanged);
	connect(_view, &StudioModelView::PoseChanged, this, &StudioModelEditWidget::PoseChanged);
	connect(_sceneWidget, &SceneWidget::frameSwapped, _view->GetInfoBar(), &InfoBar::OnDraw);
	connect(_editorContext, &EditorContext::Tick, _view->GetInfoBar(), &InfoBar::OnTick);

	SetAsset(_provider->GetDummyAsset());
}

StudioModelEditWidget::~StudioModelEditWidget() = default;

int StudioModelEditWidget::GetSceneIndex() const
{
	return _view->GetSceneIndex();
}

void StudioModelEditWidget::SetAsset(StudioModelAsset* asset)
{
	{
		const QSignalBlocker viewBlocker{_view};
		_view->Clear();

		for (auto scene : asset->GetScenes())
		{
			_view->AddScene(QString::fromStdString(scene->GetName()));
		}
	}

	const auto& scenes = asset->GetScenes();

	const int index = std::find(scenes.begin(), scenes.end(), asset->GetCurrentScene()) - scenes.begin();

	_view->SetSceneIndex(index);
	_sceneWidget->SetScene(asset->GetCurrentScene());

	_view->GetInfoBar()->SetAsset(asset);
	_timeline->SetAsset(asset);

	_camerasPanel->SetCameraOperators(asset->GetCameraOperators());
}

void StudioModelEditWidget::SetSceneIndex(int index)
{
	_view->SetSceneIndex(index);
}

void StudioModelEditWidget::OnDockLocationChanged(Qt::DockWidgetArea area)
{
	auto dock = static_cast<QDockWidget*>(sender());
	auto widget = static_cast<DockableWidget*>(dock->widget());

	widget->OnLayoutDirectionChanged(qt::GetDirectionForDockArea(area));

	if (area != Qt::DockWidgetArea::NoDockWidgetArea)
	{
		//Force the window to resize the dock area to fit to the new set of dock widgets
		_ui.Window->resizeDocks({dock}, {0}, qt::GetOrientationForDockArea(area));
	}
}

void StudioModelEditWidget::OnDockVisibilityChanged(bool visible)
{
	auto dock = static_cast<QDockWidget*>(sender());
	auto widget = static_cast<DockableWidget*>(dock->widget());

	widget->OnVisibilityChanged(visible);
}
}
