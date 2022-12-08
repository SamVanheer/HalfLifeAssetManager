#include <limits>

#include "engine/shared/activity.hpp"

#include "entity/HLMVStudioModelEntity.hpp"

#include "qt/ByteLengthValidator.hpp"

#include "soundsystem/ISoundSystem.hpp"

#include "ui/StateSnapshot.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelUndoCommands.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelSequencesPanel.hpp"

#include "ui/settings/StudioModelSettings.hpp"

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
	connect(_asset, &StudioModelAsset::LoadSnapshot, this, &StudioModelSequencesPanel::OnLoadSnapshot);
	connect(_asset, &StudioModelAsset::PoseChanged, this, &StudioModelSequencesPanel::OnPoseChanged);

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

	connect(_ui.AddEvent, &QPushButton::clicked, this, &StudioModelSequencesPanel::OnAddEvent);
	connect(_ui.RemoveEvent, &QPushButton::clicked, this, &StudioModelSequencesPanel::OnRemoveEvent);

	connect(_ui.EventFrameIndex, qOverload<int>(&QSpinBox::valueChanged), this, &StudioModelSequencesPanel::OnEventEdited);
	connect(_ui.EventId, qOverload<int>(&QSpinBox::valueChanged), this, &StudioModelSequencesPanel::OnEventEdited);
	connect(_ui.EventOptions, &QLineEdit::textChanged, this, &StudioModelSequencesPanel::OnEventEdited);
	connect(_ui.EventType, qOverload<int>(&QSpinBox::valueChanged), this, &StudioModelSequencesPanel::OnEventEdited);

	_ui.EventOptions->setValidator(new qt::ByteLengthValidator(STUDIO_MAX_EVENT_OPTIONS_LENGTH - 1, this));

	_ui.EventDataWidget->setEnabled(false);

	InitializeUI();
}

StudioModelSequencesPanel::~StudioModelSequencesPanel() = default;

void StudioModelSequencesPanel::InitializeUI()
{
	auto entity = _asset->GetEntity();

	const int sequenceIndex = entity->GetSequence();

	auto model = _asset->GetEntity()->GetEditableModel();

	this->setEnabled(!model->Sequences.empty());

	_ui.SequenceComboBox->clear();

	QStringList sequences;

	sequences.reserve(model->Sequences.size());

	for (const auto& sequence : model->Sequences)
	{
		sequences.append(sequence->Label.c_str());
	}

	_ui.SequenceComboBox->addItems(sequences);

	_ui.SequenceComboBox->setCurrentIndex(sequenceIndex);
}

void StudioModelSequencesPanel::OnModelChanged(const ModelChangeEvent& event)
{
	const auto model = _asset->GetEntity()->GetEditableModel();

	switch (event.GetId())
	{
	case ModelChangeId::ChangeEvent:
	{
		const auto& listChange = static_cast<const ModelListSubListChangeEvent&>(event);

		if (listChange.GetSourceIndex() == _ui.SequenceComboBox->currentIndex() && listChange.GetSourceSubIndex() == _ui.EventsComboBox->currentIndex())
		{
			OnEventChanged(_ui.EventsComboBox->currentIndex());
		}
		break;
	}

	case ModelChangeId::AddRemoveEvent:
	{
		const auto& listChange = static_cast<const ModelListSubListAddRemoveEvent&>(event);

		if (listChange.GetSourceIndex() == _ui.SequenceComboBox->currentIndex())
		{
			const int index = listChange.GetSourceSubIndex();

			if (listChange.GetType() == AddRemoveType::Addition)
			{
				auto entity = _asset->GetEntity();
				auto model = entity->GetEditableModel();
				auto& sequence = *model->Sequences[entity->GetSequence()];

				//Update UI to show new event
				{
					//Don't rely on the slot for this because the old index could match sometimes
					const QSignalBlocker blocker{_ui.EventsComboBox};

					//Add new entry to the combo box
					_ui.EventsComboBox->addItem(QString{"Event %1"}.arg(sequence.Events.size()));
					_ui.EventsComboBox->setCurrentIndex(static_cast<int>(index));
					_ui.EventsComboBox->setEnabled(true);
				}

				OnEventChanged(static_cast<int>(index));

				_ui.RemoveEvent->setEnabled(true);
			}
			else
			{
				//Always remove the last entry since they're numbered independently of their associated event
				_ui.EventsComboBox->removeItem(_ui.EventsComboBox->count() - 1);

				const bool hasEvents = _ui.EventsComboBox->count() > 0;

				//Sync up combo box and event data to the next event
				if (hasEvents)
				{
					const int newIndex = index > 0 ? index - 1 : 0;

					{
						const QSignalBlocker blocker{_ui.EventsComboBox};
						_ui.EventsComboBox->setCurrentIndex(newIndex);
					}

					OnEventChanged(newIndex);
				}

				_ui.EventsComboBox->setEnabled(hasEvents);
				_ui.RemoveEvent->setEnabled(hasEvents);
			}
		}
		break;
	}
	}
}

