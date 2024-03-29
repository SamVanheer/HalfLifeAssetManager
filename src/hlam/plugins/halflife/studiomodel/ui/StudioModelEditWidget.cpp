#include <algorithm>

#include <QAction>
#include <QDockWidget>
#include <QMainWindow>
#include <QMap>

#include "graphics/Scene.hpp"

#include "qt/QtUtilities.hpp"

#include "ui/DockableWidget.hpp"
#include "ui/DragNDropEventFilter.hpp"
#include "application/AssetManager.hpp"
#include "ui/SceneWidget.hpp"

#include "plugins/halflife/studiomodel/StudioModelAsset.hpp"
#include "plugins/halflife/studiomodel/ui/InfoBar.hpp"
#include "plugins/halflife/studiomodel/ui/StudioModelEditWidget.hpp"
#include "plugins/halflife/studiomodel/ui/StudioModelView.hpp"
#include "plugins/halflife/studiomodel/ui/Timeline.hpp"

#include "plugins/halflife/studiomodel/ui/dockpanels/AttachmentsPanel.hpp"
#include "plugins/halflife/studiomodel/ui/dockpanels/BodyPartsPanel.hpp"
#include "plugins/halflife/studiomodel/ui/dockpanels/BoneControllersPanel.hpp"
#include "plugins/halflife/studiomodel/ui/dockpanels/BonesPanel.hpp"
#include "plugins/halflife/studiomodel/ui/dockpanels/HitboxesPanel.hpp"
#include "plugins/halflife/studiomodel/ui/dockpanels/LightingPanel.hpp"
#include "plugins/halflife/studiomodel/ui/dockpanels/ModelDataPanel.hpp"
#include "plugins/halflife/studiomodel/ui/dockpanels/ModelDisplayPanel.hpp"
#include "plugins/halflife/studiomodel/ui/dockpanels/ScenePanel.hpp"
#include "plugins/halflife/studiomodel/ui/dockpanels/SequencesPanel.hpp"
#include "plugins/halflife/studiomodel/ui/dockpanels/TexturesPanel.hpp"

#include "plugins/halflife/studiomodel/ui/dockpanels/TransformPanel.hpp"

#include "ui/camera_operators/dockpanels/CamerasPanel.hpp"

namespace studiomodel
{
const int StateVersion = 0;

StudioModelEditWidget::StudioModelEditWidget(AssetManager* application, StudioModelAssetProvider* provider)
	: _application(application)
	, _provider(provider)
{
	_ui.setupUi(this);

	_view = new StudioModelView(_ui.Window);

	_timeline = new Timeline(_provider, this);

	_ui.MainLayout->addWidget(_timeline);

	_ui.Window->installEventFilter(_application->GetDragNDropEventFilter());

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

	_camerasPanel = new CamerasPanel(_provider->GetCameraOperators());

	addDockPanel(_camerasPanel, "Cameras");
	addDockPanel(new ScenePanel(_provider), "Scene");
	auto modelDisplayDock = addDockPanel(new ModelDisplayPanel(_provider), "Model Display");
	addDockPanel(new LightingPanel(_provider), "Lighting");
	addDockPanel(new SequencesPanel(_provider), "Sequences");
	addDockPanel(new BodyPartsPanel(_provider), "Body Parts");
	addDockPanel(new TexturesPanel(_provider), "Textures");
	addDockPanel(new ModelDataPanel(_provider), "Model Data");
	addDockPanel(new BonesPanel(_provider), "Bones");
	addDockPanel(new BoneControllersPanel(_provider), "Bone Controllers");
	addDockPanel(new AttachmentsPanel(_provider), "Attachments");
	addDockPanel(new HitboxesPanel(_provider), "Hitboxes");
	auto transformDock = addDockPanel(new TransformPanel(_provider), "Transformation");

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

	transformDock->toggleViewAction()->setShortcut(QKeySequence{Qt::CTRL + static_cast<int>(Qt::Key::Key_M)});

	connect(_view, &StudioModelView::SceneIndexChanged, this, &StudioModelEditWidget::SceneIndexChanged);
	connect(_view, &StudioModelView::PoseChanged, this, &StudioModelEditWidget::PoseChanged);

	SetAsset(_provider->GetDummyAsset());

	_initialState = SaveState();
}

StudioModelEditWidget::~StudioModelEditWidget() = default;

void StudioModelEditWidget::Tick()
{
	_view->GetInfoBar()->Tick();
	_timeline->Tick();
}

QByteArray StudioModelEditWidget::SaveState()
{
	return _ui.Window->saveState(StateVersion);
}

void StudioModelEditWidget::RestoreState(const QByteArray& state)
{
	_ui.Window->restoreState(state, StateVersion);
}

void StudioModelEditWidget::ResetToInitialState()
{
	RestoreState(_initialState);
}

void StudioModelEditWidget::AttachSceneWidget()
{
	auto sceneWidget = _application->GetSceneWidget();

	_view->SetWidget(sceneWidget->GetContainer());

	connect(sceneWidget, &SceneWidget::frameSwapped, _view->GetInfoBar(), &InfoBar::OnDraw, Qt::UniqueConnection);
}

void StudioModelEditWidget::DetachSceneWidget()
{
	auto sceneWidget = _application->GetSceneWidget();

	sceneWidget->disconnect(_view->GetInfoBar());
}

bool StudioModelEditWidget::IsControlsBarVisible() const
{
	// isVisible doesn't work here because it's not a top level widget.
	return !_view->GetControlsBar()->isHidden();
}

void StudioModelEditWidget::SetControlsBarVisible(bool state)
{
	_view->GetControlsBar()->setVisible(state);
}

bool StudioModelEditWidget::IsTimelineVisible() const
{
	return !_timeline->isHidden();
}

void StudioModelEditWidget::SetTimelineVisible(bool state)
{
	_timeline->setVisible(state);
}

bool StudioModelEditWidget::AreDockWidgetsVisible() const
{
	return _dockWidgetsVisible;
}

void StudioModelEditWidget::SetDockWidgetsVisible(bool state)
{
	if (_dockWidgetsVisible == state)
	{
		return;
	}

	_dockWidgetsVisible = state;

	if (state)
	{
		this->restoreGeometry(_savedDockWidgetsGeometry);
		this->RestoreState(_savedDockWidgetsState);
	}
	else
	{
		_savedDockWidgetsGeometry = this->saveGeometry();
		_savedDockWidgetsState = this->SaveState();

		for (auto dock : _dockWidgets)
		{
			dock->setVisible(false);
		}
	}
}

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

	_view->GetInfoBar()->SetAsset(asset);
	_timeline->SetAsset(asset);
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
		_ui.Window->resizeDocks({dock}, {1}, qt::GetOrientationForDockArea(area));
	}
}

void StudioModelEditWidget::OnDockVisibilityChanged(bool visible)
{
	auto dock = static_cast<QDockWidget*>(sender());
	auto widget = static_cast<DockableWidget*>(dock->widget());

	widget->OnVisibilityChanged(visible);
}
}
