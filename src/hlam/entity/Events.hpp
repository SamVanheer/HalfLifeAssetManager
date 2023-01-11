#pragma once

/*
*	Events used by the game.
*/
enum Event
{
	EVENT_SPECIFIC = 0,
	EVENT_SCRIPTED = 1000,
	EVENT_SHARED = 2000,
	EVENT_CLIENT = 5000,
	
	SCRIPT_EVENT_SOUND = 1004,				// Play named wave file (on CHAN_BODY)
	SCRIPT_EVENT_SOUND_VOICE = 1008,		// Play named wave file (on CHAN_VOICE)
	SCRIPT_EVENT_SOUND_VOICE_BODY = 1011,	// Play named wave file with normal attenuation (on CHAN_BODY)
	SCRIPT_EVENT_SOUND_VOICE_VOICE = 1012,	// Play named wave file with normal attenuation (on CHAN_VOICE)
	SCRIPT_EVENT_SOUND_VOICE_WEAPON = 1013,	// Play named wave file with normal attenuation (on CHAN_WEAPON)
	SCRIPT_CLIENT_EVENT_SOUND = 5004,		// Play named wave file (at a given location)
};

constexpr bool IsSoundEvent(int id)
{
	switch (id)
	{
	case SCRIPT_EVENT_SOUND:
		[[fallthrough]];
	case SCRIPT_EVENT_SOUND_VOICE:
		[[fallthrough]];
	case SCRIPT_EVENT_SOUND_VOICE_BODY:
		[[fallthrough]];
	case SCRIPT_EVENT_SOUND_VOICE_VOICE:
		[[fallthrough]];
	case SCRIPT_EVENT_SOUND_VOICE_WEAPON:
		[[fallthrough]];
	case SCRIPT_CLIENT_EVENT_SOUND:
		return true;
	}

	return false;
}
