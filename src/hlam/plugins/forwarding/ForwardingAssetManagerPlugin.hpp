#pragma once

#include "plugins/IAssetManagerPlugin.hpp"

/**
*	@brief Plugin that adds asset providers for formats that are recognized and forwarded to other programs.
*/
class ForwardingAssetManagerPlugin final : public IAssetManagerPlugin
{
public:
	QString GetName() const override { return "ForwardingPlugin"; }

	bool Initialize(AssetManager* application) override;

	void Shutdown() override {}
};