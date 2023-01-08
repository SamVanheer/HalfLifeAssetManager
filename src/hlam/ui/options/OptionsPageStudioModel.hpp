#pragma once

#include <QString>
#include <QWidget>

#include "ui_OptionsPageStudioModel.h"

#include "ui/options/OptionsPage.hpp"

class AssetManager;
class StudioModelSettings;

class OptionsPageStudioModel : public OptionsPage
{
public:
	OptionsPageStudioModel(const std::shared_ptr<StudioModelSettings>& studioModelSettings);
	~OptionsPageStudioModel();

private:
	const std::shared_ptr<StudioModelSettings> _studioModelSettings;
};

class OptionsPageStudioModelWidget final : public OptionsWidget
{
	Q_OBJECT

public:
	OptionsPageStudioModelWidget(AssetManager* application, StudioModelSettings* studioModelSettings);
	~OptionsPageStudioModelWidget();

	void ApplyChanges() override;

	int FindFirstFreeEventId(const int startId) const;

private slots:
	void OnResetFloorLength();

private:
	Ui_OptionsPageStudioModel _ui;

	AssetManager* const _application;

	StudioModelSettings* const _studioModelSettings;
};
