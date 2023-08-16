#include <limits>

#include <QAbstractItemView>
#include <QRegularExpressionValidator>

#include "entity/HLMVStudioModelEntity.hpp"

#include "formats/activity.hpp"

#include "qt/ByteLengthValidator.hpp"

#include "ui/StateSnapshot.hpp"

#include "plugins/halflife/studiomodel/StudioModelAsset.hpp"

#include "plugins/halflife/studiomodel/ui/StudioModelData.hpp"
#include "plugins/halflife/studiomodel/ui/StudioModelUndoCommands.hpp"
#include "plugins/halflife/studiomodel/ui/dockpanels/SequencesPanel.hpp"

namespace studiomodel
{
SequencesPanel::SequencesPanel(StudioModelAssetProvider* provider)
	: _provider(provider)
{
	_ui.setupUi(this);

	_ui.FPS->setRange(0, std::numeric_limits<double>::max());
	_ui.ActWeight->setRange(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
	_ui.EventId->setRange(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());

	{
		QStringList items;

		items.append("None (0)");

		for (const auto& activity : ActivityMap)
		{
			items.append(QString{"%1 (%2)"}.arg(activity.Name).arg(activity.Type));
		}

		_ui.Activity->addItems(items);

		// Set the popup to be wide enough to fit the longest name.
		_ui.Activity->view()->setMinimumWidth(_ui.Activity->minimumSizeHint().width());

		// Allow user to enter custom activities using compiler supported syntax.
		_ui.Activity->setValidator(new QRegularExpressionValidator(QRegularExpression{R"(^ACT_-?\d+$)"}, this));
		// Built-in activities can't be selected by typing them so auto-completing them is counter-intuitive.
		_ui.Activity->setCompleter(nullptr);
	}

	connect(_provider, &StudioModelAssetProvider::AssetChanged, this, &SequencesPanel::OnAssetChanged);

	connect(_ui.Sequences, qOverload<int>(&QComboBox::currentIndexChanged), this, &SequencesPanel::OnSequenceChanged);
	connect(_ui.LoopingMode, qOverload<int>(&QComboBox::currentIndexChanged), this, &SequencesPanel::OnLoopingModeChanged);

	connect(_ui.IsLooping, &QCheckBox::toggled, this, &SequencesPanel::OnSequencePropertiesChanged);
	connect(_ui.FPS, qOverload<double>(&QDoubleSpinBox::valueChanged),
		this, &SequencesPanel::OnSequencePropertiesChanged);
	connect(_ui.Activity, qOverload<int>(&QComboBox::currentIndexChanged),
		this, &SequencesPanel::OnSequencePropertiesChanged);
	connect(_ui.ActWeight, qOverload<int>(&QSpinBox::valueChanged),
		this, &SequencesPanel::OnSequencePropertiesChanged);
	connect(_ui.LinearMovement, &qt::widgets::ShortVector3Edit::ValueChanged,
		this, &SequencesPanel::OnSequencePropertiesChanged);

	connect(_ui.BlendMode, qOverload<int>(&QComboBox::currentIndexChanged), this, &SequencesPanel::OnBlendModeChanged);
	connect(_ui.BlendXSlider, &QSlider::valueChanged, this, &SequencesPanel::OnBlendXSliderChanged);
	connect(_ui.BlendXSpinner, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &SequencesPanel::OnBlendXSpinnerChanged);
	connect(_ui.BlendYSlider, &QSlider::valueChanged, this, &SequencesPanel::OnBlendYSliderChanged);
	connect(_ui.BlendYSpinner, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &SequencesPanel::OnBlendYSpinnerChanged);

	connect(_ui.Events, qOverload<int>(&QComboBox::currentIndexChanged), this, &SequencesPanel::OnEventChanged);

	connect(_ui.AddEvent, &QPushButton::clicked, this, &SequencesPanel::OnAddEvent);
	connect(_ui.RemoveEvent, &QPushButton::clicked, this, &SequencesPanel::OnRemoveEvent);

	connect(_ui.EventFrameIndex, qOverload<int>(&QSpinBox::valueChanged), this, &SequencesPanel::OnEventEdited);
	connect(_ui.EventId, qOverload<int>(&QSpinBox::valueChanged), this, &SequencesPanel::OnEventEdited);
	connect(_ui.EventOptions, &QLineEdit::textChanged, this, &SequencesPanel::OnEventEdited);

	_ui.EventOptions->setValidator(new qt::ByteLengthValidator(STUDIO_MAX_EVENT_OPTIONS_LENGTH - 1, this));

	_ui.EventDataWidget->setEnabled(false);

	OnAssetChanged(_provider->GetDummyAsset());
}

SequencesPanel::~SequencesPanel() = default;

void SequencesPanel::OnAssetChanged(StudioModelAsset* asset)
{
	if (_asset)
	{
		_asset->disconnect(this);
	}

	_asset = asset;

	auto modelData = _asset->GetModelData();

	connect(_asset, &StudioModelAsset::LoadSnapshot, this, &SequencesPanel::OnLoadSnapshot);
	connect(_asset, &StudioModelAsset::PoseChanged, this, &SequencesPanel::OnPoseChanged);

	const int sequenceIndex = _asset->GetEntity()->GetSequence();
	{
		const QSignalBlocker sequenceBlocker{_ui.Sequences};
		_ui.Sequences->setModel(modelData->Sequences);
		_ui.Sequences->setCurrentIndex(sequenceIndex);
	}
	OnSequenceChanged(sequenceIndex);

	this->setEnabled(_ui.Sequences->count() > 0);

	if (_previousModelData)
	{
		_previousModelData->DisconnectFromAll(this);
	}

	_previousModelData = modelData;

	connect(modelData->Sequences, &QAbstractItemModel::dataChanged, this,
		[this](const QModelIndex& topLeft, const QModelIndex& bottomRight)
		{
			if (topLeft.row() <= _ui.Sequences->currentIndex() && _ui.Sequences->currentIndex() <= bottomRight.row())
			{
				OnSequenceChanged(_ui.Sequences->currentIndex());
			}
		});

	connect(modelData, &StudioModelData::EventChanged, this, [this](int sequenceIndex, int eventIndex)
		{
			if (sequenceIndex == _ui.Sequences->currentIndex() && eventIndex == _ui.Events->currentIndex())
			{
				OnEventChanged(eventIndex);
			}
		});

	connect(modelData, &StudioModelData::EventAdded, this, [this](int sequenceIndex, int eventIndex)
		{
			if (sequenceIndex == _ui.Sequences->currentIndex())
			{
				auto entity = _asset->GetEntity();
				auto model = entity->GetEditableModel();
				auto& sequence = *model->Sequences[entity->GetSequence()];

				//Update UI to show new event
				{
					//Don't rely on the slot for this because the old index could match sometimes
					const QSignalBlocker blocker{_ui.Events};

					//Add new entry to the combo box
					_ui.Events->addItem(QString{"Event %1"}.arg(sequence.Events.size()));
					_ui.Events->setCurrentIndex(eventIndex);
					_ui.Events->setEnabled(true);
				}

				OnEventChanged(eventIndex);

				_ui.RemoveEvent->setEnabled(true);
			}
		});

	connect(modelData, &StudioModelData::EventRemoved, this, [this](int sequenceIndex, int eventIndex)
		{
			if (sequenceIndex == _ui.Sequences->currentIndex())
			{
				//Always remove the last entry since they're numbered independently of their associated event
				_ui.Events->removeItem(_ui.Events->count() - 1);

				const bool hasEvents = _ui.Events->count() > 0;

				//Sync up combo box and event data to the next event
				if (hasEvents)
				{
					const int newIndex = eventIndex > 0 ? eventIndex - 1 : 0;

					{
						const QSignalBlocker blocker{_ui.Events};
						_ui.Events->setCurrentIndex(newIndex);
					}

					OnEventChanged(newIndex);
				}

				_ui.Events->setEnabled(hasEvents);
				_ui.RemoveEvent->setEnabled(hasEvents);
			}
		});
}

void SequencesPanel::OnLoadSnapshot(StateSnapshot* snapshot)
{
	auto entity = _asset->GetEntity();
	const float xValue = entity->GetBlendingValue(0);
	const float yValue = entity->GetBlendingValue(1);

	//Sequence index is restored by the asset, no need to do this here

	//Reset blend values
	InitializeBlenders(xValue, yValue);
}

void SequencesPanel::OnPoseChanged(Pose pose)
{
	auto entity = _asset->GetEntity();

	switch (pose)
	{
	case Pose::Sequences:
		entity->SetSequence(_ui.Sequences->currentIndex());
		this->setEnabled(true);
		break;

	case Pose::Skeleton:
		entity->SetSequence(-1);
		this->setEnabled(false);
		break;
	}
}

void SequencesPanel::InitializeBlenders(float initialXValue, float initialYValue)
{
	const StudioBlendMode mode = static_cast<StudioBlendMode>(_ui.BlendMode->currentIndex());

	auto entity = _asset->GetEntity();

	const int sequenceIndex = entity->GetSequence();

	const studiomdl::StudioSequence emptySequence{};

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

void SequencesPanel::UpdateBlendValue(int blender, BlendUpdateSource source, QSlider* slider, QDoubleSpinBox* spinner)
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

void SequencesPanel::OnSequenceChanged(int index)
{
	const QSignalBlocker isLoopingBlocker{_ui.IsLooping};
	const QSignalBlocker fpsBlocker{_ui.FPS};
	const QSignalBlocker activityBlocker{_ui.Activity};
	const QSignalBlocker actWeightBlocker{_ui.ActWeight};
	const QSignalBlocker linearMovementBlocker{_ui.LinearMovement};

	auto entity = _asset->GetEntity();

	//Don't reset the frame unless the sequence has actually changed
	const bool sequenceHasChanged = (entity->GetSequence()) != index;

	if (sequenceHasChanged)
	{
		entity->SetSequence(index);
	}

	const studiomdl::StudioSequence emptySequence{};

	const auto& sequence = index != -1 ? *entity->GetEditableModel()->Sequences[index] : emptySequence;

	_ui.BlendCountLabel->setText(QString::number(sequence.AnimationBlends.size()));
	_ui.DurationLabel->setText(
		sequence.FPS != 0 ? QString::number(sequence.NumFrames / sequence.FPS, 'f', 2) : "Infinite");
	_ui.FrameCountLabel->setText(QString::number(sequence.NumFrames));

	_ui.IsLooping->setChecked((sequence.Flags & STUDIO_LOOPING) != 0);
	_ui.FPS->setValue(sequence.FPS);

	// Remove custom activities.
	while (_ui.Activity->count() > NumBuiltinActivities)
	{
		_ui.Activity->removeItem(NumBuiltinActivities);
	}

	// If this is a custom activity add it.
	if (sequence.Activity >= 0 && sequence.Activity < NumBuiltinActivities)
	{
		_ui.Activity->setCurrentIndex(sequence.Activity);
	}
	else
	{
		_ui.Activity->addItem(QString{"ACT_%1"}.arg(sequence.Activity));
		_ui.Activity->setCurrentIndex(NumBuiltinActivities);
		_ui.Activity->view()->setMinimumWidth(_ui.Activity->minimumSizeHint().width());
	}

	_ui.ActWeight->setValue(sequence.ActivityWeight);

	if (_ui.LinearMovement->GetValue() != sequence.LinearMovement)
	{
		_ui.LinearMovement->SetValue(sequence.LinearMovement);
	}

	if (sequenceHasChanged)
	{
		InitializeBlenders();
	}

	_ui.Events->clear();

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

		_ui.Events->addItems(events);
	}

	_ui.Events->setEnabled(hasEvents);
	_ui.RemoveEvent->setEnabled(hasEvents);
}

void SequencesPanel::OnLoopingModeChanged(int index)
{
	_asset->GetEntity()->SetLoopingMode(static_cast<StudioLoopingMode>(index));
}

void SequencesPanel::OnSequencePropertiesChanged()
{
	const auto& sequence = *_asset->GetEditableStudioModel()->Sequences[_ui.Sequences->currentIndex()];

	const SequenceProps oldProps
	{
		.IsLooping = (sequence.Flags & STUDIO_LOOPING) != 0,
		.FPS = sequence.FPS,
		.Activity = sequence.Activity,
		.ActivityWeight = sequence.ActivityWeight,
		.LinearMovement = sequence.LinearMovement
	};

	int currentActivity = _ui.Activity->currentIndex();

	if (currentActivity < 0 || currentActivity >= NumBuiltinActivities)
	{
		auto actText = _ui.Activity->currentText();
		actText.remove(0, 4);
		currentActivity = actText.toInt();
	}

	const SequenceProps newProps
	{
		.IsLooping = _ui.IsLooping->isChecked(),
		.FPS = static_cast<float>(_ui.FPS->value()),
		.Activity = currentActivity,
		.ActivityWeight = _ui.ActWeight->value(),
		.LinearMovement = _ui.LinearMovement->GetValue()
	};

	// This action will trigger a refresh which clears any unused custom activities.
	_asset->AddUndoCommand(new ChangeSequencePropsCommand(_asset, _ui.Sequences->currentIndex(), oldProps, newProps));
}

void SequencesPanel::OnBlendModeChanged(int index)
{
	InitializeBlenders();
}

void SequencesPanel::OnBlendXSliderChanged()
{
	UpdateBlendValue(SequenceBlendXIndex, BlendUpdateSource::Slider, _ui.BlendXSlider, _ui.BlendXSpinner);
}

void SequencesPanel::OnBlendXSpinnerChanged()
{
	UpdateBlendValue(SequenceBlendXIndex, BlendUpdateSource::Spinner, _ui.BlendXSlider, _ui.BlendXSpinner);
}

void SequencesPanel::OnBlendYSliderChanged()
{
	UpdateBlendValue(SequenceBlendYIndex, BlendUpdateSource::Slider, _ui.BlendYSlider, _ui.BlendYSpinner);
}

void SequencesPanel::OnBlendYSpinnerChanged()
{
	UpdateBlendValue(SequenceBlendYIndex, BlendUpdateSource::Spinner, _ui.BlendYSlider, _ui.BlendYSpinner);
}

void SequencesPanel::OnEventChanged(int index)
{
	const studiomdl::StudioSequenceEvent emptyEvent{};

	const studiomdl::StudioSequenceEvent* event = &emptyEvent;

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

	_ui.EventFrameIndex->setValue(event->Frame);
	_ui.EventId->setValue(event->EventId);

	//Only call setText if the text is different
	//This prevents the edit state from being overwritten
	const QString options{QString::fromStdString(event->Options)};

	if (_ui.EventOptions->text() != options)
	{
		_ui.EventOptions->setText(options);
	}

	_ui.EventDataWidget->setEnabled(hasEvent);
}

void SequencesPanel::OnAddEvent()
{
	_asset->AddUndoCommand(new AddRemoveEventCommand(_asset, AddRemoveType::Addition, _ui.Sequences->currentIndex(), _ui.Events->count(), {}));
}

void SequencesPanel::OnRemoveEvent()
{
	const int index = _ui.Events->currentIndex();

	auto entity = _asset->GetEntity();
	auto model = entity->GetEditableModel();
	auto& sequence = *model->Sequences[entity->GetSequence()];

	_asset->AddUndoCommand(new AddRemoveEventCommand(_asset, AddRemoveType::Removal, _ui.Sequences->currentIndex(), index, *sequence.Events[index]));
}

void SequencesPanel::OnEventEdited()
{
	const auto entity = _asset->GetEntity();
	const auto model = entity->GetEditableModel();
	const auto& sequence = *model->Sequences[entity->GetSequence()];
	const auto& event = *sequence.Events[_ui.Events->currentIndex()];

	auto changedEvent{event};

	changedEvent.Frame = _ui.EventFrameIndex->value();
	changedEvent.EventId = _ui.EventId->value();

	changedEvent.Options = _ui.EventOptions->text().toStdString();

	_asset->AddUndoCommand(new ChangeEventCommand(_asset, _ui.Sequences->currentIndex(), _ui.Events->currentIndex(), event, changedEvent));
}
}
