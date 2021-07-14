#pragma once

#include <memory>
#include <string>
#include <vector>

#include <QObject>
#include <QString>
#include <QUndoStack>
#include <QWidget>

class QMenu;

namespace ui
{
class EditorContext;
class FullscreenWidget;

namespace assets
{
class AssetProvider;

class Asset : public QObject
{
	Q_OBJECT

protected:
	Asset(QString&& fileName)
		: _fileName(std::move(fileName))
	{
	}

public:
	virtual ~Asset() {}

	virtual const AssetProvider* GetProvider() const = 0;

	QString GetFileName() const { return _fileName; }

	void SetFileName(QString&& fileName)
	{
		if (_fileName != fileName)
		{
			_fileName = std::move(fileName);

			emit FileNameChanged(_fileName);
		}
	}

	QUndoStack* GetUndoStack() const { return _undoStack; }

	bool IsActive() const { return _isActive; }

	void SetActive(bool value)
	{
		if (_isActive != value)
		{
			_isActive = value;
			emit IsActiveChanged(_isActive);
		}
	}

	/**
	*	@brief Populates the Asset menu with actions and submenus specific to this asset
	*/
	virtual void PopulateAssetMenu(QMenu* menu) = 0;

	/**
	*	@brief Gets a widget to view and edit this asset
	*/
	virtual QWidget* GetEditWidget() = 0;

	virtual void SetupFullscreenWidget(FullscreenWidget* fullscreenWidget) = 0;

	virtual void Save() = 0;

signals:
	void FileNameChanged(const QString& fileName);

	void IsActiveChanged(bool value);

private:
	QString _fileName;
	QUndoStack* const _undoStack = new QUndoStack(this);
	bool _isActive{false};
};

/**
*	@brief Provides a means of loading and saving assets
*/
class AssetProvider : public QObject
{
public:
	/**
	*	@brief e.g. "Studiomodel". Used for file filters.
	*/
	virtual QString GetProviderName() const = 0;

	virtual QStringList GetFileTypes() const = 0;

	virtual QString GetPreferredFileType() const = 0;

	/**
	*	@brief Creates the tool menu for this asset, or nullptr if no menu is available
	*/
	virtual QMenu* CreateToolMenu(EditorContext* editorContext) = 0;

	virtual bool CanLoad(const QString& fileName) const = 0;

	//TODO: pass a filesystem object to resolve additional file locations with
	virtual std::unique_ptr<Asset> Load(EditorContext* editorContext, const QString& fileName) const = 0;
};

/**
*	@brief Stores the list of asset providers
*/
class IAssetProviderRegistry
{
public:
	virtual ~IAssetProviderRegistry() {}

	virtual std::vector<AssetProvider*> GetAssetProviders() const = 0;

	virtual void AddProvider(std::unique_ptr<AssetProvider>&& provider) = 0;

	virtual std::unique_ptr<Asset> Load(EditorContext* editorContext, const QString& fileName) const = 0;
};

class AssetProviderRegistry final : public IAssetProviderRegistry
{
public:
	AssetProviderRegistry() = default;
	~AssetProviderRegistry() = default;
	AssetProviderRegistry(const AssetProviderRegistry&) = delete;
	AssetProviderRegistry& operator=(const AssetProviderRegistry&) = delete;

	std::vector<AssetProvider*> GetAssetProviders() const override;

	void AddProvider(std::unique_ptr<AssetProvider>&& provider) override;

	std::unique_ptr<Asset> Load(EditorContext* editorContext, const QString& fileName) const override;

private:
	std::vector<std::unique_ptr<AssetProvider>> _providers;
};
}
}

Q_DECLARE_METATYPE(ui::assets::Asset*)
