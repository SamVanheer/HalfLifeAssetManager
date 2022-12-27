#pragma once

#include <QString>
#include <QWidget>

#include "ui_OptionsPageExternalPrograms.h"

#include "ui/options/OptionsPage.hpp"

class ApplicationSettings;
class EditorContext;

extern const QString ExternalProgramsExeFilter;

class OptionsPageExternalPrograms : public OptionsPage
{
public:
	OptionsPageExternalPrograms(const std::shared_ptr<ApplicationSettings>& applicationSettings);
	~OptionsPageExternalPrograms();

private:
	const std::shared_ptr<ApplicationSettings> _applicationSettings;
};

class OptionsPageExternalProgramsWidget final : public OptionsWidget
{
	Q_OBJECT

public:
	OptionsPageExternalProgramsWidget(EditorContext* editorContext, ApplicationSettings* applicationSettings);
	~OptionsPageExternalProgramsWidget();

	void ApplyChanges() override;

	// TODO: could convert this into a table or something to avoid having to add more slots and stuff.
private slots:
	void OnBrowseCompiler();
	void OnBrowseDecompiler();
	void OnBrowseXashModelViewer();
	void OnBrowseQuake1ModelViewer();
	void OnBrowseSource1ModelViewer();

private:
	Ui_OptionsPageExternalPrograms _ui;

	EditorContext* const _editorContext;

	ApplicationSettings* const _applicationSettings;
};
