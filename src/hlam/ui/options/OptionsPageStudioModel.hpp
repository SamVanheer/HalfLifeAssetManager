#pragma once

#include <QString>
#include <QWidget>

#include "ui_OptionsPageStudioModel.h"

#include "ui/options/OptionsPage.hpp"

class EditorContext;
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
	OptionsPageStudioModelWidget(EditorContext* editorContext, StudioModelSettings* studioModelSettings);
	~OptionsPageStudioModelWidget();

	void ApplyChanges() override;

	int FindFirstFreeEventId(const int startId) const;

private slots:
	void OnResetFloorLength();

private:
	Ui_OptionsPageStudioModel _ui;

	EditorContext* const _editorContext;

	StudioModelSettings* const _studioModelSettings;
};
