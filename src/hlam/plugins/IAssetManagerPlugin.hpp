#pragma once

#include <QString>

class AssetManager;
class QSettings;

/**
*	@brief Interface that plugins use to register asset providers, options pages, etc.
*/
class IAssetManagerPlugin
{
public:
	virtual ~IAssetManagerPlugin() = default;

	virtual QString GetName() const = 0;

	virtual bool Initialize(AssetManager* application) = 0;

	virtual void Shutdown() = 0;

	virtual void LoadSettings(QSettings& settings) = 0;
	virtual void SaveSettings(QSettings& settings) = 0;
};
