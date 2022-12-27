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
	QApplication* const Application;
	ApplicationSettings* const ApplicationSettings;
	AssetProviderRegistry* const AssetProviderRegistry;
	OptionsPageRegistry* const OptionsPageRegistry;
};
