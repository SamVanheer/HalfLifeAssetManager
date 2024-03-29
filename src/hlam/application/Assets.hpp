#pragma once

#include <cstdio>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include <QFlags>
#include <QObject>
#include <QString>
#include <QUndoStack>
#include <QWidget>

class AssetProvider;
class AssetManager;
class QMenu;

enum class ProviderFeature
{
	None = 0,
	AssetLoading = 1 << 0,
	AssetSaving = 1 << 1
};

Q_DECLARE_FLAGS(ProviderFeatures, ProviderFeature)
Q_DECLARE_OPERATORS_FOR_FLAGS(ProviderFeatures)

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

	virtual AssetProvider* GetProvider() const = 0;

	QString GetFileName() const { return _fileName; }

	void SetFileName(QString&& fileName)
	{
		if (_fileName != fileName)
		{
			_fileName = std::move(fileName);
			emit FileNameChanged(this);
		}
	}

	QUndoStack* GetUndoStack() const { return _undoStack; }

	/**
	*	@brief Gets a widget to view and edit this asset
	*/
	virtual QWidget* GetEditWidget() = 0;

	virtual void Save() = 0;

	virtual bool TryRefresh() = 0;

	virtual bool CanTakeScreenshot() const = 0;

	virtual void TakeScreenshot() = 0;

signals:
	void EditWidgetChanged();

	void FileNameChanged(Asset* asset);

private:
	QString _fileName;
	QUndoStack* const _undoStack = new QUndoStack(this);
	bool _isActive{false};
};

struct AssetLoadInExternalProgram
{
	QString ExternalProgramKey;
	bool PromptBeforeOpening = true;
};

using AssetLoadData = std::variant<std::unique_ptr<Asset>, AssetLoadInExternalProgram>;

/**
*	@brief Provides a means of loading and saving assets
*/
class AssetProvider : public QObject
{
	Q_OBJECT

protected:
	explicit AssetProvider(AssetManager* application)
		: _application(application)
	{
	}

public:
	/**
	*	@brief e.g. "StudioModel". Used for file filters.
	*/
	virtual QString GetProviderName() const = 0;

	virtual QStringList GetFileTypes() const = 0;

	virtual QString GetPreferredFileType() const = 0;

	virtual ProviderFeatures GetFeatures() const = 0;

	virtual void Initialize() {}

	virtual void Shutdown() {}

	/**
	*	@brief Called every frame while an asset loaded through this provider is active.
	*/
	virtual void Tick() {}

	/**
	*	@brief Creates the tool menu for this asset, or nullptr if no menu is available.
	*/
	virtual QMenu* CreateToolMenu() { return nullptr; }

	/**
	*	@brief Populates the Asset menu with actions and submenus, or nullptr if no menu is available.
	*	The provider is responsible for forwarding actions to the current asset.
	*/
	virtual void PopulateAssetMenu(QMenu* menu) {}

	virtual bool CanLoad(const QString& fileName, FILE* file) const = 0;

	//TODO: pass a filesystem object to resolve additional file locations with
	virtual AssetLoadData Load(const QString& fileName, FILE* file) = 0;

	/**
	*	@brief Returns whether the given file is a candidate for loading when a file list is presented to the user.
	*/
	virtual bool IsCandidateForLoading(const QString& fileName, FILE* file) const
	{
		return CanLoad(fileName, file);
	}

protected:
	AssetManager* const _application;
};

/**
*	@brief Stores the list of asset providers
*/
class AssetProviderRegistry final
{
public:
	AssetProviderRegistry() = default;
	~AssetProviderRegistry() = default;
	AssetProviderRegistry(const AssetProviderRegistry&) = delete;
	AssetProviderRegistry& operator=(const AssetProviderRegistry&) = delete;

	std::vector<AssetProvider*> GetAssetProviders() const;

	void AddProvider(std::unique_ptr<AssetProvider> provider);

	void Initialize();
	void Shutdown();

	std::variant<std::unique_ptr<Asset>, AssetLoadInExternalProgram> Load(const QString& fileName) const;

private:
	std::vector<std::unique_ptr<AssetProvider>> _providers;
};

Q_DECLARE_METATYPE(Asset*)
Q_DECLARE_METATYPE(AssetProvider*)
