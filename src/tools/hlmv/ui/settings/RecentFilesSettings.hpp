#pragma once

#include <QObject>
#include <QSettings>
#include <QString>
#include <QStringList>

namespace ui::settings
{
/**
*	@brief Stores the list of recent files, from newest (0) to oldest (GetCount() - 1)
*/
class RecentFilesSettings final : public QObject
{
	Q_OBJECT

public:
	static constexpr int DefaultMaxRecentFiles{10};

	RecentFilesSettings(QObject* parent = nullptr)
		: QObject(parent)
	{
	}

	~RecentFilesSettings() = default;

	void LoadSettings(QSettings& settings)
	{
		_recentFiles.clear();

		settings.beginGroup("recent_files");

		_maxRecentFiles = std::max(0, settings.value("MaxRecentFiles", DefaultMaxRecentFiles).toInt());

		const int fileCount = settings.beginReadArray("list");

		for (int i = 0; i < fileCount; ++i)
		{
			settings.setArrayIndex(i);
			_recentFiles.append(settings.value("FileName").toString());
		}

		settings.endArray();

		settings.endGroup();

		CheckMaximum();

		emit RecentFilesChanged();
	}

	void SaveSettings(QSettings& settings)
	{
		settings.beginGroup("recent_files");

		settings.setValue("MaxRecentFiles", _maxRecentFiles);

		settings.remove("List");

		settings.beginWriteArray("List", _recentFiles.size());

		for (int i = 0; i < _recentFiles.size(); ++i)
		{
			settings.setArrayIndex(i);
			settings.setValue("FileName", _recentFiles[i]);
		}

		settings.endArray();

		settings.endGroup();
	}

	int GetCount() const { return _recentFiles.count(); }

	QString At(int index) const { return _recentFiles.at(index); }

	void Add(const QString& fileName)
	{
		if (const int existingIndex = _recentFiles.indexOf(fileName); existingIndex != -1)
		{
			_recentFiles.move(existingIndex, 0);
		}
		else
		{
			_recentFiles.prepend(fileName);
			CheckMaximum();
		}

		emit RecentFilesChanged();
	}

	void Remove(const QString& fileName)
	{
		_recentFiles.removeOne(fileName);
		emit RecentFilesChanged();
	}

	int GetMaxRecentFiles() const { return _maxRecentFiles; }

	void SetMaxRecentFiles(int value)
	{
		_maxRecentFiles = value;

		if (CheckMaximum())
		{
			emit RecentFilesChanged();
		}
	}

private:
	bool CheckMaximum()
	{
		if (_maxRecentFiles > 0 && _recentFiles.size() > _maxRecentFiles)
		{
			_recentFiles.erase(_recentFiles.begin() + _maxRecentFiles, _recentFiles.end());
			return true;
		}

		return false;
	}

signals:
	void RecentFilesChanged();

private:
	QStringList _recentFiles;

	int _maxRecentFiles{DefaultMaxRecentFiles};
};
}
