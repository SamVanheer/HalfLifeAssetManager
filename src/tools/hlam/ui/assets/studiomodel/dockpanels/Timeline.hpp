#pragma once

#include <QWidget>

#include "ui_Timeline.h"

namespace ui::assets::studiomodel
{
class StudioModelAsset;

class Timeline final : public QWidget
{
public:
	static constexpr int SpeedMinimum = 0;
	static constexpr int SpeedMaximum = 10;
	static constexpr int SpeedDefault = 1;
	static constexpr int SpeedSliderMultiplier = 100;

	static constexpr int SkipFramesAmount = 5;

	static constexpr int FrameSliderRangeMultiplier = 10;

	Timeline(StudioModelAsset* asset, QWidget* parent = nullptr);

	~Timeline();

private:
	void SetFrame(double value, bool updateEntity);
	void SetFramerate(double value);

	void ModifyFrame(int amount);

private slots:
	void OnTick();

	void OnFrameSliderChanged();
	void OnFrameSpinnerChanged();

	void OnFramerateSliderChanged();
	void OnFramerateSpinnerChanged();

	void OnRestart();

	void OnTogglePlayback();

	void OnPreviousFrame();
	void OnSkipFramesEarlier();

	void OnNextFrame();
	void OnSkipFramesLater();

private:
	Ui_Timeline _ui;

	StudioModelAsset* const _asset;
};
}
