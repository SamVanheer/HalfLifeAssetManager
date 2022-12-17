#pragma once

#include "plugins/IAssetManagerPlugin.hpp"

/**
*	@brief Plugin that adds asset providers for Half-Life 1 formats and options pages to go with them.
*/
class HalfLifeAssetManagerPlugin final : public IAssetManagerPlugin
{
public:
	QString GetName() const override { return "HalfLifePlugin"; }

	bool Initialize(ApplicationBuilder& builder) override;

	void Shutdown() override;

	void LoadSettings(QSettings& settings) override;
	void SaveSettings(QSettings& settings) override;
};