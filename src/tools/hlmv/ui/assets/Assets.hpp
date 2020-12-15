#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <QObject>
#include <QString>
#include <QUndoStack>
#include <QWidget>

#include <entt/core/type_info.hpp>

class QMenu;

namespace ui
{
class EditorContext;
class FullscreenWidget;

namespace assets
{
class IAssetProvider;

class Asset : public QObject
{
	Q_OBJECT

protected:
	Asset(QString&& fileName)
		: _fileName(std::move(fileName))
	{
	}

public:
	virtual ~Asset() = 0 {}

	virtual entt::id_type GetAssetType() const = 0;

	virtual const IAssetProvider* GetProvider() const = 0;

	QString GetFileName() const { return _fileName; }

	void SetFileName(QString&& fileName)
	{
		if (_fileName != fileName)
		{
			_fileName = std::move(fileName);

			emit FileNameChanged(fileName);
		}
	}

	QUndoStack* GetUndoStack() const { return _undoStack; }

	/**
	*	@brief Populates the Asset menu with actions and submenus specific to this asset
	*/
	virtual void PopulateAssetMenu(QMenu* menu) = 0;

	/**
	*	@brief Gets a widget to view and edit this asset
	*/
	virtual QWidget* GetEditWidget() = 0;

	virtual void SetupFullscreenWidget(EditorContext* editorContext, FullscreenWidget* fullscreenWidget) = 0;

	virtual void Save(const QString& fileName) = 0;

	void AddUndoCommand(QUndoCommand* command)
	{
		_undoStack->push(command);
		emit AssetChanged(command);
	}

signals:
	void FileNameChanged(const QString& fileName);

	/**
	*	@brief Emitted for every undo command that is added
	*/
	void AssetChanged(QUndoCommand* command);

protected:
	QString _fileName;
	QUndoStack* const _undoStack = new QUndoStack(this);
};

/**
*	@brief Provides a means of loading and saving assets
*/
class IAssetProvider
{
public:
	virtual ~IAssetProvider() = 0 {}

	virtual entt::id_type GetAssetType() const = 0;

	virtual bool CanLoad(const QString& fileName) const = 0;

	//TODO: pass a filesystem object to resolve additional file locations with
	virtual std::unique_ptr<Asset> Load(EditorContext* editorContext, const QString& fileName) const = 0;

	virtual void Save(const QString& fileName, Asset& asset) const = 0;
};

/**
*	@brief Stores the list of asset providers
*/
class IAssetProviderRegistry
{
public:
	virtual ~IAssetProviderRegistry() = 0 {}

	virtual void AddProvider(std::unique_ptr<IAssetProvider>&& provider) = 0;

	virtual std::unique_ptr<Asset> Load(EditorContext* editorContext, const QString& fileName) const = 0;
};

class AssetProviderRegistry final : public IAssetProviderRegistry
{
public:
	AssetProviderRegistry() = default;
	~AssetProviderRegistry() = default;
	AssetProviderRegistry(const AssetProviderRegistry&) = delete;
	AssetProviderRegistry& operator=(const AssetProviderRegistry&) = delete;

	void AddProvider(std::unique_ptr<IAssetProvider>&& provider) override;

	std::unique_ptr<Asset> Load(EditorContext* editorContext, const QString& fileName) const override;

private:
	std::unordered_map<entt::id_type, std::unique_ptr<IAssetProvider>> _providers;
};
}
}
