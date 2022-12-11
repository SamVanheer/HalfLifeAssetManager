#pragma once

#include "soundsystem/ISoundSystem.hpp"

namespace soundsystem
{
class DummySoundSystem final : public ISoundSystem
{
public:
	bool IsSoundAvailable() const override { return false; }

	bool Initialize(filesystem::IFileSystem*) override { return true; }

	void Shutdown() override {}

	void RunFrame() override {}

	void PlaySound(std::string_view, float, int) override {}

	void StopAllSounds() override {}
};
}
