#pragma once

#include <QString>
#include <QWidget>

#include "ui_StudioModelView.h"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"

namespace studiomodel
{
/**
*	@brief A scene view, an info bar and a tab bar to select the current scene
*/
class StudioModelView final : public QWidget
{
	Q_OBJECT

public:
	StudioModelView(QWidget* parent);
	~StudioModelView();

	InfoBar* GetInfoBar() const { return _ui.InfoBar; }

	int GetSceneIndex() const;

	void AddScene(const QString& label);

	void SetWidget(QWidget* widget);

signals:
	void SceneIndexChanged(int index);

	void PoseChanged(Pose pose);

public slots:
	void SetSceneIndex(int index);

private slots:
	void OnPoseChanged(int index);

private:
	Ui_StudioModelView _ui;
};
}
