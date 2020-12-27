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
				"\"@ApplicationsDirX86@/Half-Life Asset Manager/bin/HLAM.exe\" \"%1\"",
				"Half-Life studiomodel file",
				"application/octet-stream");
		}
	}
}
