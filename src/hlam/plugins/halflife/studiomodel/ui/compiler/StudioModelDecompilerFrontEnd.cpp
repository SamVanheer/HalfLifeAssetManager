#include "plugins/halflife/studiomodel/ui/compiler/StudioModelDecompilerFrontEnd.hpp"
#include "settings/ApplicationSettings.hpp"
#include "application/EditorContext.hpp"
#include "ui/options/OptionsPageExternalPrograms.hpp"

namespace studiomodel
{
StudioModelDecompilerFrontEnd::StudioModelDecompilerFrontEnd(EditorContext* editorContext)
	: CommandLineFrontEnd(editorContext)
{
	SetProgram(
		_editorContext->GetApplicationSettings()->GetExternalPrograms()->GetProgram(StudiomdlDecompilerFileNameKey),
		ExternalProgramsExeFilter);
	SetInputFileFilter("MDL Files (*.mdl);;All Files (*.*)");
}

StudioModelDecompilerFrontEnd::~StudioModelDecompilerFrontEnd()
{
	//Sync any changes made to settings
	_editorContext->GetApplicationSettings()->GetExternalPrograms()->SetProgram(
		StudiomdlDecompilerFileNameKey, GetProgram());

	_editorContext->GetApplicationSettings()->SaveSettings();
}
}
