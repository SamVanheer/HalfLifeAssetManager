#pragma once

#include <QColor>
#include <QObject>

namespace ui::assets::studiomodel
{
class StudioModelAsset;

/**
*	@brief Stores shared UI state for studio models
*/
class StudioModelContext final : public QObject
{
	Q_OBJECT

public:
	StudioModelContext(StudioModelAsset* asset, QObject* parent = nullptr)
		: QObject(parent)
		, _asset(asset)
	{
	}

	~StudioModelContext() = default;

	StudioModelAsset* GetAsset() { return _asset; }

	QColor GetBackgroundColor() const { return _backgroundColor; }

signals:
	void BackgroundColorChanged(QColor color);

public slots:
	void SetBackgroundColor(QColor color)
	{
		if (_backgroundColor != color)
		{
			_backgroundColor = color;

			emit BackgroundColorChanged(_backgroundColor);
		}
	}

private:
	StudioModelAsset* const _asset;
	QColor _backgroundColor;
};
}
