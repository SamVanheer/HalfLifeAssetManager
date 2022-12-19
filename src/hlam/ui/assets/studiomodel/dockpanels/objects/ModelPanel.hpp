#pragma once

#include <QBoxLayout>
#include <QWidget>

#include "ui_ModelPanel.h"

class StateSnapshot;

namespace studiomodel
{
class StudioModelAssetProvider;

class ModelPanel final : public QWidget
{
public:
	explicit ModelPanel(StudioModelAssetProvider* provider);
	~ModelPanel() = default;

public slots:
	void OnLayoutDirectionChanged(QBoxLayout::Direction direction);

private slots:
	void OnOriginChanged();

	void OnCenterOnWorldOrigin();
	void OnAlignOnGround();

private:
	Ui_ModelPanel _ui;
	StudioModelAssetProvider* const _provider;
};
}
