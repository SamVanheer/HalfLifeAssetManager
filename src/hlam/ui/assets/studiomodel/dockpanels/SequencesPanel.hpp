#pragma once

#include <QSlider>
#include <QSpinBox>

#include "ui_SequencesPanel.h"

#include "entity/StudioModelEntity.hpp"
#include "formats/studiomodel/StudioModelFileFormat.hpp"

#include "ui/DockableWidget.hpp"
#include "ui/assets/studiomodel/StudioModelAsset.hpp"

namespace ui
{
class StateSnapshot;

namespace assets::studiomodel
{
class StudioModelAsset;
class StudioModelData;

class SequencesPanel final : public DockableWidget
{
public:
	SequencesPanel(StudioModelAsset* asset);
	~SequencesPanel();

private:
	enum class BlendUpdateSource
	{
		Slider,
		Spinner
	};

	void InitializeBlenders(float initialXValue = 0.f, float initialYValue = 0.f);

	void UpdateBlendValue(int blender, BlendUpdateSource source, QSlider* slider, QDoubleSpinBox* spinner);

private slots:
	void OnAssetChanged(StudioModelAsset* asset);

	void OnLoadSnapshot(StateSnapshot* snapshot);

	void OnPoseChanged(Pose pose);

	void OnSequenceChanged(int index);

	void OnLoopingModeChanged(int index);

	void OnBlendModeChanged(int index);

	void OnBlendXSliderChanged();

	void OnBlendXSpinnerChanged();

	void OnBlendYSliderChanged();

	void OnBlendYSpinnerChanged();

	void OnEventChanged(int index);

	void OnAddEvent();

	void OnRemoveEvent();

	void OnEventEdited();

private:
	Ui_SequencesPanel _ui;
	StudioModelAsset* const _asset;
	StudioModelData* _previousModelData{};

	double _blendsScales[SequenceBlendCount]{};
};
}
}