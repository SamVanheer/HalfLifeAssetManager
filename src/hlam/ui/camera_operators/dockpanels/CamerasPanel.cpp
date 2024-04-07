#include <algorithm>
#include <cassert>

#include <QFontMetrics>
#include <QGridLayout>
#include <QLabel>
#include <QSignalBlocker>

#include "qt/QtUtilities.hpp"

#include "ui/camera_operators/CameraOperators.hpp"
#include "ui/camera_operators/dockpanels/CamerasPanel.hpp"

CamerasPanel::CamerasPanel(CameraOperators* cameraOperators)
	: _cameraOperators(cameraOperators)
{
	_ui.setupUi(this);

	connect(_ui.Cameras, qOverload<int>(&QComboBox::currentIndexChanged), this, &CamerasPanel::OnChangeCamera);
	connect(_ui.Previous, &QPushButton::clicked, this, &CamerasPanel::ChangeToPreviousCamera);
	connect(_ui.Next, &QPushButton::clicked, this, &CamerasPanel::ChangeToNextCamera);

	connect(_cameraOperators, &CameraOperators::CameraChanged, this, &CamerasPanel::OnAssetCameraChanged);

	{
		const QSignalBlocker camerasBlocker{_ui.Cameras};

		for (int i = 0; i < _cameraOperators->Count(); ++i)
		{
			const auto cameraOperator = _cameraOperators->Get(i);
			AddCameraOperator(cameraOperator->GetName(), cameraOperator->CreateEditWidget());
		}
	}

	OnAssetCameraChanged(nullptr, _cameraOperators->GetCurrent());
}

CamerasPanel::~CamerasPanel() = default;

QWidget* CamerasPanel::GetWidget(int index) const
{
	if (index < 0 || index >= _widgets.size())
	{
		return nullptr;
	}

	return _widgets[index];
}

static QVector<QLabel*> GetCameraLabels(QWidget* panel)
{
	QVector<QLabel*> labels;

	if (auto grid = qobject_cast<QGridLayout*>(panel->layout()); grid)
	{
		for (int i = 0; i < grid->rowCount(); ++i)
		{
			auto item = grid->itemAtPosition(i, 0);

			if (auto label = qobject_cast<QLabel*>(item->widget()); label)
			{
				labels.push_back(label);
			}
		}
	}

	return labels;
}

void CamerasPanel::AddCameraOperator(const QString& name, QWidget* widget)
{
	assert(widget);

	_widgets.emplace_back(widget);

	// Add the widget before adding the combo box item because the combo box
	// will select the first item that gets added automatically
	_ui.CamerasStack->addWidget(widget);
	_ui.Cameras->addItem(name);

	// Redo layout.
	int widthToSet = -1;

	for (auto widget : _widgets)
	{
		for (auto label : GetCameraLabels(widget))
		{
			QFontMetrics fm{ label->font() };

			const int width = fm.horizontalAdvance(label->text());

			if (width > widthToSet)
			{
				widthToSet = width;
			}
		}
	}

	for (auto widget : _widgets)
	{
		for (auto label : GetCameraLabels(widget))
		{
			label->setFixedWidth(widthToSet);
		}
	}
}

void CamerasPanel::ChangeCamera(int index)
{
	_ui.Cameras->setCurrentIndex(index);
}

void CamerasPanel::OnChangeCamera(int index)
{
	_ui.CamerasStack->setCurrentIndex(index);

	auto widget = GetWidget(index);

	SceneCameraOperator* cameraOperator = nullptr;

	if (widget)
	{
		cameraOperator = widget->property(CameraOperatorPropertyKey).value<SceneCameraOperator*>();
	}

	if (_cameraOperators)
	{
		_cameraOperators->SetCurrent(cameraOperator);
	}
}

void CamerasPanel::OnAssetCameraChanged(SceneCameraOperator* previous, SceneCameraOperator* current)
{
	int index = -1;

	for (int i = 0; i < GetCount(); ++i)
	{
		const auto widget = GetWidget(i);

		if (const auto candidate = widget->property(CameraOperatorPropertyKey).value<SceneCameraOperator*>();
			candidate == current)
		{
			index = i;
			break;
		}
	}

	ChangeCamera(index);
}

void CamerasPanel::OnLayoutDirectionChanged(QBoxLayout::Direction direction)
{
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
