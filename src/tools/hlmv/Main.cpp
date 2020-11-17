#include "application/HLMVApplication.hpp"

//TODO: temporarily declare globals here
#include "engine/shared/renderer/sprite/ISpriteRenderer.h"
#include "engine/shared/renderer/studiomodel/IStudioModelRenderer.h"
#include "soundsystem/ISoundSystem.h"

sprite::ISpriteRenderer* g_pSpriteRenderer = nullptr;
studiomdl::IStudioModelRenderer* g_pStudioMdlRenderer = nullptr;
soundsystem::ISoundSystem* g_pSoundSystem = nullptr;

int main(int argc, char* argv[])
{
	HLMVApplication app;

	return app.Run(argc, argv);
}
