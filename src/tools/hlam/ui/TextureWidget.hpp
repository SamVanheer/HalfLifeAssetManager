#pragma once

#include <QColor>
#include <QImage>
#include <QWidget>

namespace ui
{
/**
*	@brief Draws a texture on-screen
*/
class TextureWidget : public QWidget
{
	Q_OBJECT

public:
	using QWidget::QWidget;

	QColor GetBackgroundColor() const { return _backgroundColor; }

	void SetBackgroundColor(const QColor& color)
	{
		_backgroundColor = color;
		update();
	}

	QImage GetImage() const { return _image; }

	void SetImage(const QImage& image)
	{
		_image = image;
		update();
	}

signals:
	void ScaleChanged(double amount);

public slots:
	void ResetImagePosition()
	{
		_imageOffset = {};
	}

protected:
	void paintEvent(QPaintEvent* event) override;

	void mousePressEvent(QMouseEvent* event) override final;

	void mouseReleaseEvent(QMouseEvent* event) override final;

	void mouseMoveEvent(QMouseEvent* event) override final;

private:
	QColor _backgroundColor{Qt::GlobalColor::gray};

	QImage _image;

	QPoint _imageOffset{};

	Qt::MouseButtons _trackedMouseButtons;
	QPoint _dragPosition{};
};
}
