#pragma once

#include <memory>

#include <QDialog>

class Ui_QCDataDialog;

namespace studiomodel
{
class StudioModelAssetProvider;

class QCDataDialog final : public QDialog
{
public:
	QCDataDialog(StudioModelAssetProvider* provider, QWidget* parent);
	~QCDataDialog();

private:
	std::unique_ptr<Ui_QCDataDialog> _ui;
	StudioModelAssetProvider* const _provider;
	StudioModelAsset* const _asset;
};
}
