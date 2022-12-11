#pragma once

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QEvent>
#include <QMimeData>
#include <QObject>

#include "EditorContext.hpp"

namespace ui
{
/**
*	@brief Handles drag&drop events and automatically forwards dropped files to the editor context
*/
class DragNDropEventFilter : public QObject
{
public:
	DragNDropEventFilter(EditorContext* editorContext, QObject* parent)
		: QObject(parent)
		, _editorContext(editorContext)
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
				_editorContext->TryLoadAsset(url.toLocalFile());
			}

			return true;
		}
		}

		return false;
	}

private:
	EditorContext* const _editorContext;
};
}
