#include "application/HLMVApplication.hpp"

//TODO: temporarily declare globals here
#include "engine/shared/renderer/sprite/ISpriteRenderer.hpp"

sprite::ISpriteRenderer* g_pSpriteRenderer = nullptr;

int main(int argc, char* argv[])
{
	HLMVApplication app;

	return app.Run(argc, argv);
}
