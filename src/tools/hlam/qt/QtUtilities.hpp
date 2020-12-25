#pragma once

#include <QString>

namespace qt
{
bool LaunchDefaultProgram(const QString& fileName);

QString GetImagesFileFilter();

QString GetSeparatedImagesFileFilter();
}
