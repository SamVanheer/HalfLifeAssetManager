#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>

#include "core/shared/Platform.hpp"
#include "core/shared/Logging.hpp"

#include "utility/IOUtils.hpp"
#include "utility/StringUtils.hpp"

#include "graphics/GraphicsUtils.hpp"
#include "graphics/Palette.hpp"
#include "graphics/TextureLoader.hpp"

#include "engine/shared/studiomodel/StudioModel.hpp"

namespace studiomdl
{
StudioModel::StudioModel(std::string&& fileName, studio_ptr<studiohdr_t>&& studioHeader, studio_ptr<studiohdr_t>&& textureHeader,
	std::vector<studio_ptr<studioseqhdr_t>>&& sequenceHeaders, bool isDol)
	: _fileName(std::move(fileName))
	, _studioHeader(std::move(studioHeader))
	, _textureHeader(std::move(textureHeader))
	, _sequenceHeaders(std::move(sequenceHeaders))
	, _isDol(isDol)
{
	assert(_studioHeader);
}

StudioModel::~StudioModel()
{
}

mstudioanim_t* StudioModel::GetAnim(const mstudioseqdesc_t* pseqdesc) const
{
	mstudioseqgroup_t* pseqgroup = _studioHeader->GetSequenceGroup(pseqdesc->seqgroup);

	if (pseqdesc->seqgroup == 0)
	{
		return (mstudioanim_t*)((byte*)_studioHeader.get() + pseqgroup->unused2 + pseqdesc->animindex);
	}

	return (mstudioanim_t*)((byte*)_sequenceHeaders[pseqdesc->seqgroup - 1].get() + pseqdesc->animindex);
}
}