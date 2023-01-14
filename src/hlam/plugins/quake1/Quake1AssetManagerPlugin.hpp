#pragma once

#include "plugins/IAssetManagerPlugin.hpp"

/**
*	@brief Plugin that adds an asset providers for Quake 1 Alias models.
*/
class Quake1AssetManagerPlugin final : public IAssetManagerPlugin
{
public:
	QString GetName() const override { return "Quake1Plugin"; }

	bool Initialize(AssetManager* application) override;

	void Shutdown() override {}
};