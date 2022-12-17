#pragma once

class QApplication;
class QSettings;

namespace ui::assets
{
class IAssetProviderRegistry;
}

namespace ui::options
{
class OptionsPageRegistry;
}

namespace ui::settings
{
class ColorSettings;
}

/**
*	@brief Allows plugins to add to application systems during startup.
*/
class ApplicationBuilder final
{
public:
	QApplication* const Application;
	QSettings* const Settings;
	// TODO: shouldn't pass specific settings objects around here
	ui::settings::ColorSettings* const ColorSettings;
	ui::assets::IAssetProviderRegistry* const AssetProviderRegistry;
	ui::options::OptionsPageRegistry* const OptionsPageRegistry;
};
