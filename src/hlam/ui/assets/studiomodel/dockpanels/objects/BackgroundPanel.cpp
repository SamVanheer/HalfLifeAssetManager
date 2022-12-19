#include <QFileDialog>

#include "entity/BackgroundEntity.hpp"

#include "qt/QtUtilities.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/dockpanels/objects/BackgroundPanel.hpp"

#include "ui/settings/StudioModelSettings.hpp"

namespace studiomodel
{
BackgroundPanel::BackgroundPanel(StudioModelAssetProvider* provider)
	: _provider(provider)
{
	_ui.setupUi(this);

	connect(_ui.ShowBackground, &QCheckBox::stateChanged, this, &BackgroundPanel::OnShowBackgroundChanged);

	connect(_ui.BackgroundTexture, &QLineEdit::textChanged, this, &BackgroundPanel::OnTextureChanged);
	connect(_ui.BrowseBackgroundTexture, &QPushButton::clicked, this, &BackgroundPanel::OnBrowseTexture);
}

void BackgroundPanel::OnShowBackgroundChanged()
{
	_provider->GetStudioModelSettings()->ShowBackground = _ui.ShowBackground->isChecked();
}

void BackgroundPanel::OnTextureChanged()
{
	if (const QString fileName = _ui.BackgroundTexture->text(); !fileName.isEmpty())
	{
		if (QImage image{fileName}; !image.isNull())
		{
			image.convertTo(QImage::Format::Format_RGBA8888);

			_provider->GetCurrentAsset()->GetBackgroundEntity()->SetImage(
				{image.width(), image.height(), reinterpret_cast<const std::byte*>(image.constBits())});

			_ui.ShowBackground->setChecked(true);
		}
	}
}

void BackgroundPanel::OnBrowseTexture()
{
	const QString fileName{QFileDialog::getOpenFileName(nullptr, {}, {}, qt::GetImagesFileFilter())};

	if (!fileName.isEmpty())
	{
		_ui.BackgroundTexture->setText(fileName);
	}
}
}
