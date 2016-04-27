#ifndef UI_UTILITY_CMESHCLIENTDATA_H
#define UI_UTILITY_CMESHCLIENTDATA_H

#include "ui/wxInclude.h"

#include "shared/studiomodel/studio.h"

namespace ui
{
/**
*	Client data class for StudioModel mesh data.
*/
class CMeshClientData final : public wxClientData
{
public:
	CMeshClientData( const mstudiomesh_t* const pMesh )
		: m_pMesh( pMesh )
	{
	}

	const mstudiomesh_t* const m_pMesh;

private:
	CMeshClientData( const CMeshClientData& ) = delete;
	CMeshClientData& operator=( const CMeshClientData& ) = delete;
};
}

#endif //UI_UTILITY_CMESHCLIENTDATA_H