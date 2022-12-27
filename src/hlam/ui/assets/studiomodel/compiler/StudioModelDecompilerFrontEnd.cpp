#include "settings/ApplicationSettings.hpp"
#include "ui/EditorContext.hpp"
#include "ui/options/OptionsPageExternalPrograms.hpp"
#include "ui/assets/studiomodel/compiler/StudioModelDecompilerFrontEnd.hpp"

namespace studiomodel
{
StudioModelDecompilerFrontEnd::StudioModelDecompilerFrontEnd(EditorContext* editorContext)
	: CommandLineFrontEnd(editorContext)
{
	SetProgram(_editorContext->GetApplicationSettings()->GetStudiomdlDecompilerFileName(), ExternalProgramsExeFilter);
	SetInputFileFilter("MDL Files (*.mdl);;All Files (*.*)");
}

StudioModelDecompilerFrontEnd::~StudioModelDecompilerFrontEnd()
{
	//Sync any changes made to settings
	_editorContext->GetApplicationSettings()->SetStudiomdlDecompilerFileName(GetProgram());

	_editorContext->GetApplicationSettings()->SaveSettings(*_editorContext->GetSettings());
}
}
