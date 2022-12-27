#pragma once

#include "plugins/IAssetManagerPlugin.hpp"

/**
*	@brief Plugin that adds an asset providers for Source 1 Studio models.
*/
class Source1AssetManagerPlugin final : public IAssetManagerPlugin
{
public:
	QString GetName() const override { return "Source1Plugin"; }

	bool Initialize(ApplicationBuilder& builder) override;

	void Shutdown() override {}

	void LoadSettings(QSettings& settings) override {}
	void SaveSettings(QSettings& settings) override {}
};