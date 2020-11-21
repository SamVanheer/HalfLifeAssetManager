#pragma once

#include <QSlider>
#include <QSpinBox>
#include <QWidget>

#include "ui_StudioModelSequencesPanel.h"

namespace ui::assets::studiomodel
{
class StudioModelContext;

class StudioModelSequencesPanel final : public QWidget
{
public:
	StudioModelSequencesPanel(StudioModelContext* context, QWidget* parent = nullptr);
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
	StudioModelContext* const _context;

	//TODO: define constant for blend count
	double _blendsScales[2];
};
}
