#pragma once

#include <QMainWindow>

namespace ui
{
/**
*	@brief Window to host fullscreen widgets specified by assets that support it
*	Can be used as an event filter to handle exiting fullscreen by pressing the exit fullscreen key
*/
class FullscreenWidget final : public QMainWindow
{
	Q_OBJECT

public:
	static constexpr Qt::Key ExitFullscreenKey{Qt::Key::Key_Escape};

	FullscreenWidget(QWidget* parent = nullptr);
	~FullscreenWidget();

	void ExitFullscreen();

protected:
	bool eventFilter(QObject* object, QEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;
};
}
