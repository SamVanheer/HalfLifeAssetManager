#ifndef GAME_EVENTS_H
#define GAME_EVENTS_H

/*
*	Events used by the game.
*/
enum Event
{
	EVENT_SPECIFIC				= 0,
	EVENT_SCRIPTED				= 1000,
	EVENT_SHARED				= 2000,
	EVENT_CLIENT				= 5000,
	
	SCRIPT_EVENT_SOUND			= 1004,		// Play named wave file (on CHAN_BODY)
	SCRIPT_EVENT_SOUND_VOICE	= 1008,		// Play named wave file (on CHAN_VOICE)
	SCRIPT_CLIENT_EVENT_SOUND	= 5004,		// Play named wave file (at a given location)
};

#endif //GAME_EVENTS_H