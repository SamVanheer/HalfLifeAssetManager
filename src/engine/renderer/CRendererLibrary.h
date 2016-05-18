#ifndef ENGINE_RENDERER_CRENDERERLIBRARY_H
#define ENGINE_RENDERER_CRENDERERLIBRARY_H

#include "shared/renderer/IRendererLibrary.h"

namespace renderer
{
class CRendererLibrary final : public IRendererLibrary
{
public:
	CRendererLibrary() = default;

	bool Connect( const CreateInterfaceFn* const pFactories, const size_t uiNumFactories ) override final;

	void Disconnect() override final;

private:
	CRendererLibrary( const CRendererLibrary& ) = delete;
	CRendererLibrary& operator=( const CRendererLibrary& ) = delete;
};
}

#endif //ENGINE_RENDERER_CRENDERERLIBRARY_H