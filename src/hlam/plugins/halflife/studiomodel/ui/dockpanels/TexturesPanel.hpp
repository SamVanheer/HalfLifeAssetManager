#pragma once

#include <QString>

#include "ui_TexturesPanel.h"

#include "ui/DockableWidget.hpp"

namespace studiomdl
{
class EditableStudioModel;
}

class StateSnapshot;

namespace studiomodel
{
class StudioModelAsset;
class StudioModelAssetProvider;
class StudioModelData;

const inline QString TexturePathName{QStringLiteral("TexturePath")};

class TexturesPanel final : public DockableWidget
{
	Q_OBJECT

public:
	explicit TexturesPanel(StudioModelAssetProvider* provider);
	~TexturesPanel();

	void OnVisibilityChanged(bool visible) override;

private:
	void SetTextureName(bool updateTextures);
	void ImportTextureFrom(const QString& fileName, studiomdl::EditableStudioModel& model, int textureIndex);
	void UpdateColormapValue();
	void UpdateUVMapProperties();

private slots:
	void OnAssetChanged(StudioModelAsset* asset);

	void OnSaveSnapshot(StateSnapshot* snapshot);
	void OnLoadSnapshot(StateSnapshot* snapshot);

	void OnScaleChanged(float adjust);

	void OnTextureChanged(int index);

	void OnTextureViewScaleSliderChanged(int value);
	void OnTextureViewScaleSpinnerChanged(double value);

	void OnTextureNameChanged();
	void OnTextureNameRejected();

	void OnChromeChanged();
	void OnAdditiveChanged();
	void OnTransparentChanged();
	void OnFlatShadeChanged();
	void OnFullbrightChanged();
	void OnMipmapsChanged();

	void OnOverlayUVMapChanged();

	void OnImportTexture();
	void OnExportTexture();
	void OnExportUVMap();

	void OnImportAllTextures();
	void OnExportAllTextures();

	void OnTopColorSliderChanged();
	void OnBottomColorSliderChanged();
	void OnTopColorSpinnerChanged();
	void OnBottomColorSpinnerChanged();

private:
	Ui_TexturesPanel _ui;
	StudioModelAssetProvider* const _provider;
	StudioModelAsset* _asset{};
	StudioModelData* _previousModelData{};

	bool _updatingUI{false};
};
}
