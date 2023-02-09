#include <memory>

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
	const QFileInfo info(fileName);

	if (!info.exists(fileName))
	{
		QMessageBox::critical(nullptr, "Error",
			QString{R"(Unable to start default program

The file "%1" does not exist)"}
		.arg(info.absoluteFilePath()));

		return false;
	}

	if (!QDesktopServices::openUrl(QUrl::fromLocalFile(fileName)))
	{
		QMessageBox::critical(nullptr, "Error",
			QString{R"(Unable to start default program

Make sure the %1 extension is associated with a program)"}
				.arg(info.completeSuffix()));

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
			formatsStrings.append(QString{"*.%1"}.arg(QString{format}.toLower()));
		}

		cachedFilter = QString{"Image Files (%1);;All Files (*.*)"}.arg(formatsStrings.join(' '));
	}

	return cachedFilter;
}

QString GetSeparatedImagesFileFilter()
{
	static QString cachedFilter;

	if (cachedFilter.isEmpty())
	{
		const auto formats = QImageWriter::supportedImageFormats();

		QStringList formatsStrings;

		formatsStrings.reserve(formats.size());

		for (const auto& format : formats)
		{
			const QString formatString{format};

			formatsStrings.append(QString{"%1 Files (*.%2)"}.arg(formatString.toUpper()).arg(formatString.toLower()));
		}

		cachedFilter = QString{"%1;;All Files (*.*)"}.arg(formatsStrings.join(";;"));
	}

	return cachedFilter;
}
}
