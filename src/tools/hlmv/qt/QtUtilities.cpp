#include <QDesktopServices>
#include <QFileInfo>
#include <QMessageBox>
#include <QUrl>

#include "qt/QtUtilities.hpp"

namespace qt
{
bool LaunchDefaultProgram(const QString& fileName)
{
	if (!QDesktopServices::openUrl(QUrl::fromLocalFile(fileName)))
	{
		QMessageBox::critical(nullptr, "Error",
			QString{"Unable to start default program\nMake sure the %1 extension is associated with a program"}.arg(QFileInfo(fileName).completeSuffix()));

		return false;
	}

	return true;
}
}
