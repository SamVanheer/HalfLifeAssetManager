#include "Credits.h"

namespace tools
{
std::string GetSharedCredits()
{
	//TODO: need to add Qt license
	return std::string( 
		"This product contains software technology licensed from Id Software, Inc.\n"
		"( \"Id Technology\" ). Id Technology © 1996 Id Software, Inc.\n"
		"All Rights Reserved.\n\n"
		"Copyright © 1998-2002, Valve LLC.\n"
		"All rights reserved.\n\n"
		"Contains OpenAL Soft\n"
		"Contains Ogg Vorbis\n"
		"Contains AudioFile, Copyright (c) 2017 Adam Stark\n"
		"Contains The OpenGL Extension Wrangler Library (GLEW)\n"
		"Copyright © 2008 - 2016, Nigel Stewart <nigels[]users sourceforge net>\n"
		"Copyright © 2002 - 2008, Milan Ikits <milan ikits[]ieee org>\n"
		"Copyright © 2002 - 2008, Marcelo E.Magallon <mmagallo[]debian org>\n"
		"Copyright © 2002, Lev Povalahev\n"
		"All rights reserved.\n\n"
		"Contains The OpenGL Mathemathics library (GLM)\n"
		"Copyright © 2005 - 2016 G-Truc Creation\n\n"
		"Build Date: ") + __DATE__ + "\n";
}
}