#pragma once

#include <QSettings>
#include <QString>
#include <QWidget>

#include "ui_OptionsPageStudioModel.h"

#include "ui/options/OptionsPage.hpp"

namespace ui
{
class EditorContext;

namespace settings
{
class StudioModelSettings;
}

namespace options
{
extern const QString OptionsPageStudiomodelId;
extern const QString StudioModelExeFilter;

class OptionsPageStudioModel : public OptionsPage
{
public:
	OptionsPageStudioModel(const std::shared_ptr<settings::StudioModelSettings>& studioModelSettings);
	~OptionsPageStudioModel();

private:
	const std::shared_ptr<settings::StudioModelSettings> _studioModelSettings;
};

class OptionsPageStudioModelWidget final : public OptionsWidget
{
	Q_OBJECT

public:
	OptionsPageStudioModelWidget(EditorContext* editorContext, settings::StudioModelSettings* studioModelSettings);
	~OptionsPageStudioModelWidget();

	void ApplyChanges(QSettings& settings) override;

private slots:
	void OnResetFloorLength();

	void OnBrowseCompiler();
	void OnBrowseDecompiler();

private:
	Ui_OptionsPageStudioModel _ui;

	EditorContext* const _editorContext;

	settings::StudioModelSettings* const _studioModelSettings;
};
}
}
