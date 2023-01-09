#pragma once

#include <memory>

#include <QColor>
#include <QLoggingCategory>
#include <QWidget>

class AssetManager;
class Ui_MessagesPanel;

class MessagesPanel final : public QWidget
{
public:
	explicit MessagesPanel(AssetManager* application, QWidget* parent);
	~MessagesPanel();

private slots:
	void OnMessageReceived(QtMsgType type, const QMessageLogContext& context, const QString& msg);

private:
	std::unique_ptr<Ui_MessagesPanel> _ui;
	AssetManager* const _application;
	QColor _defaultTextColor;
};
