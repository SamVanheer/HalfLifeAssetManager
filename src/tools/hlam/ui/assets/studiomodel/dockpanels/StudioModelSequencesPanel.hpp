#pragma once

#include <QSlider>
#include <QSpinBox>
#include <QWidget>

#include "ui_StudioModelSequencesPanel.h"

#include "engine/shared/studiomodel/StudioModelFileFormat.hpp"
#include "game/entity/StudioModelEntity.hpp"

namespace ui
{
class StateSnapshot;

namespace assets::studiomodel
{
class ModelChangeEvent;
class StudioModelAsset;

class StudioModelSequencesPanel final : public QWidget
{
public:
	StudioModelSequencesPanel(StudioModelAsset* asset, QWidget* parent = nullptr);
	~StudioModelSequencesPanel();

private:
	enum class BlendUpdateSource
	{
		Slider,
		Spinner
	};

	void InitializeUI();

	void InitializeBlenders(float initialXValue = 0.f, float initialYValue = 0.f);

	void UpdateBlendValue(int blender, BlendUpdateSource source, QSlider* slider, QDoubleSpinBox* spinner);

private slots:
	void OnModelChanged(const ModelChangeEvent& event);

	void OnLoadSnapshot(StateSnapshot* snapshot);

	void OnSequenceChanged(int index);

	void OnLoopingModeChanged(int index);

	void OnBlendModeChanged(int index);

	void OnBlendXSliderChanged();

	void OnBlendXSpinnerChanged();

	void OnBlendYSliderChanged();

	void OnBlendYSpinnerChanged();

	void OnEventChanged(int index);

	void OnPlaySoundChanged();

	void OnPitchFramerateAmplitudeChanged();

	void OnAddEvent();

	void OnRemoveEvent();

	void OnEventEdited();

private:
	Ui_StudioModelSequencesPanel _ui;
	StudioModelAsset* const _asset;

	double _blendsScales[SequenceBlendCount]{};
};
}
}
