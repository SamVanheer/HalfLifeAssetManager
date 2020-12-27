function Component()
{
	component.loaded.connect(this, AddRegisterFileTypes);
}

AddRegisterFileTypes = function()
{
	// don't show when updating or uninstalling
	if (installer.isInstaller())
	{
		installer.addWizardPageItem(component, "RegisterFileTypesForm", QInstaller.TargetDirectory);
    }
}

Component.prototype.createOperations = function()
{
	component.createOperations();

	if (component.userInterface("RegisterFileTypesForm"))
	{
		var isRegisterMDLChecked = component.userInterface("RegisterFileTypesForm").RegisterMDLExtension.checked;
    }
	
	if (installer.value("os") === "win")
	{
		if (isRegisterMDLChecked)
		{
			component.addOperation("RegisterFileType",
				".mdl",
				"\"@TargetDir@/bin/HLAM.exe\" \"%1\"",
				"Half-Life studiomodel file",
				"application/octet-stream");
		}
		
		// Exit code 1638 is returned when the redist is already installed
		component.addOperation("Execute", "{0,1638}", "@TargetDir@/redist/VC_redist.x86.exe", "/quiet", "/norestart");
	}
}
