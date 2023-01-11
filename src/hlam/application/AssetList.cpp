#include <algorithm>
#include <cassert>

#include <QFileInfo>
#include <QMessageBox>
#include <QString>

#include "application/AssetIO.hpp"
#include "application/AssetList.hpp"
#include "application/AssetManager.hpp"
#include "application/Assets.hpp"

#include "qt/QtLogging.hpp"

#include "settings/ApplicationSettings.hpp"
#include "settings/RecentFilesSettings.hpp"

#include "ui/MainWindow.hpp"

AssetList::AssetList(AssetManager* application, std::shared_ptr<spdlog::logger> logger)
	: _application(application)
	, _logger(logger)
{
}

AssetList::~AssetList() = default;

int AssetList::IndexOf(const Asset* asset) const
{
	if (asset)
	{
		if (auto it = std::find_if(_assets.begin(), _assets.end(),
			[&](const auto& candidate)
			{
				return candidate.get() == asset;
			});
			it != _assets.end())
		{
			return it - _assets.begin();
		}
	}

	return -1;
}

void AssetList::SetCurrent(Asset* asset)
{
	if (asset)
	{
		assert(IndexOf(asset) != -1);
	}

	if (_currentAsset == asset)
	{
		return;
	}

	_currentAsset = asset;

	emit ActiveAssetChanged(_currentAsset);
}

AssetLoadResult AssetList::TryLoad(const QString& fileName)
{
	const AssetLoadResult result = TryLoadCore(fileName);

	switch (result)
	{
	case AssetLoadResult::Success:
		_application->GetApplicationSettings()->GetRecentFiles()->Add(fileName);
		break;

	case AssetLoadResult::Failed:
		_application->GetApplicationSettings()->GetRecentFiles()->Remove(fileName);
		break;
	}

	return result;
}

AssetLoadResult AssetList::TryLoadCore(QString fileName)
{
	fileName = fileName.trimmed();

	if (fileName.isEmpty())
	{
		_logger->error("Asset filename is empty");
		return AssetLoadResult::Failed;
	}

	const QFileInfo fileInfo{fileName};

	fileName = fileInfo.absoluteFilePath();

	_logger->trace("Trying to load asset \"{}\"", fileName);

	if (!fileInfo.exists())
	{
		_logger->error("Asset \"{}\" does not exist", fileName);
		return AssetLoadResult::Failed;
	}

	// First check if it's already loaded.
	for (int i = 0; i < Count(); ++i)
	{
		auto asset = Get(i);

		if (asset->GetFileName() == fileName)
		{
			SetCurrent(asset);
			const bool result = RefreshCurrent();

			return result ? AssetLoadResult::Success : AssetLoadResult::Cancelled;
		}
	}

	if (_application->GetApplicationSettings()->OneAssetAtATime)
	{
		if (!TryClose(0, true))
		{
			//User canceled, abort load
			return AssetLoadResult::Cancelled;
		}
	}

	try
	{
		auto asset = _application->GetAssetProviderRegistry()->Load(fileName);

		return std::visit([&, this](auto&& result)
			{
				using T = std::decay_t<decltype(result)>;

				if constexpr (std::is_same_v<T, std::unique_ptr<Asset>>)
				{
					if (!result)
					{
						_logger->debug("Asset \"{}\" couldn't be loaded", fileName);
						return AssetLoadResult::Failed;
					}

					connect(result.get(), &Asset::FileNameChanged, this, &AssetList::OnAssetFileNameChanged);

					_assets.push_back(std::move(result));

					emit AssetAdded(_assets.size() - 1);

					_logger->trace("Loaded asset \"{}\"", fileName);
				}
				else if constexpr (std::is_same_v<T, AssetLoadInExternalProgram>)
				{
					// Nothing.
				}
				else
				{
					static_assert(always_false_v<T>, "Unhandled Asset load return type");
				}

				return AssetLoadResult::Success;
			}, asset);
	}
	catch (const AssetException& e)
	{
		_logger->error("Error loading asset \"{}\":\n{}", fileName, e.what());
	}

	return AssetLoadResult::Failed;
}

bool AssetList::TryClose(int index, bool verifyUnsavedChanges, bool allowCancel)
{
	assert(index != -1);

	emit AboutToCloseAsset(index);

	{
		if (verifyUnsavedChanges && !VerifyNoUnsavedChanges(Get(index), allowCancel))
		{
			//User cancelled or an error occurred
			return false;
		}

		const TimerSuspender timerSuspender{_application};

		emit AboutToRemoveAsset(index);

		// Don't destroy the asset until after we've cleaned everything up.
		const std::unique_ptr<Asset> asset = std::move(_assets[index]);

		_assets.erase(_assets.begin() + index);

		emit AssetRemoved(index);
	}

	return true;
}

bool AssetList::Save(Asset* asset)
{
	assert(asset);

	_logger->trace("Trying to save asset \"{}\"", asset->GetFileName());

	try
	{
		asset->Save();
	}
	catch (const AssetException& e)
	{
		_logger->error("Error saving asset:\n{}", e.what());
		return false;
	}

	auto undoStack = asset->GetUndoStack();

	undoStack->setClean();

	return true;
}

bool AssetList::VerifyNoUnsavedChanges(Asset* asset, bool allowCancel)
{
	assert(asset);

	if (asset->GetUndoStack()->isClean())
	{
		return true;
	}

	QMessageBox::StandardButtons buttons = QMessageBox::StandardButton::Save | QMessageBox::StandardButton::Discard;

	if (allowCancel)
	{
		buttons |= QMessageBox::StandardButton::Cancel;
	}

	const QMessageBox::StandardButton action = QMessageBox::question(
		_application->GetMainWindow(),
		{},
		QString{"Save changes made to \"%1\"?"}.arg(asset->GetFileName()),
		buttons,
		QMessageBox::StandardButton::Save);

	switch (action)
	{
	case QMessageBox::StandardButton::Save: return Save(asset);
	case QMessageBox::StandardButton::Discard: return true;
	default:
	case QMessageBox::StandardButton::Cancel: return false;
	}
}

bool AssetList::RefreshCurrent()
{
	if (auto asset = GetCurrent(); asset)
	{
		if (!VerifyNoUnsavedChanges(asset, true))
		{
			//User canceled, abort refresh
			return false;
		}

		return asset->TryRefresh();
	}

	return false;
}

void AssetList::OnAssetFileNameChanged(Asset* asset)
{
	_application->GetApplicationSettings()->GetRecentFiles()->Add(asset->GetFileName());
}
