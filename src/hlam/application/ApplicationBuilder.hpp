#pragma once

class AssetProviderRegistry;
class ApplicationSettings;
class OptionsPageRegistry;
class QApplication;

/**
*	@brief Allows plugins to add to application systems during startup.
*/
class ApplicationBuilder final
{
public:
	QApplication* const GUIApplication;
	ApplicationSettings* const Settings;
	AssetProviderRegistry* const AssetProviders;
	OptionsPageRegistry* const OptionsPages;
};
