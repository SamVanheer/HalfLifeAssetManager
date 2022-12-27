#pragma once

#include <QWidget>

#include "ui_Timeline.h"

class StateSnapshot;

namespace studiomodel
{
class StudioModelAsset;
class StudioModelAssetProvider;

class Timeline final : public QWidget
{
public:
	static constexpr int SpeedMinimum = 0;
	static constexpr int SpeedMaximum = 10;
	static constexpr int SpeedDefault = 1;
	static constexpr int SpeedSliderMultiplier = 100;

	static constexpr int SkipFramesAmount = 5;

	static constexpr int FrameSliderRangeMultiplier = 10;

	Timeline(StudioModelAssetProvider* provider, QWidget* parent = nullptr);

	~Timeline();

	void SetAsset(StudioModelAsset* asset);

private:
	void InitializeUI();

	void SetFrame(double value, bool updateEntity);
	void SetFramerate(double value, bool updateEntity);

	void ModifyFrame(int amount);

private slots:
	void OnTick();

	void OnLoadSnapshot(StateSnapshot* snapshot);

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
	StudioModelAssetProvider* const _provider;
	StudioModelAsset* _asset{};
};
}
