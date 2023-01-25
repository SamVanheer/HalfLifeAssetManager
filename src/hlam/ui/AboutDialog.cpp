#include <QApplication>
#include <QDebug>
#include <QMessageBox>
#include <QString>

#include "ProjectInfo.hpp"

#include "application/AssetManager.hpp"

// This file is rebuilt whenever the ProjectInfo.hpp header is regenerated so keep it as lightweight as possible.

static QString GetVersionInfo()
{
	return QString{"%1.%2.%3 Beta"}
		.arg(HLAMVersionMajor)
		.arg(HLAMVersionMinor)
		.arg(HLAMVersionPatch);
}

static QString GetBuildConfiguration()
{
#ifdef NDEBUG
	return "Release";
#else
	return "Debug";
#endif
}

void LogAppInfo()
{
	qCCritical(HLAM) << QApplication::applicationName() << GetVersionInfo();
	qCCritical(HLAM) << "Build Configuration:" << GetBuildConfiguration();
	qCCritical(HLAM) << "Git Info:"
		<< "\nBranch:" << QString::fromUtf8(HLAMGitBranchName.data())
		<< "\nTag:" << QString::fromUtf8(HLAMGitTagName.data())
		<< "\nCommit Hash:" << QString::fromUtf8(HLAMGitCommitHash.data());
	qCCritical(HLAM) << "Qt Version:" << QT_VERSION_STR;
	qCCritical(HLAM) << "Build Date:" << __DATE__;
}

void ShowAboutDialog(QWidget* parent)
{
	const QString programName{QApplication::applicationName()};

	QMessageBox::about(parent, "About " + programName,
		QString::fromUtf8(
			reinterpret_cast<const char*>(u8R"(%1 %2

2023 Sam Vanheer

Email:	sam.vanheer@outlook.com

Build Configuration: %3
Git Info:
	Branch: %4
	Tag: %5
	Commit Hash: %6

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

Uses Qt %7

Build Date: %8
)"))
			.arg(programName)
			.arg(GetVersionInfo())
			.arg(GetBuildConfiguration())
			.arg(QString::fromUtf8(HLAMGitBranchName.data()))
			.arg(QString::fromUtf8(HLAMGitTagName.data()))
			.arg(QString::fromUtf8(HLAMGitCommitHash.data()))
			.arg(QT_VERSION_STR)
			.arg(__DATE__)
);
}
