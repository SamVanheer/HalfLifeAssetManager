#pragma once

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QEvent>
#include <QMimeData>
#include <QObject>

#include "application/AssetManager.hpp"

/**
*	@brief Handles drag&drop events and automatically forwards dropped files to the editor context
*/
class DragNDropEventFilter : public QObject
{
public:
	DragNDropEventFilter(AssetManager* application, QObject* parent)
		: QObject(parent)
		, _application(application)
	{
	}

protected:
	bool eventFilter(QObject* watched, QEvent* event) override
	{
		switch (event->type())
		{
		case QEvent::DragEnter:
		{
			auto dragEvent = static_cast<QDragEnterEvent*>(event);

			if (dragEvent->mimeData()->hasUrls())
			{
				dragEvent->acceptProposedAction();
			}

			return true;
		}

		case QEvent::Drop:
		{
			auto dropEvent = static_cast<QDropEvent*>(event);

			for (const auto& url : dropEvent->mimeData()->urls())
			{
				_application->TryLoadAsset(url.toLocalFile());
			}

			return true;
		}
		}

		return false;
	}

private:
	AssetManager* const _application;
};
