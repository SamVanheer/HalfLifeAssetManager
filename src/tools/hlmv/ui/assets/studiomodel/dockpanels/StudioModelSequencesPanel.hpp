#pragma once

#include <QSlider>
#include <QSpinBox>
#include <QWidget>

#include "ui_StudioModelSequencesPanel.h"

namespace ui::assets::studiomodel
{
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

	void UpdateBlendValue(int blender, BlendUpdateSource source, QSlider* slider, QDoubleSpinBox* spinner);

private slots:
	void OnSequenceChanged(int index);

	void OnLoopingModeChanged(int index);

	void OnBlendXSliderChanged();

	void OnBlendXSpinnerChanged();

	void OnBlendYSliderChanged();

	void OnBlendYSpinnerChanged();

	void OnEditEvents();

	void OnEventChanged(int index);

	void OnPlaySoundChanged();

	void OnPitchFramerateAmplitudeChanged();

private:
	Ui_StudioModelSequencesPanel _ui;
	StudioModelAsset* const _asset;

	//TODO: define constant for blend count
	double _blendsScales[2];
};
}
