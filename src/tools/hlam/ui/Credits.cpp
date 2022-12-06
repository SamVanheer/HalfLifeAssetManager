#include <QObject>

#include "ui/Credits.hpp"

namespace ui
{
std::string GetSharedCredits()
{
	return std::string( 
		"This product contains software technology licensed from Id Software, Inc.\n"
		"( \"Id Technology\" ). Id Technology © 1996 Id Software, Inc.\n"
		"All Rights Reserved.\n\n"
		"Copyright © 1998-2002, Valve LLC.\n"
		"All rights reserved.\n\n"
		"Contains OpenAL Soft\n"
		"Contains Ogg Vorbis\n"
		"Contains AudioFile, Copyright (c) 2017 Adam Stark\n"
		"Contains The OpenGL Mathemathics library (GLM)\n"
		"Copyright © 2005 - 2016 G-Truc Creation\n\n"
		"Uses Qt ") + QT_VERSION_STR + "\n\n"
		+ "Build Date: " + + __DATE__ + "\n";
}
}