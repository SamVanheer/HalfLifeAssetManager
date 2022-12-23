#pragma once

#include <QMainWindow>

/**
*	@brief Window to host fullscreen widgets specified by assets that support it
*	Can be used as an event filter to handle exiting fullscreen by pressing the exit fullscreen key
*/
class FullscreenWidget final : public QMainWindow
{
	Q_OBJECT

public:
	static constexpr Qt::Key ExitFullscreenKey{Qt::Key::Key_Escape};
	static constexpr Qt::Key ToggleFullscreenKey{Qt::Key::Key_F11};

	FullscreenWidget();
	~FullscreenWidget();

	void SetWidget(QWidget* widget);

	void ExitFullscreen();

signals:
	void ExitedFullscreen();

private:
	bool ProcessKeyEvent(QKeyEvent* event);

	void CleanupOldWidget();

protected:
	bool eventFilter(QObject* object, QEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;
	void closeEvent(QCloseEvent* event) override;
};
