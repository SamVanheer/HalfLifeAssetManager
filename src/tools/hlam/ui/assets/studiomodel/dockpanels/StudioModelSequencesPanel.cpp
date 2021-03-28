#include <limits>

#include "engine/shared/activity.hpp"

#include "entity/HLMVStudioModelEntity.hpp"

#include "qt/ByteLengthValidator.hpp"

#include "soundsystem/ISoundSystem.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelUndoCommands.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelSequencesPanel.hpp"

namespace ui::assets::studiomodel
{
StudioModelSequencesPanel::StudioModelSequencesPanel(StudioModelAsset* asset, QWidget* parent)
	: QWidget(parent)
	, _asset(asset)
{
	_ui.setupUi(this);

	_ui.EventId->setRange(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
	_ui.EventType->setRange(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());

	_ui.PlaySound->setEnabled(_asset->GetScene()->GetEntityContext()->SoundSystem->IsSoundAvailable());
	_ui.PitchFramerateAmplitude->setEnabled(_asset->GetScene()->GetEntityContext()->SoundSystem->IsSoundAvailable());

	connect(_asset, &StudioModelAsset::ModelChanged, this, &StudioModelSequencesPanel::OnModelChanged);

	connect(_ui.SequenceComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &StudioModelSequencesPanel::OnSequenceChanged);
	connect(_ui.LoopingModeComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &StudioModelSequencesPanel::OnLoopingModeChanged);

	connect(_ui.BlendMode, qOverload<int>(&QComboBox::currentIndexChanged), this, &StudioModelSequencesPanel::OnBlendModeChanged);
	connect(_ui.BlendXSlider, &QSlider::valueChanged, this, &StudioModelSequencesPanel::OnBlendXSliderChanged);
	connect(_ui.BlendXSpinner, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelSequencesPanel::OnBlendXSpinnerChanged);
	connect(_ui.BlendYSlider, &QSlider::valueChanged, this, &StudioModelSequencesPanel::OnBlendYSliderChanged);
	connect(_ui.BlendYSpinner, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelSequencesPanel::OnBlendYSpinnerChanged);

	connect(_ui.EventsComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &StudioModelSequencesPanel::OnEventChanged);

	connect(_ui.PlaySound, &QCheckBox::stateChanged, this, &StudioModelSequencesPanel::OnPlaySoundChanged);
	connect(_ui.PitchFramerateAmplitude, &QCheckBox::stateChanged, this, &StudioModelSequencesPanel::OnPitchFramerateAmplitudeChanged);

	connect(_ui.EventFrameIndex, qOverload<int>(&QSpinBox::valueChanged), this, &StudioModelSequencesPanel::OnEventEdited);
	connect(_ui.EventId, qOverload<int>(&QSpinBox::valueChanged), this, &StudioModelSequencesPanel::OnEventEdited);
	connect(_ui.EventOptions, &QLineEdit::textChanged, this, &StudioModelSequencesPanel::OnEventEdited);
	connect(_ui.EventType, qOverload<int>(&QSpinBox::valueChanged), this, &StudioModelSequencesPanel::OnEventEdited);

	_ui.EventOptions->setValidator(new qt::ByteLengthValidator(STUDIO_MAX_EVENT_OPTIONS_LENGTH - 1, this));

	_ui.EventDataWidget->setEnabled(false);

	//Select the first page to make it clear it's the active page
	_ui.PageSelectorList->setCurrentRow(0);

	auto model = _asset->GetScene()->GetEntity()->GetEditableModel();

	QStringList sequences;

	sequences.reserve(model->Sequences.size());

	for (const auto& sequence : model->Sequences)
	{
		sequences.append(sequence->Label.c_str());
	}

	_ui.SequenceComboBox->addItems(sequences);
}

StudioModelSequencesPanel::~StudioModelSequencesPanel() = default;

void StudioModelSequencesPanel::OnModelChanged(const ModelChangeEvent& event)
{
	const auto model = _asset->GetScene()->GetEntity()->GetEditableModel();

	switch (event.GetId())
	{
	case ModelChangeId::ChangeEvent:
	{
		const auto& listChange = static_cast<const ModelEventChangeEvent&>(event);

		if (listChange.GetSourceIndex() == _ui.SequenceComboBox->currentIndex() && listChange.GetEventIndex() == _ui.EventsComboBox->currentIndex())
		{
			OnEventChanged(_ui.EventsComboBox->currentIndex());
		}
		break;
	}
	}
}

void StudioModelSequencesPanel::InitializeBlenders(const BlendMode mode)
{
	auto entity = _asset->GetScene()->GetEntity();

	const auto& sequence = *entity->GetEditableModel()->Sequences[entity->GetSequence()];

	const float initialBlendValue = 0.f;

	if (mode == BlendMode::CounterStrike)
	{
		entity->SetCounterStrikeBlending(0, initialBlendValue);
		entity->SetCounterStrikeBlending(1, initialBlendValue);
	}
	else
	{
		entity->SetBlending(0, initialBlendValue);
		entity->SetBlending(1, initialBlendValue);
	}

	QSlider* const sliders[] =
	{
		_ui.BlendXSlider,
		_ui.BlendYSlider
	};

	QDoubleSpinBox* const spinners[] =
	{
		_ui.BlendXSpinner,
		_ui.BlendYSpinner
	};

	for (int blender = 0; blender < SequenceBlendCount; ++blender)
	{
		const auto hasBlender = mode == BlendMode::CounterStrike || sequence.BlendData[blender].Type != 0;

		const auto slider = sliders[blender];

		const auto spinner = spinners[blender];

		if (hasBlender)
		{
			float start, end;

			if (mode == BlendMode::CounterStrike)
			{
				start = studiomdl::CounterStrikeBlendRanges[blender].Start;
				end = studiomdl::CounterStrikeBlendRanges[blender].End;
			}
			else
			{
				//Swap values if the range is inverted
				if (sequence.BlendData[blender].End < sequence.BlendData[blender].Start)
				{
					start = sequence.BlendData[blender].End;
					end = sequence.BlendData[blender].Start;
				}
				else
				{
					start = sequence.BlendData[blender].Start;
					end = sequence.BlendData[blender].End;
				}
			}

			//Should probably scale as needed so the range is sufficiently large
			//This prevents ranges that cover less than a whole integer from not doing anything
			if ((end - start) < 1.0f)
			{
				_blendsScales[blender] = 100.0f;
			}
			else
			{
				_blendsScales[blender] = 1.0f;
			}

			//Using this avoids the lossy round trip, which makes the value more accurate
			const auto value = initialBlendValue;

			slider->setRange(static_cast<int>(start * _blendsScales[blender]), static_cast<int>(end * _blendsScales[blender]));
			slider->setValue(value * _blendsScales[blender]);

			spinner->setRange(start, end);
			spinner->setValue(value);
		}
		else
		{
			//Make the slider be nice and centered
			slider->setRange(-1, 1);
			slider->setValue(0);

			spinner->setRange(-1, 1);
			spinner->setValue(0);
		}

		slider->setEnabled(hasBlender);
		spinner->setEnabled(hasBlender);
	}
}

void StudioModelSequencesPanel::UpdateBlendValue(int blender, BlendUpdateSource source, QSlider* slider, QDoubleSpinBox* spinner)
{
	if (source == BlendUpdateSource::Slider)
	{
		spinner->setValue(slider->value() / _blendsScales[blender]);
	}
	else
	{
		slider->setValue(spinner->value() * _blendsScales[blender]);
	}

	auto entity = _asset->GetScene()->GetEntity();

	if (static_cast<BlendMode>(_ui.BlendMode->currentIndex()) == BlendMode::CounterStrike)
	{
		entity->SetCounterStrikeBlending(blender, spinner->value());
	}
	else
	{
		entity->SetBlending(blender, spinner->value());
	}
}

void StudioModelSequencesPanel::OnSequenceChanged(int index)
{
	auto entity = _asset->GetScene()->GetEntity();

	entity->SetSequence(index);

	const auto& sequence = *entity->GetEditableModel()->Sequences[index];

	const auto durationInSeconds = sequence.NumFrames / sequence.FPS;

	_ui.SequenceLabel->setText(QString::number(index));
	_ui.FrameCountLabel->setText(QString::number(sequence.NumFrames));
	_ui.FPSLabel->setText(QString::number(sequence.FPS, 'g', 2));
	_ui.DurationLabel->setText(QString::number(durationInSeconds, 'g', 2));

	_ui.EventCountLabel->setText(QString::number(sequence.Events.size()));
	_ui.IsLoopingLabel->setText((sequence.Flags & STUDIO_LOOPING) ? "Yes" : "No");
	_ui.BlendCountLabel->setText(QString::number(sequence.AnimationBlends.size()));
	_ui.ActivityWeightLabel->setText(QString::number(sequence.ActivityWeight));

	QString activityName{"Unknown"};

	if (sequence.Activity >= ACT_IDLE && sequence.Activity <= ACT_FLINCH_RIGHTLEG)
	{
		activityName = activity_map[sequence.Activity - 1].name;
	}
	else if (sequence.Activity == ACT_RESET)
	{
		activityName = "None";
	}

	_ui.ActivityNameLabel->setText(QString{"%1 (%2)"}.arg(activityName).arg(sequence.Activity));

	InitializeBlenders(static_cast<BlendMode>(_ui.BlendMode->currentIndex()));

	_ui.EventsComboBox->clear();

	{
		const QSignalBlocker blocker{_ui.EventFrameIndex};
		_ui.EventFrameIndex->setRange(0, sequence.NumFrames - 1);
	}

	const bool hasEvents = !sequence.Events.empty();

	if (hasEvents)
	{
		QStringList events;

		events.reserve(sequence.Events.size());

		for (int i = 0; i < sequence.Events.size(); ++i)
		{
			events.append(QString("Event %1").arg(i + 1));
		}

		_ui.EventsComboBox->addItems(events);
	}

	_ui.EventsComboBox->setEnabled(hasEvents);
}

void StudioModelSequencesPanel::OnLoopingModeChanged(int index)
{
	_asset->GetScene()->GetEntity()->SetLoopingMode(static_cast<StudioLoopingMode>(index));
}

void StudioModelSequencesPanel::OnBlendModeChanged(int index)
{
	InitializeBlenders(static_cast<BlendMode>(index));
}

void StudioModelSequencesPanel::OnBlendXSliderChanged()
{
	UpdateBlendValue(SequenceBlendXIndex, BlendUpdateSource::Slider, _ui.BlendXSlider, _ui.BlendXSpinner);
}

void StudioModelSequencesPanel::OnBlendXSpinnerChanged()
{
	UpdateBlendValue(SequenceBlendXIndex, BlendUpdateSource::Spinner, _ui.BlendXSlider, _ui.BlendXSpinner);
}

void StudioModelSequencesPanel::OnBlendYSliderChanged()
{
	UpdateBlendValue(SequenceBlendYIndex, BlendUpdateSource::Slider, _ui.BlendYSlider, _ui.BlendYSpinner);
}

void StudioModelSequencesPanel::OnBlendYSpinnerChanged()
{
	UpdateBlendValue(SequenceBlendYIndex, BlendUpdateSource::Spinner, _ui.BlendYSlider, _ui.BlendYSpinner);
}

void StudioModelSequencesPanel::OnEventChanged(int index)
{
	const studiomdl::SequenceEvent emptyEvent{};

	const studiomdl::SequenceEvent* event = &emptyEvent;

	const bool hasEvent = index != -1;

	if (hasEvent)
	{
		const auto entity = _asset->GetScene()->GetEntity();
		const auto model = entity->GetEditableModel();
		const auto& sequence = *model->Sequences[entity->GetSequence()];

		event = sequence.Events[index].get();
	}

	const QSignalBlocker eventFrameIndex{_ui.EventFrameIndex};
	const QSignalBlocker eventId{_ui.EventId};
	const QSignalBlocker eventOptions{_ui.EventOptions};
	const QSignalBlocker eventType{_ui.EventType};

	_ui.EventFrameIndex->setValue(event->Frame);
	_ui.EventId->setValue(event->EventId);

	//Only call setText if the text is different
	//This prevents the edit state from being overwritten
	const QString options{event->Options.c_str()};

	if (_ui.EventOptions->text() != options)
	{
		_ui.EventOptions->setText(options);
	}

	_ui.EventType->setValue(event->Type);

	_ui.EventDataWidget->setEnabled(hasEvent);
}

void StudioModelSequencesPanel::OnPlaySoundChanged()
{
	_asset->GetScene()->GetEntity()->PlaySound = _ui.PlaySound->isChecked();
}

void StudioModelSequencesPanel::OnPitchFramerateAmplitudeChanged()
{
	_asset->GetScene()->GetEntity()->PitchFramerateAmplitude = _ui.PitchFramerateAmplitude->isChecked();
}

void StudioModelSequencesPanel::OnEventEdited()
{
	const auto entity = _asset->GetScene()->GetEntity();
	const auto model = entity->GetEditableModel();
	const auto& sequence = *model->Sequences[entity->GetSequence()];
	const auto& event = *sequence.Events[_ui.EventsComboBox->currentIndex()];

	auto changedEvent{event};

	changedEvent.Frame = _ui.EventFrameIndex->value();
	changedEvent.EventId = _ui.EventId->value();
	changedEvent.Type = _ui.EventType->value();

	changedEvent.Options = _ui.EventOptions->text().toStdString();

	_asset->AddUndoCommand(new ChangeEventCommand(_asset, _ui.SequenceComboBox->currentIndex(), _ui.EventsComboBox->currentIndex(), event, changedEvent));
}
}
