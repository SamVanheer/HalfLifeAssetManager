#include "ui/EditorContext.hpp"
#include "ui/options/OptionsPageExternalPrograms.hpp"
#include "ui/assets/studiomodel/compiler/StudioModelDecompilerFrontEnd.hpp"
#include "ui/settings/GeneralSettings.hpp"

namespace studiomodel
{
StudioModelDecompilerFrontEnd::StudioModelDecompilerFrontEnd(EditorContext* editorContext)
	: CommandLineFrontEnd(editorContext)
{
	SetProgram(_editorContext->GetGeneralSettings()->GetStudiomdlDecompilerFileName(), ExternalProgramsExeFilter);
	SetInputFileFilter("MDL Files (*.mdl);;All Files (*.*)");
}

StudioModelDecompilerFrontEnd::~StudioModelDecompilerFrontEnd()
{
	//Sync any changes made to settings
	_editorContext->GetGeneralSettings()->SetStudiomdlDecompilerFileName(GetProgram());

	_editorContext->GetGeneralSettings()->SaveSettings(*_editorContext->GetSettings());
}
}
