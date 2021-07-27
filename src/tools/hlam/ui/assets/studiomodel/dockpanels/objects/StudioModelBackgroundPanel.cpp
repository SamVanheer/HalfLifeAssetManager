#include <QFileDialog>
#include <QMessageBox>

#include "graphics/IGraphicsContext.hpp"

#include "qt/QtUtilities.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/dockpanels/objects/StudioModelBackgroundPanel.hpp"

namespace ui::assets::studiomodel
{
StudioModelBackgroundPanel::StudioModelBackgroundPanel(StudioModelAsset* asset, QWidget* parent)
	: QWidget(parent)
	, _asset(asset)
{
	_ui.setupUi(this);

	connect(_ui.ShowBackground, &QCheckBox::stateChanged, this, &StudioModelBackgroundPanel::OnShowBackgroundChanged);

	connect(_ui.BackgroundTexture, &QLineEdit::textChanged, this, &StudioModelBackgroundPanel::OnTextureChanged);
	connect(_ui.BrowseBackgroundTexture, &QPushButton::clicked, this, &StudioModelBackgroundPanel::OnBrowseTexture);
}

void StudioModelBackgroundPanel::OnShowBackgroundChanged()
{
	_asset->GetScene()->ShowBackground = _ui.ShowBackground->isChecked();
}

void StudioModelBackgroundPanel::OnTextureChanged()
{
	auto scene = _asset->GetScene();

	const QString fileName = _ui.BackgroundTexture->text();

	bool setTexture = false;

	scene->GetGraphicsContext()->Begin();

	if (!fileName.isEmpty())
	{
		QImage image{fileName};

		if (!image.isNull())
		{
			image.convertTo(QImage::Format::Format_RGBA8888);

			if (scene->BackgroundTexture == 0)
			{
				glGenTextures(1, &scene->BackgroundTexture);
			}

			glBindTexture(GL_TEXTURE_2D, scene->BackgroundTexture);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.constBits());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			setTexture = true;
		}
	}

	if (!setTexture)
	{
		if (scene->BackgroundTexture != 0)
		{
			glDeleteTextures(1, &scene->BackgroundTexture);
			scene->BackgroundTexture = 0;
		}
	}

	scene->GetGraphicsContext()->End();
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
