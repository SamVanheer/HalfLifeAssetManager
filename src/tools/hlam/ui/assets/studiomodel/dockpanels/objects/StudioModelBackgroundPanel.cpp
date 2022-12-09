#include <QFileDialog>
#include <QMessageBox>

#include "entity/BackgroundEntity.hpp"

#include "graphics/IGraphicsContext.hpp"
#include "graphics/Scene.hpp"

#include "qt/QtUtilities.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/dockpanels/objects/StudioModelBackgroundPanel.hpp"

#include "ui/settings/StudioModelSettings.hpp"

namespace ui::assets::studiomodel
{
StudioModelBackgroundPanel::StudioModelBackgroundPanel(StudioModelAsset* asset)
	: _asset(asset)
{
	_ui.setupUi(this);

	connect(_ui.ShowBackground, &QCheckBox::stateChanged, this, &StudioModelBackgroundPanel::OnShowBackgroundChanged);

	connect(_ui.BackgroundTexture, &QLineEdit::textChanged, this, &StudioModelBackgroundPanel::OnTextureChanged);
	connect(_ui.BrowseBackgroundTexture, &QPushButton::clicked, this, &StudioModelBackgroundPanel::OnBrowseTexture);
}

void StudioModelBackgroundPanel::OnShowBackgroundChanged()
{
	_asset->GetProvider()->GetStudioModelSettings()->ShowBackground = _ui.ShowBackground->isChecked();
}

void StudioModelBackgroundPanel::OnTextureChanged()
{
	if (const QString fileName = _ui.BackgroundTexture->text(); !fileName.isEmpty())
	{
		if (QImage image{fileName}; !image.isNull())
		{
			image.convertTo(QImage::Format::Format_RGBA8888);

			auto scene = _asset->GetScene();

			scene->GetGraphicsContext()->Begin();
			_asset->GetBackgroundEntity()->SetImage(scene->GetOpenGLFunctions(), image.width(), image.height(), reinterpret_cast<const std::byte*>(image.constBits()));
			scene->GetGraphicsContext()->End();

			_ui.ShowBackground->setChecked(true);
		}
	}
}

void StudioModelBackgroundPanel::OnBrowseTexture()
{
	const QString fileName{QFileDialog::getOpenFileName(nullptr, {}, {}, qt::GetImagesFileFilter())};

	if (!fileName.isEmpty())
	{
		_ui.BackgroundTexture->setText(fileName);
	}
}
}
