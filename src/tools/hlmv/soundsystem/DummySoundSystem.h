#pragma once

#include "soundsystem/ISoundSystem.h"

namespace soundsystem
{
/**
*	@brief Dummy sound system used when no audio device is available
*/
class DummySoundSystem final : public ISoundSystem
{
public:
	bool Initialize(filesystem::IFileSystem*) override { return true; }

	void Shutdown() {}

	void RunFrame() {}

	void PlaySound(const char*, float, int) {}

	void StopAllSounds() override {}
};
}
