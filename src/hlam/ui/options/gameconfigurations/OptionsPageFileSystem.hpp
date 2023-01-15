#pragma once

#include <QString>

#include "ui_OptionsPageFileSystem.h"

#include "ui/options/OptionsPage.hpp"

class AssetManager;

extern const QString OptionsPageGameConfigurationsCategory;

class OptionsPageFileSystem final : public OptionsPage
{
public:
	explicit OptionsPageFileSystem();
};

class OptionsPageFileSystemWidget final : public OptionsWidget
{
public:
	explicit OptionsPageFileSystemWidget(AssetManager* application);
	~OptionsPageFileSystemWidget();

	void ApplyChanges() override;

private:
	Ui_OptionsPageFileSystem _ui;

	AssetManager* const _application;
};
