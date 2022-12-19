#include "ui/assets/studiomodel/StudioModelView.hpp"

namespace studiomodel
{
StudioModelView::StudioModelView(QWidget* parent)
	: QWidget(parent)
{
	_ui.setupUi(this);

	_ui.ViewSelection->setShape(QTabBar::Shape::RoundedSouth);
	_ui.ViewSelection->setToolTip("The current view");

	connect(_ui.ViewSelection, &QTabBar::currentChanged, this, &StudioModelView::SceneIndexChanged);
	connect(_ui.Pose, qOverload<int>(&QComboBox::currentIndexChanged), this, &StudioModelView::OnPoseChanged);
}

StudioModelView::~StudioModelView() = default;

int StudioModelView::GetSceneIndex() const
{
	return _ui.ViewSelection->currentIndex();
}

void StudioModelView::AddScene(const QString& label)
{
	_ui.ViewSelection->addTab(label);
}

void StudioModelView::SetWidget(QWidget* widget)
{
	if (_ui.SceneContainer->count() > 0)
	{
		_ui.SceneContainer->removeItem(_ui.SceneContainer->itemAt(0));
	}

	_ui.SceneContainer->addWidget(widget);
}

void StudioModelView::SetSceneIndex(int index)
{
	_ui.ViewSelection->setCurrentIndex(index);
}

void StudioModelView::OnPoseChanged(int index)
{
	emit PoseChanged(static_cast<Pose>(index));
}
}
