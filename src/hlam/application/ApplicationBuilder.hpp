#pragma once

class ColorSettings;
class GeneralSettings;
class IAssetProviderRegistry;
class OptionsPageRegistry;
class QApplication;
class QSettings;

/**
*	@brief Allows plugins to add to application systems during startup.
*/
class ApplicationBuilder final
{
public:
	QApplication* const Application;
	QSettings* const Settings;
	// TODO: shouldn't pass specific settings objects around here
	GeneralSettings* const GeneralSettings;
	ColorSettings* const ColorSettings;
	IAssetProviderRegistry* const AssetProviderRegistry;
	OptionsPageRegistry* const OptionsPageRegistry;
};
