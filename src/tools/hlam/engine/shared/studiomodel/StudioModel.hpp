#pragma once

#include <array>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <glm/vec3.hpp>

#include "core/shared/Const.hpp"

#include "utility/mathlib.hpp"
#include "utility/Color.hpp"

#include "graphics/OpenGL.hpp"

#include "engine/shared/studiomodel/StudioModelFileFormat.hpp"

namespace graphics
{
class TextureLoader;
}

namespace studiomdl
{
//TODO: refactor to use data structures defined by new editable model format
constexpr std::array<std::array<double, 2>, SequenceBlendCount> CounterStrikeBlendRanges{{{-180, 180}, {-45, 45}}};

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
	StudioModel(std::string&& fileName, studio_ptr<studiohdr_t>&& studioHeader, studio_ptr<studiohdr_t>&& textureHeader,
		std::vector<studio_ptr<studioseqhdr_t>>&& sequenceHeaders, bool isDol);
	~StudioModel();

	StudioModel(const StudioModel&) = delete;
	StudioModel& operator=(const StudioModel&) = delete;

	const std::string& GetFileName() const { return _fileName; }

	void SetFileName(std::string&& fileName)
	{
		_fileName = std::move(fileName);
	}

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

	mstudioanim_t* GetAnim(const mstudioseqdesc_t* pseqdesc) const;

	bool IsDol() const { return _isDol; }

private:
	std::string _fileName;

	studio_ptr<studiohdr_t> _studioHeader;
	studio_ptr<studiohdr_t> _textureHeader;

	std::vector<studio_ptr<studioseqhdr_t>> _sequenceHeaders;

	bool _isDol;
};

/**
*	Returns the string representation for a studio control value.
*	@param iControl Value containing a STUDIO_* control bit.
*	@return String representation for the control, or nullptr.
*/
const char* ControlToString(const int iControl);

/**
*	Returns the description for a studio control value.
*	@param iControl Value containing a STUDIO_* control bit.
*	@return String description for the control, or nullptr.
*/
const char* ControlToStringDescription(const int iControl);

/**
*	Converts a string to a control value.
*	@param pszString String representation of a control value.
*	@return Control value, or -1 if the string does not represent a valid control.
*/
int StringToControl(const char* const pszString);
}
