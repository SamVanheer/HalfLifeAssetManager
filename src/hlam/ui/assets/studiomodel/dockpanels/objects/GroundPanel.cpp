#include <limits>

#include <QFileDialog>

#include "entity/GroundEntity.hpp"

#include "qt/QtUtilities.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/dockpanels/objects/GroundPanel.hpp"

#include "ui/settings/StudioModelSettings.hpp"

namespace studiomodel
{
GroundPanel::GroundPanel(StudioModelAssetProvider* provider)
	: _provider(provider)
{
	_ui.setupUi(this);

	_ui.GroundTextureSize->setValue(_provider->GetStudioModelSettings()->FloorTextureLength);

	_ui.GroundOrigin->SetRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
	_ui.GroundOrigin->SetDecimals(6);

	connect(_ui.ShowGround, &QCheckBox::stateChanged, this, &GroundPanel::OnShowGroundChanged);
	connect(_ui.MirrorModelOnGround, &QCheckBox::stateChanged, this, &GroundPanel::OnMirrorOnGroundChanged);
	connect(_ui.EnableGroundTextureTiling, &QGroupBox::toggled, this, &GroundPanel::OnEnableGroundTextureTilingChanged);
	connect(_ui.GroundTextureSize, qOverload<int>(&QSpinBox::valueChanged), this, &GroundPanel::OnGroundTextureSizeChanged);

	connect(_ui.GroundTexture, &QLineEdit::textChanged, this, &GroundPanel::OnTextureChanged);
	connect(_ui.BrowseGroundTexture, &QPushButton::clicked, this, &GroundPanel::OnBrowseTexture);

	connect(_ui.GroundOrigin, &qt::widgets::Vector3Edit::ValueChanged, this, &GroundPanel::OnOriginChanged);
}

void GroundPanel::OnLayoutDirectionChanged(QBoxLayout::Direction direction)
{
	_ui.MainLayout->setDirection(direction);
}

void GroundPanel::OnShowGroundChanged()
{
	_provider->GetStudioModelSettings()->ShowGround = _ui.ShowGround->isChecked();

	if (!_provider->GetStudioModelSettings()->ShowGround)
	{
		_ui.MirrorModelOnGround->setChecked(false);
	}
}

void GroundPanel::OnMirrorOnGroundChanged()
{
	_provider->GetStudioModelSettings()->MirrorOnGround = _ui.MirrorModelOnGround->isChecked();

	if (_provider->GetStudioModelSettings()->MirrorOnGround)
	{
		_ui.ShowGround->setChecked(true);
	}
}

void GroundPanel::OnEnableGroundTextureTilingChanged()
{
	_provider->GetStudioModelSettings()->EnableFloorTextureTiling = _ui.EnableGroundTextureTiling->isChecked();
}

void GroundPanel::OnGroundTextureSizeChanged()
{
	_provider->GetStudioModelSettings()->FloorTextureLength = _ui.GroundTextureSize->value();
}

void GroundPanel::OnTextureChanged()
{
	bool hasTexture = false;

	if (const QString fileName = _ui.GroundTexture->text(); !fileName.isEmpty())
	{
		if (QImage image{fileName}; !image.isNull())
		{
			image.convertTo(QImage::Format::Format_RGBA8888);

			_provider->GetCurrentAsset()->GetGroundEntity()->SetImage(
				{image.width(), image.height(), reinterpret_cast<const std::byte*>(image.constBits())});

			_ui.ShowGround->setChecked(true);
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
	const QString fileName{QFileDialog::getOpenFileName(nullptr, {}, {}, qt::GetImagesFileFilter())};

	if (!fileName.isEmpty())
	{
		_ui.GroundTexture->setText(fileName);
	}
}

void GroundPanel::OnOriginChanged()
{
	_provider->GetStudioModelSettings()->FloorOrigin = _ui.GroundOrigin->GetValue();
}
}
