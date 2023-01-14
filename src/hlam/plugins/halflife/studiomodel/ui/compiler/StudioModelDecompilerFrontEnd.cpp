#include "plugins/halflife/studiomodel/ui/compiler/StudioModelDecompilerFrontEnd.hpp"
#include "settings/ApplicationSettings.hpp"
#include "settings/ExternalProgramSettings.hpp"
#include "application/AssetManager.hpp"
#include "ui/options/OptionsPageExternalPrograms.hpp"

namespace studiomodel
{
StudioModelDecompilerFrontEnd::StudioModelDecompilerFrontEnd(AssetManager* application)
	: CommandLineFrontEnd(application)
{
	SetProgram(
		_application->GetApplicationSettings()->GetExternalPrograms()->GetProgram(StudiomdlDecompilerFileNameKey),
		ExternalProgramsExeFilter);
	SetInputFileFilter("MDL Files (*.mdl);;All Files (*.*)");
}

StudioModelDecompilerFrontEnd::~StudioModelDecompilerFrontEnd()
{
	//Sync any changes made to settings
	_application->GetApplicationSettings()->GetExternalPrograms()->SetProgram(
		StudiomdlDecompilerFileNameKey, GetProgram());

	_application->GetApplicationSettings()->SaveSettings();
}
}
