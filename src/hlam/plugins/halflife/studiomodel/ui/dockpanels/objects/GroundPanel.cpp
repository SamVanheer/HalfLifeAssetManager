#include <QFileDialog>
#include <QSignalBlocker>

#include "entity/GroundEntity.hpp"

#include "qt/QtUtilities.hpp"

#include "plugins/halflife/studiomodel/StudioModelAsset.hpp"
#include "plugins/halflife/studiomodel/ui/dockpanels/objects/GroundPanel.hpp"

#include "application/AssetManager.hpp"

namespace studiomodel
{
GroundPanel::GroundPanel(StudioModelAssetProvider* provider)
	: _provider(provider)
{
	_ui.setupUi(this);

	connect(_provider, &StudioModelAssetProvider::AssetChanged, this, &GroundPanel::OnAssetChanged);

	connect(_ui.ShowGround, &QCheckBox::stateChanged, this, &GroundPanel::OnShowGroundChanged);
	connect(_ui.MirrorModelOnGround, &QCheckBox::stateChanged, this, &GroundPanel::OnMirrorOnGroundChanged);
	connect(_ui.EnableTexture, &QCheckBox::toggled, this, &GroundPanel::OnEnableTextureChanged);
	connect(_ui.EnableGroundTextureTiling, &QGroupBox::toggled, this, &GroundPanel::OnEnableGroundTextureTilingChanged);
	connect(_ui.GroundTextureSize, qOverload<int>(&QSpinBox::valueChanged), this, &GroundPanel::OnGroundTextureSizeChanged);

	connect(_ui.GroundTexture, &QLineEdit::textChanged, this, &GroundPanel::OnTextureChanged);
	connect(_ui.BrowseGroundTexture, &QPushButton::clicked, this, &GroundPanel::OnBrowseTexture);

	connect(_ui.GroundOrigin, &qt::widgets::SimpleVector3Edit::ValueChanged, this, &GroundPanel::OnOriginChanged);
}

void GroundPanel::OnAssetChanged(StudioModelAsset* asset)
{
	const QSignalBlocker showGroundBlocker{_ui.ShowGround};
	const QSignalBlocker mirrorBlocker{_ui.MirrorModelOnGround};
	const QSignalBlocker enableTextureBlocker{_ui.EnableTexture};
	const QSignalBlocker tilingBlocker{_ui.EnableGroundTextureTiling};
	const QSignalBlocker textureSizeBlocker{_ui.GroundTextureSize};
	const QSignalBlocker groundTextureBlocker{_ui.GroundTexture};
	const QSignalBlocker originBlocker{_ui.GroundOrigin};

	auto entity = asset->GetGroundEntity();

	_ui.ShowGround->setChecked(entity->ShowGround);
	_ui.MirrorModelOnGround->setChecked(entity->MirrorOnGround);
	_ui.EnableTexture->setChecked(entity->EnableTexture);
	_ui.EnableGroundTextureTiling->setChecked(entity->EnableGroundTextureTiling);
	_ui.GroundTextureSize->setValue(entity->GroundTextureLength);
	_ui.GroundTexture->setText(QString::fromStdString(entity->GetImageName()));
	_ui.GroundOrigin->SetValue(entity->GetOrigin());
}

void GroundPanel::OnShowGroundChanged()
{
	_provider->GetCurrentAsset()->GetGroundEntity()->ShowGround = _ui.ShowGround->isChecked();

	if (!_provider->GetCurrentAsset()->GetGroundEntity()->ShowGround)
	{
		_ui.MirrorModelOnGround->setChecked(false);
	}
}

void GroundPanel::OnMirrorOnGroundChanged()
{
	_provider->GetCurrentAsset()->GetGroundEntity()->MirrorOnGround = _ui.MirrorModelOnGround->isChecked();

	if (_provider->GetCurrentAsset()->GetGroundEntity()->MirrorOnGround)
	{
		_ui.ShowGround->setChecked(true);
	}
}

void GroundPanel::OnEnableTextureChanged()
{
	_provider->GetCurrentAsset()->GetGroundEntity()->EnableTexture = _ui.EnableTexture->isChecked();

	if (_provider->GetCurrentAsset()->GetGroundEntity()->EnableTexture)
	{
		_ui.ShowGround->setChecked(true);
	}
}

void GroundPanel::OnEnableGroundTextureTilingChanged()
{
	_provider->GetCurrentAsset()->GetGroundEntity()->EnableGroundTextureTiling = _ui.EnableGroundTextureTiling->isChecked();
}

void GroundPanel::OnGroundTextureSizeChanged()
{
	_provider->GetCurrentAsset()->GetGroundEntity()->GroundTextureLength = _ui.GroundTextureSize->value();
}

void GroundPanel::OnTextureChanged()
{
	bool hasTexture = false;

	if (const QString fileName = _ui.GroundTexture->text(); !fileName.isEmpty())
	{
		if (QImage image{fileName}; !image.isNull())
		{
			image.convertTo(QImage::Format::Format_RGBA8888);

			_provider->GetCurrentAsset()->GetGroundEntity()->SetImage(fileName.toStdString(),
				{image.width(), image.height(), reinterpret_cast<const std::byte*>(image.constBits())});

			_ui.ShowGround->setChecked(true);
			_ui.EnableTexture->setChecked(true);
			hasTexture = true;
		}
	}

	if (!hasTexture)
	{
		_provider->GetCurrentAsset()->GetGroundEntity()->ClearImage();
	}
}

void GroundPanel::OnBrowseTexture()
{
	const QString PathKey{"GroundPath"};

	const QString fileName{QFileDialog::getOpenFileName(nullptr, {},
		_provider->GetCurrentAsset()->GetApplication()->GetPath(PathKey),
		qt::GetImagesFileFilter())};

	if (!fileName.isEmpty())
	{
		_ui.GroundTexture->setText(fileName);
		_provider->GetCurrentAsset()->GetApplication()->SetPath(PathKey, fileName);
	}
}

void GroundPanel::OnOriginChanged()
{
	_provider->GetCurrentAsset()->GetGroundEntity()->SetOrigin(_ui.GroundOrigin->GetValue());
}
}
