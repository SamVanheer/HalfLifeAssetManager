#pragma once

#include <QSettings>
#include <QString>
#include <QWidget>

#include "ui_OptionsPageStudioModel.h"

#include "ui/options/OptionsPage.hpp"

class EditorContext;

class StudioModelSettings;

extern const QString OptionsPageStudiomodelId;
extern const QString StudioModelExeFilter;

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

	void ApplyChanges(QSettings& settings) override;

private slots:
	void OnResetFloorLength();

	void OnBrowseCompiler();
	void OnBrowseDecompiler();
	void OnBrowseXashModelViewer();

private:
	Ui_OptionsPageStudioModel _ui;

	EditorContext* const _editorContext;

	StudioModelSettings* const _studioModelSettings;
};
