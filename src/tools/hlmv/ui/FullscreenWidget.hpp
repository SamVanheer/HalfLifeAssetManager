#pragma once

#include <QMainWindow>

namespace graphics
{
class Scene;
}

namespace ui
{
class EditorContext;

class FullscreenWidget final : public QMainWindow
{
	Q_OBJECT

public:
	FullscreenWidget(EditorContext* editorContext, graphics::Scene* scene, QWidget* parent = nullptr);
	~FullscreenWidget();

signals:
	void Closing(FullscreenWidget* widget);

protected:
	bool eventFilter(QObject* object, QEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;
	void closeEvent(QCloseEvent* event) override;
};
}
