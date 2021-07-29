#include <algorithm>
#include <cassert>

#include <QDebug>

#include "qt/QtUtilities.hpp"

#include "ui/camera_operators/dockpanels/CamerasPanel.hpp"

namespace ui::camera_operators
{
CamerasPanel::CamerasPanel(QWidget* parent)
	: QWidget(parent)
{
	_ui.setupUi(this);

	connect(_ui.Cameras, qOverload<int>(&QComboBox::currentIndexChanged), this, &CamerasPanel::OnChangeCamera);
	connect(_ui.Previous, &QPushButton::clicked, this, &CamerasPanel::ChangeToPreviousCamera);
	connect(_ui.Next, &QPushButton::clicked, this, &CamerasPanel::ChangeToNextCamera);
}

CamerasPanel::~CamerasPanel() = default;

QWidget* CamerasPanel::GetWidget(int index) const
{
	assert(index >= 0 && index < _widgets.size());

	if (index < 0 || index >= _widgets.size())
	{
		return nullptr;
	}

	return _widgets[index];
}

void CamerasPanel::AddCameraOperator(const QString& name, QWidget* widget)
{
	assert(widget);

	if (std::find(_widgets.begin(), _widgets.end(), widget) != _widgets.end())
	{
		qDebug() << "Attempted to add duplicate camera operator";
		return;
	}

	_widgets.emplace_back(widget);

	//Add the widget before adding the combo box item because the combo box will select the first item that gets added automatically
	_ui.CamerasStack->addWidget(widget);
	_ui.Cameras->addItem(name);
}

void CamerasPanel::ChangeCamera(int index)
{
	_ui.Cameras->setCurrentIndex(index);
}

void CamerasPanel::OnChangeCamera(int index)
{
	_ui.CamerasStack->setCurrentIndex(index);

	emit CameraChanged(index);
}

void CamerasPanel::OnLayoutDirectionChanged(QBoxLayout::Direction direction)
{
	_ui.NavigationLayout->setDirection(direction);

	for (int i = 0; i < _ui.CamerasStack->count(); ++i)
	{
		auto widget = _ui.CamerasStack->widget(i);

		qt::TrySetBoxLayoutDirection(widget, direction);
	}

	_ui.CamerasStack->adjustSize();
	adjustSize();
}

void CamerasPanel::ChangeToPreviousCamera()
{
	if (_ui.Cameras->count() <= 0)
	{
		return;
	}

	int index = _ui.Cameras->currentIndex() - 1;

	if (index < 0)
	{
		index = _ui.Cameras->count() - 1;
	}

	_ui.Cameras->setCurrentIndex(index);
}

void CamerasPanel::ChangeToNextCamera()
{
	if (_ui.Cameras->count() <= 0)
	{
		return;
	}

	int index = _ui.Cameras->currentIndex() + 1;

	if (index >= _ui.Cameras->count())
	{
		index = 0;
	}

	_ui.Cameras->setCurrentIndex(index);
}
}