void StudioModelSequencesPanel::OnLoadSnapshot(StateSnapshot* snapshot)
{
	auto entity = _asset->GetEntity();
	const float xValue = entity->GetBlendingValue(0);
	const float yValue = entity->GetBlendingValue(1);

	InitializeUI();

	//Sequence index is restored by the asset, no need to do this here

	//Reset blend values
	InitializeBlenders(xValue, yValue);
}

void StudioModelSequencesPanel::OnPoseChanged(Pose pose)
{
	auto entity = _asset->GetEntity();

	switch (pose)
	{
	case Pose::Sequences:
		entity->SetSequence(_ui.SequenceComboBox->currentIndex());
		this->setEnabled(true);
		break;

	case Pose::Skeleton:
		entity->SetSequence(-1);
		this->setEnabled(false);
		break;
	}
}

void StudioModelSequencesPanel::InitializeBlenders(float initialXValue, float initialYValue)
{
	const StudioBlendMode mode = static_cast<StudioBlendMode>(_ui.BlendMode->currentIndex());

	auto entity = _asset->GetEntity();

	const int sequenceIndex = entity->GetSequence();

	const studiomdl::Sequence emptySequence{};

	const auto& sequence = sequenceIndex != -1 ? *entity->GetEditableModel()->Sequences[sequenceIndex] : emptySequence;

	entity->SetBlendMode(mode);

	entity->SetBlending(0, initialXValue);
	entity->SetBlending(1, initialYValue);

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

	const float initialValues[] =
	{
		initialXValue,
		initialYValue
	};

	for (int blender = 0; blender < SequenceBlendCount; ++blender)
	{
		const auto hasBlender = mode == StudioBlendMode::CounterStrike || sequence.BlendData[blender].Type != 0;

		const auto slider = sliders[blender];

		const auto spinner = spinners[blender];

		if (hasBlender)
		{
			float start, end;

			if (mode == StudioBlendMode::CounterStrike)
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
			const auto value = initialValues[blender];

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

	auto entity = _asset->GetEntity();

	entity->SetBlending(blender, spinner->value());
}

void StudioModelSequencesPanel::OnSequenceChanged(int index)
{
	auto entity = _asset->GetEntity();

	//Don't reset the frame unless the sequence has actually changed
	const bool sequenceHasChanged = (entity->GetSequence()) != index;

	if (sequenceHasChanged)
	{
		entity->SetSequence(index);
	}

	const studiomdl::Sequence emptySequence{};

	const auto& sequence = index != -1 ? *entity->GetEditableModel()->Sequences[index] : emptySequence;

	const auto durationInSeconds = sequence.FPS != 0 ? sequence.NumFrames / sequence.FPS : 0;

	_ui.SequenceLabel->setText(QString::number(index));
	_ui.FrameCountLabel->setText(QString::number(sequence.NumFrames));
	_ui.FPSLabel->setText(QString::number(sequence.FPS, 'f', 2));
	_ui.DurationLabel->setText(QString::number(durationInSeconds, 'f', 2));

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

	_ui.ActivityName->setText(QString{"%1 (%2)"}.arg(activityName).arg(sequence.Activity));
	_ui.ActivityName->setCursorPosition(0);

	if (sequenceHasChanged)
	{
		InitializeBlenders();
	}

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
	_ui.RemoveEvent->setEnabled(hasEvents);

	_ui.EventsWidget->setEnabled(index != -1);
}

void StudioModelSequencesPanel::OnLoopingModeChanged(int index)
{
	_asset->GetEntity()->SetLoopingMode(static_cast<StudioLoopingMode>(index));
}

void StudioModelSequencesPanel::OnBlendModeChanged(int index)
{
	InitializeBlenders();
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
		const auto entity = _asset->GetEntity();
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
	_asset->GetProvider()->GetStudioModelSettings()->PlaySound = _ui.PlaySound->isChecked();
}

void StudioModelSequencesPanel::OnPitchFramerateAmplitudeChanged()
{
	_asset->GetProvider()->GetStudioModelSettings()->PitchFramerateAmplitude = _ui.PitchFramerateAmplitude->isChecked();
}

void StudioModelSequencesPanel::OnAddEvent()
{
	_asset->AddUndoCommand(new AddRemoveEventCommand(_asset, AddRemoveType::Addition, _ui.SequenceComboBox->currentIndex(), _ui.EventsComboBox->count(), {}));
}

void StudioModelSequencesPanel::OnRemoveEvent()
{
	const int index = _ui.EventsComboBox->currentIndex();

	auto entity = _asset->GetEntity();
	auto model = entity->GetEditableModel();
	auto& sequence = *model->Sequences[entity->GetSequence()];

	_asset->AddUndoCommand(new AddRemoveEventCommand(_asset, AddRemoveType::Removal, _ui.SequenceComboBox->currentIndex(), index, *sequence.Events[index]));
}

void StudioModelSequencesPanel::OnEventEdited()
{
	const auto entity = _asset->GetEntity();
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
