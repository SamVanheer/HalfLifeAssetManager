#pragma once

#include <cassert>
#include <memory>
#include <vector>

#include "formats/studiomodel/StudioModelFileFormat.hpp"

namespace studiomdl
{
struct StudioDataDeleter
{
	void operator()(studiohdr_t* pointer) const
	{
		delete[] pointer;
	}

	void operator()(studioseqhdr_t* pointer) const
	{
		delete[] pointer;
	}
};

template<typename T>
using studio_ptr = std::unique_ptr<T, StudioDataDeleter>;

/**
*	Container representing a studiomodel and its data.
*/
class StudioModel final
{
public:
	StudioModel(studio_ptr<studiohdr_t>&& studioHeader, studio_ptr<studiohdr_t>&& textureHeader,
		std::vector<studio_ptr<studioseqhdr_t>>&& sequenceHeaders, bool isDol)
		: _studioHeader(std::move(studioHeader))
		, _textureHeader(std::move(textureHeader))
		, _sequenceHeaders(std::move(sequenceHeaders))
		, _isDol(isDol)
	{
		assert(_studioHeader);
	}

	~StudioModel() = default;

	StudioModel(const StudioModel&) = delete;
	StudioModel& operator=(const StudioModel&) = delete;

	studiohdr_t* GetStudioHeader() const { return _studioHeader.get(); }

	bool HasSeparateTextureHeader() const { return !!_textureHeader; }

	studiohdr_t* GetTextureHeader() const
	{
		if (_textureHeader)
		{
			return _textureHeader.get();
		}

		return _studioHeader.get();
	}

	studioseqhdr_t* GetSeqGroupHeader(const size_t i) const { return _sequenceHeaders[i].get(); }

	mstudioanim_t* GetAnim(const mstudioseqdesc_t* pseqdesc) const
	{
		mstudioseqgroup_t* pseqgroup = _studioHeader->GetSequenceGroup(pseqdesc->seqgroup);

		if (pseqdesc->seqgroup == 0)
		{
			return (mstudioanim_t*)((std::byte*)_studioHeader.get() + pseqgroup->unused2 + pseqdesc->animindex);
		}

		return (mstudioanim_t*)((std::byte*)_sequenceHeaders[pseqdesc->seqgroup - 1].get() + pseqdesc->animindex);
	}

	bool IsDol() const { return _isDol; }

private:
	studio_ptr<studiohdr_t> _studioHeader;
	studio_ptr<studiohdr_t> _textureHeader;

	std::vector<studio_ptr<studioseqhdr_t>> _sequenceHeaders;

	bool _isDol;
};
}
