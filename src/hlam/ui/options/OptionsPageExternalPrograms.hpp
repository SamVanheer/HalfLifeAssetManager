#pragma once

#include <QSettings>
#include <QString>
#include <QWidget>

#include "ui_OptionsPageExternalPrograms.h"

#include "ui/options/OptionsPage.hpp"

class EditorContext;

class GeneralSettings;

extern const QString ExternalProgramsExeFilter;

class OptionsPageExternalPrograms : public OptionsPage
{
public:
	OptionsPageExternalPrograms(const std::shared_ptr<GeneralSettings>& generalSettings);
	~OptionsPageExternalPrograms();

private:
	const std::shared_ptr<GeneralSettings> _generalSettings;
};

class OptionsPageExternalProgramsWidget final : public OptionsWidget
{
	Q_OBJECT

public:
	OptionsPageExternalProgramsWidget(EditorContext* editorContext, GeneralSettings* generalSettings);
	~OptionsPageExternalProgramsWidget();

	void ApplyChanges(QSettings& settings) override;

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

	GeneralSettings* const _generalSettings;
};
