#pragma once

#include <string_view>

#undef PlaySound

/**
*	@defgroup SoundSystem OpenAL based sound system.
*
*	@{
*/

/**
*	A sound system that can be used to play back sounds. Sounds are non-looping.
*/
class ISoundSystem
{
public:
	virtual ~ISoundSystem() {}

	/**
	*	@brief Whether sound is available (i.e. is a device available)
	*/
	virtual bool IsSoundAvailable() const = 0;

	virtual bool Initialize() = 0;

	virtual void Shutdown() = 0;

	virtual void SetMuted(bool muted) = 0;

	virtual void RunFrame() = 0;

	/**
	*	@brief Plays a sound by name.
	*	The filename is relative to the game's sound directory, and is looked up using the filesystem.
	*	@param fileName Sound filename.
	*	@param volume Volume. Expressed as a range between [0, 1].
	*	@param pitch Pitch amount. Expressed as a range between [0, 255].
	*/
	virtual void PlaySound(std::string_view fileName, float volume, int pitch) = 0;

	virtual void StopAllSounds() = 0;
};

/** @} */
