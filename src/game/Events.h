#ifndef GAME_EVENTS_H
#define GAME_EVENTS_H

/*
*	Events used by the game.
*/
#define EVENT_SPECIFIC			0
#define EVENT_SCRIPTED			1000
#define EVENT_SHARED			2000
#define EVENT_CLIENT			5000

#define SCRIPT_EVENT_SOUND			1004		// Play named wave file (on CHAN_BODY)
#define SCRIPT_EVENT_SOUND_VOICE	1008		// Play named wave file (on CHAN_VOICE)
#define SCRIPT_CLIENT_EVENT_SOUND	5004		// Play named wave file (at a given location)

#endif //GAME_EVENTS_H