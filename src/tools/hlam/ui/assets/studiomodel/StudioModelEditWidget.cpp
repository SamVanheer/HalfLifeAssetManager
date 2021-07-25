#include <QAction>
#include <QBoxLayout>
#include <QDockWidget>
#include <QMainWindow>
#include <QMap>

#include "entity/HLMVStudioModelEntity.hpp"
#include "graphics/Scene.hpp"

#include "ui/EditorContext.hpp"
#include "ui/SceneWidget.hpp"
#include "ui/TextureWidget.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelColors.hpp"
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
#include "ui/assets/studiomodel/dockpanels/StudioModelSequencesPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelTexturesPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelTransformPanel.hpp"
#include "ui/assets/studiomodel/dockpanels/Timeline.hpp"

#include "ui/camera_operators/CameraOperator.hpp"
#include "ui/camera_operators/CameraOperators.hpp"

#include "ui/camera_operators/ArcBallCameraOperator.hpp"
#include "ui/camera_operators/FirstPersonCameraOperator.hpp"
#include "ui/camera_operators/dockpanels/CamerasPanel.hpp"

#include "ui/settings/ColorSettings.hpp"
#include "ui/settings/StudioModelSettings.hpp"

namespace ui::assets::studiomodel
{
StudioModelEditWidget::StudioModelEditWidget(
	EditorContext* editorContext, StudioModelAsset* asset, QWidget* parent)
	: QWidget(parent)
	, _editorContext(editorContext)
	, _asset(asset)
{
	_ui.setupUi(this);

	const auto scene = _asset->GetScene();

	_view = new StudioModelView(_ui.Window);

	_sceneWidget = new SceneWidget(scene, this);

	scene->SetGraphicsContext(std::make_unique<OpenGLGraphicsContext>(_sceneWidget));

	_textureWidget = new TextureWidget(this);

	SetTextureBackgroundColor();

	_view->AddWidget(_sceneWidget->GetContainer(), "Scene");
	_view->AddWidget(_textureWidget, "Texture");

	_ui.Window->setCentralWidget(_view);

	//Needed so the window will actually show up
	_ui.Window->setWindowFlags(Qt::WindowType::Widget);

	//Don't enable nested docks for now. The docks are so large they break the window's size and cause rendering problems
	_ui.Window->setDockOptions(QMainWindow::DockOption::AnimatedDocks | QMainWindow::DockOption::AllowTabbedDocks /* | QMainWindow::DockOption::AllowNestedDocks*/);

	_ui.Window->setTabPosition(Qt::DockWidgetArea::BottomDockWidgetArea, QTabWidget::TabPosition::North);

	_ui.Window->setDocumentMode(true);

	_camerasPanel = new camera_operators::CamerasPanel();

	auto cameraOperators = _asset->GetCameraOperators();

	for (int i = 0; i < cameraOperators->Count(); ++i)
	{
		const auto cameraOperator = cameraOperators->Get(i);
		_camerasPanel->AddCameraOperator(cameraOperator->GetName(), cameraOperator->CreateEditWidget());
	}

	OnAssetCameraChanged(nullptr, cameraOperators->GetCurrent());

	_texturesPanel = new StudioModelTexturesPanel(_asset);
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

	addDockPanel(_camerasPanel, "Cameras");
	addDockPanel(new StudioModelModelInfoPanel(_asset), "Model Info");
	auto modelDisplayDock = addDockPanel(new StudioModelModelDisplayPanel(_asset), "Model Display");
	addDockPanel(new StudioModelLightingPanel(_asset), "Lighting");
	addDockPanel(new StudioModelSequencesPanel(_asset), "Sequences");
	addDockPanel(new StudioModelBodyPartsPanel(_asset), "Body Parts");
	auto texturesDock = addDockPanel(_texturesPanel, "Textures");
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

	connect(cameraOperators, &camera_operators::CameraOperators::CameraChanged, this, &StudioModelEditWidget::OnAssetCameraChanged);
	connect(_camerasPanel, &camera_operators::CamerasPanel::CameraChanged, this, &StudioModelEditWidget::OnCameraChanged);

	connect(_sceneWidget, &SceneWidget::frameSwapped, _view->GetInfoBar(), &InfoBar::OnDraw);
	connect(_sceneWidget, &SceneWidget::CreateDeviceResources, _texturesPanel, &StudioModelTexturesPanel::OnCreateDeviceResources);

	connect(texturesDock, &QDockWidget::visibilityChanged, this, &StudioModelEditWidget::OnTexturesDockVisibilityChanged);
	connect(modelDataDock, &QDockWidget::dockLocationChanged, modelDataPanel, &StudioModelModelDataPanel::OnLayoutDirectionChanged);
	connect(transformDock, &QDockWidget::visibilityChanged, transformPanel, &StudioModelTransformPanel::ResetValues);

	connect(_editorContext->GetColorSettings(), &settings::ColorSettings::ColorsChanged, this, &StudioModelEditWidget::SetTextureBackgroundColor);
	connect(_texturesPanel, &StudioModelTexturesPanel::CurrentTextureChanged, _textureWidget, &TextureWidget::ResetImagePosition);
	connect(_texturesPanel, &StudioModelTexturesPanel::TextureViewChanged, this, &StudioModelEditWidget::OnTextureViewChanged);

	connect(_textureWidget, &TextureWidget::ScaleChanged, _texturesPanel, &StudioModelTexturesPanel::AdjustScale);

	//TODO: should happen automatically
	OnTextureViewChanged();
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

void StudioModelEditWidget::OnTexturesDockVisibilityChanged(bool visible)
{
	if (_asset->GetProvider()->GetStudioModelSettings()->ShouldActivateTextureViewWhenTexturesPanelOpened() && visible)
	{
		_view->SetCurrentWidget(_textureWidget);
	}
}

void StudioModelEditWidget::SetTextureBackgroundColor()
{
	_textureWidget->SetBackgroundColor(_editorContext->GetColorSettings()->GetColor(BackgroundColor.Name));
}

void StudioModelEditWidget::OnTextureViewChanged()
{
	_textureWidget->SetImage(_texturesPanel->GenerateTextureForDisplay());
}
}
