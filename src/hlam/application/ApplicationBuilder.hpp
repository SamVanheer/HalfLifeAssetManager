#pragma once

class QApplication;
class QSettings;

class IAssetProviderRegistry;

class OptionsPageRegistry;

class ColorSettings;

/**
*	@brief Allows plugins to add to application systems during startup.
*/
class ApplicationBuilder final
{
public:
	QApplication* const Application;
	QSettings* const Settings;
	// TODO: shouldn't pass specific settings objects around here
	ColorSettings* const ColorSettings;
	IAssetProviderRegistry* const AssetProviderRegistry;
	OptionsPageRegistry* const OptionsPageRegistry;
};
