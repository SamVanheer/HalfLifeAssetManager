#include <cmath>

#include <QStyle>

#include "entity/HLMVStudioModelEntity.hpp"

#include "ui/StateSnapshot.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/dockpanels/Timeline.hpp"

namespace ui::assets::studiomodel
{
Timeline::Timeline(QWidget* parent)
	: QWidget(parent)
{
	_ui.setupUi(this);

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

	{
		const QSignalBlocker blockSlider(_ui.FramerateSlider);
		const QSignalBlocker blockSpinner(_ui.FramerateSpinner);

		_ui.FramerateSlider->setRange(SpeedMinimum * SpeedSliderMultiplier, SpeedMaximum * SpeedSliderMultiplier);
		_ui.FramerateSpinner->setRange(SpeedMinimum, SpeedMaximum);
	}

	_ui.FramerateSlider->setValue(SpeedDefault * SpeedSliderMultiplier);
	_ui.FramerateSpinner->setValue(SpeedDefault);

	InitializeUI();
}

Timeline::~Timeline() = default;

void Timeline::SetAsset(StudioModelAsset* asset)
{
	if (_asset == asset)
	{
		return;
	}

	if (_asset)
	{
		disconnect(_asset, &StudioModelAsset::Tick, this, &Timeline::OnTick);
		disconnect(_asset, &StudioModelAsset::LoadSnapshot, this, &Timeline::OnLoadSnapshot);
	}

	_asset = asset;

	if (_asset)
	{
		connect(_asset, &StudioModelAsset::Tick, this, &Timeline::OnTick);
		connect(_asset, &StudioModelAsset::LoadSnapshot, this, &Timeline::OnLoadSnapshot);
	}

	InitializeUI();
}

void Timeline::InitializeUI()
{
	if (_asset)
	{
		auto entity = _asset->GetScene()->GetEntity();
		auto model = entity->GetEditableModel();

		const int sequenceIndex = entity->GetSequence();

		const bool hasValidSequence = sequenceIndex >= 0 && sequenceIndex < model->Sequences.size();

		this->setEnabled(hasValidSequence);

		if (hasValidSequence)
		{
			SetFrame(entity->GetFrame(), false);
		}
		else
		{
			SetFrame(0, false);
		}

		SetFramerate(entity->GetFrameRate(), false);
	}
	else
	{
		SetFrame(0, false);
		SetFramerate(1, false);
		this->setEnabled(false);
	}
}

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

void Timeline::SetFramerate(double value, bool updateEntity)
{
	//Block change signals so we don't get stuck in recursive calls
	const QSignalBlocker blockSlider(_ui.FramerateSlider);
	const QSignalBlocker blockSpinner(_ui.FramerateSpinner);

	_ui.FramerateSlider->setValue(static_cast<int>(std::round(value * SpeedSliderMultiplier)));
	_ui.FramerateSpinner->setValue(value);

	if (updateEntity && _asset)
	{
		_asset->GetScene()->GetEntity()->SetFrameRate(static_cast<float>(value));
	}
}

void Timeline::ModifyFrame(int amount)
{
	auto entity = _asset->GetScene()->GetEntity();

	const auto& sequence = *entity->GetEditableModel()->Sequences[entity->GetSequence()];

	auto newFrameValue = entity->GetFrame() + amount;

	//TODO: the entity already has logic for wrapping animations. This needs to be incorporated into that
	if (newFrameValue < 0)
	{
		newFrameValue += (sequence.NumFrames - 1);
	}

	if (newFrameValue >= sequence.NumFrames)
	{
		newFrameValue -= sequence.NumFrames - 1;
	}

	entity->SetFrame(newFrameValue);
}

void Timeline::OnTick()
{
	auto entity = _asset->GetScene()->GetEntity();
	auto model = entity->GetEditableModel();

	const int sequenceIndex = entity->GetSequence();

	//TODO: need to make sure the last frame can be correctly set and played
	int frameRange;

	if (sequenceIndex != -1)
	{
		const auto& sequence = *model->Sequences[sequenceIndex];
		frameRange = sequence.NumFrames - 1;
	}
	else
	{
		frameRange = 1;
	}

	const int sliderRange = frameRange * FrameSliderRangeMultiplier;

	if (sliderRange != _ui.FrameSlider->maximum())
	{
		_ui.FrameSlider->setRange(0, frameRange * FrameSliderRangeMultiplier);
	}

	if (frameRange != _ui.FrameSpinner->maximum())
	{
		_ui.FrameSpinner->setRange(0, frameRange);
	}

	const double frame = entity->GetFrame();

	//Only update if the values differ by a meaningful amount
	//This prevents the UI from reverting user input and stops it from updating when the value hasn't changed
	if (std::abs( frame - _ui.FrameSpinner->value()) > 0.0001)
	{
		SetFrame(frame, false);
	}
}

void Timeline::OnLoadSnapshot(StateSnapshot* snapshot)
{
	InitializeUI();
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
	SetFramerate(static_cast<double>(_ui.FramerateSlider->value()) / SpeedSliderMultiplier, true);
}

void Timeline::OnFramerateSpinnerChanged()
{
	SetFramerate(_ui.FramerateSpinner->value(), true);
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
