#include <cmath>

#include <QStyle>

#include "entity/CHLMVStudioModelEntity.h"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/dockpanels/Timeline.hpp"

namespace ui::assets::studiomodel
{
Timeline::Timeline(StudioModelAsset* asset, QWidget* parent)
	: QWidget(parent)
	, _asset(asset)
{
	_ui.setupUi(this);

	connect(_asset, &StudioModelAsset::Tick, this, &Timeline::OnTick);

	connect(_ui.FrameSlider, &QSlider::valueChanged, this, &Timeline::OnFrameSliderChanged);
	connect(_ui.FrameSpinner, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Timeline::OnFrameSpinnerChanged);

	connect(_ui.FramerateSlider, &QSlider::valueChanged, this, &Timeline::OnFramerateSliderChanged);
	connect(_ui.FramerateSpinner, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Timeline::OnFramerateSpinnerChanged);

	connect(_ui.Restart, &QPushButton::clicked, this, &Timeline::OnRestart);
	connect(_ui.TogglePlayback, &QPushButton::clicked, this, &Timeline::OnTogglePlayback);
	connect(_ui.PreviousFrame, &QPushButton::clicked, this, &Timeline::OnPreviousFrame);
	connect(_ui.SkipFramesEarlier, &QPushButton::clicked, this, &Timeline::OnSkipFramesEarlier);
	connect(_ui.NextFrame, &QPushButton::clicked, this, &Timeline::OnNextFrame);
	connect(_ui.SkipFramesLater, &QPushButton::clicked, this, &Timeline::OnSkipFramesLater);

	auto style = this->style();

	_ui.Restart->setIcon(style->standardIcon(QStyle::StandardPixmap::SP_BrowserReload));
	_ui.PreviousFrame->setIcon(style->standardIcon(QStyle::StandardPixmap::SP_MediaSeekBackward));
	_ui.SkipFramesEarlier->setIcon(style->standardIcon(QStyle::StandardPixmap::SP_MediaSkipBackward));
	_ui.TogglePlayback->setIcon(style->standardIcon(QStyle::StandardPixmap::SP_MediaPause));
	_ui.SkipFramesLater->setIcon(style->standardIcon(QStyle::StandardPixmap::SP_MediaSkipForward));
	_ui.NextFrame->setIcon(style->standardIcon(QStyle::StandardPixmap::SP_MediaSeekForward));

	_ui.FramerateSlider->setRange(SpeedMinimum * SpeedSliderMultiplier, SpeedMaximum * SpeedSliderMultiplier);
	_ui.FramerateSlider->setValue(SpeedDefault * SpeedSliderMultiplier);

	_ui.FramerateSpinner->setRange(SpeedMinimum, SpeedMaximum);
	_ui.FramerateSpinner->setValue(SpeedDefault);
}

Timeline::~Timeline() = default;

void Timeline::SetFrame(double value, bool updateEntity)
{
	//Block change signals so we don't get stuck in recursive calls
	const QSignalBlocker blockSlider(_ui.FrameSlider);
	const QSignalBlocker blockSpinner(_ui.FrameSpinner);

	_ui.FrameSlider->setValue(static_cast<int>(std::floor(value * FrameSliderRangeMultiplier)));
	_ui.FrameSpinner->setValue(value);

	if (updateEntity)
	{
		_asset->GetScene()->GetEntity()->SetFrame(static_cast<float>(value));
	}
}

void Timeline::SetFramerate(double value)
{
	//Block change signals so we don't get stuck in recursive calls
	const QSignalBlocker blockSlider(_ui.FramerateSlider);
	const QSignalBlocker blockSpinner(_ui.FramerateSpinner);

	_ui.FramerateSlider->setValue(static_cast<int>(std::round(value) * SpeedSliderMultiplier));
	_ui.FramerateSpinner->setValue(value);

	_asset->GetScene()->GetEntity()->SetFrameRate(static_cast<float>(value));
}

void Timeline::ModifyFrame(int amount)
{
	auto entity = _asset->GetScene()->GetEntity();

	const auto sequence = entity->GetModel()->GetStudioHeader()->GetSequence(entity->GetSequence());

	auto newFrameValue = entity->GetFrame() + amount;

	//TODO: the entity already has logic for wrapping animations. This needs to be incorporated into that
	if (newFrameValue < 0)
	{
		newFrameValue += (sequence->numframes - 1);
	}

	if (newFrameValue >= sequence->numframes)
	{
		newFrameValue -= sequence->numframes - 1;
	}

	//TODO: needs to be passed as a float
	entity->SetFrame(newFrameValue);
}

void Timeline::OnTick()
{
	auto entity = _asset->GetScene()->GetEntity();

	const auto sequence = entity->GetModel()->GetStudioHeader()->GetSequence(entity->GetSequence());

	//TODO: need to make sure the last frame can be correctly set and played
	const int frameRange = sequence->numframes - 1;

	_ui.FrameSlider->setRange(0, frameRange * FrameSliderRangeMultiplier);
	_ui.FrameSpinner->setRange(0, frameRange);

	SetFrame(entity->GetFrame(), false);
}

void Timeline::OnFrameSliderChanged()
{
	SetFrame(static_cast<double>(_ui.FrameSlider->value()) / FrameSliderRangeMultiplier, true);
}

void Timeline::OnFrameSpinnerChanged()
{
	SetFrame(_ui.FrameSpinner->value(), true);
}

void Timeline::OnFramerateSliderChanged()
{
	SetFramerate(static_cast<double>(_ui.FramerateSlider->value()) / SpeedSliderMultiplier);
}

void Timeline::OnFramerateSpinnerChanged()
{
	SetFramerate(_ui.FramerateSpinner->value());
}

void Timeline::OnRestart()
{
	_asset->GetScene()->GetEntity()->SetFrame(0);
}

void Timeline::OnTogglePlayback()
{
	auto entity = _asset->GetScene()->GetEntity();

	entity->PlaySequence = !entity->PlaySequence;

	auto style = this->style();

	if (entity->PlaySequence)
	{
		_ui.TogglePlayback->setIcon(style->standardIcon(QStyle::StandardPixmap::SP_MediaPause));
	}
	else
	{
		_ui.TogglePlayback->setIcon(style->standardIcon(QStyle::StandardPixmap::SP_MediaPlay));
	}
}

void Timeline::OnPreviousFrame()
{
	ModifyFrame(-1);
}

void Timeline::OnSkipFramesEarlier()
{
	ModifyFrame(-SkipFramesAmount);
}

void Timeline::OnNextFrame()
{
	ModifyFrame(1);
}

void Timeline::OnSkipFramesLater()
{
	ModifyFrame(SkipFramesAmount);
}
}
