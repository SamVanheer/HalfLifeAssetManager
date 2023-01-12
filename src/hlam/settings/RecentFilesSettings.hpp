#pragma once

#include <QObject>
#include <QSettings>
#include <QString>
#include <QStringList>

/**
*	@brief Stores the list of recent files, from newest (0) to oldest (GetCount() - 1)
*/
class RecentFilesSettings final : public QObject
{
	Q_OBJECT

public:
	static constexpr int DefaultMaxRecentFiles{10};

	explicit RecentFilesSettings(QSettings* settings)
		: _settings(settings)
	{
	}

	~RecentFilesSettings() = default;

	void LoadSettings()
	{
		_recentFiles.clear();

		_settings->beginGroup("RecentFiles");

		_maxRecentFiles = std::max(0, _settings->value("MaxRecentFiles", DefaultMaxRecentFiles).toInt());

		const int fileCount = _settings->beginReadArray("List");

		for (int i = 0; i < fileCount; ++i)
		{
			_settings->setArrayIndex(i);
			_recentFiles.append(_settings->value("FileName").toString());
		}

		_settings->endArray();

		_settings->endGroup();

		CheckMaximum();

		emit RecentFilesChanged();
	}

	void SaveSettings()
	{
		_settings->beginGroup("RecentFiles");

		_settings->setValue("MaxRecentFiles", _maxRecentFiles);

		_settings->remove("List");

		_settings->beginWriteArray("List", _recentFiles.size());

		for (int i = 0; i < _recentFiles.size(); ++i)
		{
			_settings->setArrayIndex(i);
			_settings->setValue("FileName", _recentFiles[i]);
		}

		_settings->endArray();

		_settings->endGroup();
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
	QSettings* const _settings;

	QStringList _recentFiles;

	int _maxRecentFiles{DefaultMaxRecentFiles};
};
