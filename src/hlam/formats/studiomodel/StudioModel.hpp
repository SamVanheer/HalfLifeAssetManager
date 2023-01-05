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
struct StudioPtr
{
	constexpr StudioPtr() noexcept = default;

	explicit constexpr StudioPtr(T* data, std::size_t sizeInBytes) noexcept
		: Header(data)
		, SizeInBytes(sizeInBytes)
	{
	}

	std::unique_ptr<T, StudioDataDeleter> Header;
	std::size_t SizeInBytes{};

	constexpr T* get() const noexcept
	{
		return Header.get();
	}

	constexpr operator bool() const noexcept
	{
		return !!Header;
	}

	constexpr T* operator->() const noexcept
	{
		return Header.get();
	}
};

/**
*	Container representing a studiomodel and its data.
*/
class StudioModel final
{
public:
	StudioModel(StudioPtr<studiohdr_t>&& studioHeader, StudioPtr<studiohdr_t>&& textureHeader,
		std::vector<StudioPtr<studioseqhdr_t>>&& sequenceHeaders, bool isDol)
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
		return GetTextureHeaderPtr().get();
	}

	studioseqhdr_t* GetSeqGroupHeader(const size_t i) const { return _sequenceHeaders[i].get(); }

	const StudioPtr<studiohdr_t>& GetStudioHeaderPtr() const { return _studioHeader; }

	const StudioPtr<studiohdr_t>& GetTextureHeaderPtr() const
	{
		if (_textureHeader)
		{
			return _textureHeader;
		}

		return _studioHeader;
	}

	const StudioPtr<studioseqhdr_t>& GetSeqGroupHeaderPtr(const size_t i) const { return _sequenceHeaders[i]; }

	size_t GetSeqGroupCount() const { return _sequenceHeaders.size(); }

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
	StudioPtr<studiohdr_t> _studioHeader;
	StudioPtr<studiohdr_t> _textureHeader;

	std::vector<StudioPtr<studioseqhdr_t>> _sequenceHeaders;

	bool _isDol;
};
}
