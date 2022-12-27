#include <QFileDialog>
#include <QSignalBlocker>

#include "entity/BackgroundEntity.hpp"

#include "qt/QtUtilities.hpp"

#include "plugins/halflife/studiomodel/StudioModelAsset.hpp"
#include "plugins/halflife/studiomodel/ui/dockpanels/objects/BackgroundPanel.hpp"

namespace studiomodel
{
BackgroundPanel::BackgroundPanel(StudioModelAssetProvider* provider)
	: _provider(provider)
{
	_ui.setupUi(this);

	connect(_provider, &StudioModelAssetProvider::AssetChanged, this, &BackgroundPanel::OnAssetChanged);

	connect(_ui.ShowBackground, &QCheckBox::stateChanged, this, &BackgroundPanel::OnShowBackgroundChanged);

	connect(_ui.BackgroundTexture, &QLineEdit::textChanged, this, &BackgroundPanel::OnTextureChanged);
	connect(_ui.BrowseBackgroundTexture, &QPushButton::clicked, this, &BackgroundPanel::OnBrowseTexture);
}

void BackgroundPanel::OnAssetChanged(StudioModelAsset* asset)
{
	const QSignalBlocker backgroundTextureBlocker{_ui.BackgroundTexture};
	const QSignalBlocker showBackgroundBlocker{_ui.ShowBackground};

	auto entity = asset->GetBackgroundEntity();

	_ui.BackgroundTexture->setText(QString::fromStdString(entity->GetImageName()));
	_ui.ShowBackground->setChecked(entity->ShowBackground);
}

void BackgroundPanel::OnShowBackgroundChanged()
{
	_provider->GetCurrentAsset()->GetBackgroundEntity()->ShowBackground = _ui.ShowBackground->isChecked();
}

void BackgroundPanel::OnTextureChanged()
{
	if (const QString fileName = _ui.BackgroundTexture->text(); !fileName.isEmpty())
	{
		if (QImage image{fileName}; !image.isNull())
		{
			image.convertTo(QImage::Format::Format_RGBA8888);

			_provider->GetCurrentAsset()->GetBackgroundEntity()->SetImage(fileName.toStdString(),
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
