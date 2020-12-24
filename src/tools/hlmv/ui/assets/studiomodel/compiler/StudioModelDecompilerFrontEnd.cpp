#include "ui/EditorContext.hpp"
#include "ui/options/OptionsPageStudioModel.hpp"
#include "ui/settings/StudioModelSettings.hpp"
#include "ui/assets/studiomodel/compiler/StudioModelDecompilerFrontEnd.hpp"

namespace ui::assets::studiomodel
{
StudioModelDecompilerFrontEnd::StudioModelDecompilerFrontEnd(EditorContext* editorContext, settings::StudioModelSettings* studioModelSettings, QWidget* parent)
	: CommandLineFrontEnd(editorContext, parent)
	, _studioModelSettings(studioModelSettings)
{
	SetProgram(_studioModelSettings->GetStudiomdlDecompilerFileName(), options::StudioModelExeFilter);
	SetInputFileFilter("MDL Files (*.mdl);;All Files (*.*)");
}

StudioModelDecompilerFrontEnd::~StudioModelDecompilerFrontEnd()
{
	//Sync any changes made to settings
	_studioModelSettings->SetStudiomdlDecompilerFileName(GetProgram());

	_studioModelSettings->SaveSettings(*_editorContext->GetSettings());
}
}
