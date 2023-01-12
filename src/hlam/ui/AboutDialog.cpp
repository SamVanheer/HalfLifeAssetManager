#include <QApplication>
#include <QMessageBox>
#include <QString>

#include "ProjectInfo.hpp"

// This file is rebuilt whenever the ProjectInfo.hpp header is regenerated so keep it as lightweight as possible.

void ShowAboutDialog(QWidget* parent)
{
	const QString programName{QApplication::applicationName()};

	QString buildConfiguration;

#ifdef NDEBUG
	buildConfiguration = "Release";
#else
	buildConfiguration = "Debug";
#endif

	QMessageBox::about(parent, "About " + programName,
		QString::fromUtf8(
			reinterpret_cast<const char*>(u8R"(%1 %2.%3.%4 Beta

2023 Sam Vanheer

Email:	sam.vanheer@outlook.com

Build Configuration: %5
Git Info:
	Branch: %6
	Tag: %7
	Commit Hash: %8

Based on Jed's Half-Life Model Viewer v1.3 © 2004 Neil 'Jed' Jedrzejewski
Email:	jed@wunderboy.org
Web:	http://www.wunderboy.org/

Also based on Half-Life Model Viewer v1.25 © 2002 Mete Ciragan
Email:	mete@swissquake.ch
Web:	http://www.milkshape3d.com/

This product contains software technology licensed from Id Software, Inc.
( "Id Technology" ). Id Technology © 1996 Id Software, Inc.
All Rights Reserved.

Copyright © 1998-2002, Valve LLC.
All rights reserved.

Uses OpenAL Soft
Uses Ogg Vorbis
Uses Libnyquist, Copyright (c) 2019, Dimitri Diakopoulos All rights reserved.
Uses The OpenGL Mathemathics library (GLM)
Copyright © 2005 - 2016 G-Truc Creation

Uses Qt %9

Build Date: %10
)"))
			.arg(programName)
			.arg(HLAMVersionMajor)
			.arg(HLAMVersionMinor)
			.arg(HLAMVersionPatch)
			.arg(buildConfiguration)
			.arg(QString::fromUtf8(HLAMGitBranchName.data()))
			.arg(QString::fromUtf8(HLAMGitTagName.data()))
			.arg(QString::fromUtf8(HLAMGitCommitHash.data()))
			.arg(QT_VERSION_STR)
			.arg(__DATE__)
);
}
