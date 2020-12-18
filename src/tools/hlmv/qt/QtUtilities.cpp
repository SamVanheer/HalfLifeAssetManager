#include <QDesktopServices>
#include <QFileInfo>
#include <QImageWriter>
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

QString GetImagesFileFilter()
{
	static QString cachedFilter;

	if (cachedFilter.isEmpty())
	{
		const auto formats = QImageWriter::supportedImageFormats();

		QStringList formatsStrings;

		formatsStrings.reserve(formats.size());

		for (const auto& format : formats)
		{
			formatsStrings.append(QString{"*.%1"}.arg(QString{format}));
		}

		//TODO: also have one filter per format
		cachedFilter = QString{"Image Files (%1);;All Files (*.*)"}.arg(formatsStrings.join(' '));
	}

	return cachedFilter;
}
}
