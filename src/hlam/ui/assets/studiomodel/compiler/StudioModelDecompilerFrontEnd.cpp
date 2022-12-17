#include "ui/EditorContext.hpp"
#include "ui/options/OptionsPageStudioModel.hpp"
#include "ui/settings/StudioModelSettings.hpp"
#include "ui/assets/studiomodel/compiler/StudioModelDecompilerFrontEnd.hpp"

namespace studiomodel
{
StudioModelDecompilerFrontEnd::StudioModelDecompilerFrontEnd(EditorContext* editorContext, StudioModelSettings* studioModelSettings)
	: CommandLineFrontEnd(editorContext)
	, _studioModelSettings(studioModelSettings)
{
	SetProgram(_studioModelSettings->GetStudiomdlDecompilerFileName(), StudioModelExeFilter);
	SetInputFileFilter("MDL Files (*.mdl);;All Files (*.*)");
}

StudioModelDecompilerFrontEnd::~StudioModelDecompilerFrontEnd()
{
	//Sync any changes made to settings
	_studioModelSettings->SetStudiomdlDecompilerFileName(GetProgram());

	_studioModelSettings->SaveSettings(*_editorContext->GetSettings());
}
}
