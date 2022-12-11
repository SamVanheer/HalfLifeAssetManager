#pragma once

#include <QSettings>
#include <QString>

namespace ui::settings
{
inline const QString PathsGroupName{QStringLiteral("paths")};

inline QString GetSavedPath(QSettings& settings, const QString& pathName)
{
	settings.beginGroup(PathsGroupName);
	const QString path{settings.value(pathName).toString()};
	settings.endGroup();

	return path;
}

inline void SetSavedPath(QSettings& settings, const QString& pathName, const QString& path)
{
	settings.beginGroup(PathsGroupName);
	settings.setValue(pathName, path);
	settings.endGroup();
}
}
