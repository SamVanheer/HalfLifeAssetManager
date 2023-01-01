#pragma once

#include <memory>

#include <QDialog>

class Ui_ModelInfoDialog;

namespace studiomodel
{
class StudioModelAssetProvider;

class ModelInfoDialog final : public QDialog
{
public:
	explicit ModelInfoDialog(StudioModelAssetProvider* provider, QWidget* parent);
	~ModelInfoDialog();

private:
	std::unique_ptr<Ui_ModelInfoDialog> _ui;
	StudioModelAssetProvider* const _provider;
	StudioModelAsset* const _asset;
};
}
