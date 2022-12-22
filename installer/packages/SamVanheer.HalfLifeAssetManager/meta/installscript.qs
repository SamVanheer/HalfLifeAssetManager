function Component()
{
	var validOs = false;

    if (systemInfo.kernelType === "winnt")
	{
		var windowsMajorVersion = MajorVersion(systemInfo.kernelVersion);
		var windowsMinorVersion = MinorVersion(systemInfo.kernelVersion);
		
		// Windows 7 and newer is supported (6.1 and newer)
        if (windowsMajorVersion > 6 || (windowsMajorVersion == 6 && windowsMinorVersion >= 1))
		{
            validOs = true;
		}
	}
	
	if (!validOs)
	{
        CancelInstaller("Installation on " + systemInfo.prettyProductName + " is not supported");
        return;
    }
	
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

function CancelInstaller(message)
{
    installer.setDefaultPageVisible(QInstaller.Introduction, false);
    installer.setDefaultPageVisible(QInstaller.TargetDirectory, false);
    installer.setDefaultPageVisible(QInstaller.ComponentSelection, false);
    installer.setDefaultPageVisible(QInstaller.ReadyForInstallation, false);
    installer.setDefaultPageVisible(QInstaller.StartMenuSelection, false);
    installer.setDefaultPageVisible(QInstaller.PerformInstallation, false);
    installer.setDefaultPageVisible(QInstaller.LicenseCheck, false);

    var abortText = "<font color='red'>" + message +"</font>";
    installer.setValue("FinishedText", abortText);
}

function MajorVersion(str)
{
    return parseInt(str.split(".", 1));
}

function MinorVersion(str)
{
    return parseInt(str.split(".", 2).slice(1));
}

Component.prototype.createOperations = function()
{
	component.createOperations();

	if (component.userInterface("RegisterFileTypesForm"))
	{
		var isRegisterMDLChecked = component.userInterface("RegisterFileTypesForm").RegisterMDLExtension.checked;
    }
	
	if (systemInfo.productType === "windows")
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
		//3010 means reboot required, but it's unlikely to actually be necessary
		//5100 means newer version installed
		component.addOperation("Execute", "{0,1638,3010,5100}", "@TargetDir@/redist/VC_redist.x86.exe", "/quiet", "/norestart");
	}
}
