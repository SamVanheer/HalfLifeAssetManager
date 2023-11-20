#pragma once

#include "soundsystem/ISoundSystem.hpp"

class DummySoundSystem final : public ISoundSystem
{
public:
	bool IsSoundAvailable() const override { return false; }

	bool Initialize() override { return true; }

	void Shutdown() override {}

	void SetMuted(bool muted) override {}

	void RunFrame() override {}

	void PlaySound(std::string_view, float, int) override {}

	void StopAllSounds() override {}
};
