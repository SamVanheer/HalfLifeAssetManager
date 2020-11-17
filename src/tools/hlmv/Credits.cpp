#include <QObject>

#include "Credits.hpp"

namespace tools
{
std::string GetSharedCredits()
{
	return std::string( 
		u8"This product contains software technology licensed from Id Software, Inc.\n"
		u8"( \"Id Technology\" ). Id Technology © 1996 Id Software, Inc.\n"
		u8"All Rights Reserved.\n\n"
		u8"Copyright © 1998-2002, Valve LLC.\n"
		u8"All rights reserved.\n\n"
		u8"Contains OpenAL Soft\n"
		u8"Contains Ogg Vorbis\n"
		u8"Contains AudioFile, Copyright (c) 2017 Adam Stark\n"
		u8"Contains The OpenGL Extension Wrangler Library (GLEW)\n"
		u8"Copyright © 2008 - 2016, Nigel Stewart <nigels[]users sourceforge net>\n"
		u8"Copyright © 2002 - 2008, Milan Ikits <milan ikits[]ieee org>\n"
		u8"Copyright © 2002 - 2008, Marcelo E.Magallon <mmagallo[]debian org>\n"
		u8"Copyright © 2002, Lev Povalahev\n"
		u8"All rights reserved.\n\n"
		u8"Contains The OpenGL Mathemathics library (GLM)\n"
		u8"Copyright © 2005 - 2016 G-Truc Creation\n\n"
		u8"Uses Qt ") + QT_VERSION_STR + u8"\n\n"
		+ u8"Build Date: " + + __DATE__ + u8"\n";
}
}