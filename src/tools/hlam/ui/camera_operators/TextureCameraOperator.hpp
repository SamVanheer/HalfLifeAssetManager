#pragma once

#include <memory>

#include <glm/vec2.hpp>

#include "ui/camera_operators/CameraOperator.hpp"

class TextureEntity;

namespace ui::camera_operators
{
/**
*	@brief Camera operator that allows the user to view a texture and move the camera along the major axes
*/
class TextureCameraOperator final : public CameraOperator
{
	Q_OBJECT

public:
	static constexpr float DefaultFirstPersonFieldOfView = 74.f;

	TextureCameraOperator(settings::GeneralSettings* generalSettings, std::shared_ptr<TextureEntity> textureEntity)
		: CameraOperator(generalSettings)
		, _textureEntity(textureEntity)
	{
		// TODO: not actually used.
		_camera.SetOrigin(glm::vec3{0});
		_camera.SetFieldOfView(DefaultFirstPersonFieldOfView);
	}

	QString GetName() const override
	{
		return "Texture";
	}

	QWidget* CreateEditWidget() override
	{
		// TODO: probably won't support this.
		return nullptr;
	}

	void MouseEvent(QMouseEvent& event) override;

	// TODO: split CameraOperator into a base class with only input handling and a derived class for all of this stuff.
	void CenterView(const glm::vec3&, const glm::vec3&, float, float) override {}

	void SaveView() override {}

	void RestoreView() override {}

signals:
	void ScaleChanged(float adjust);

private:
	const std::shared_ptr<TextureEntity> _textureEntity;

	Qt::MouseButtons _trackedMouseButtons;
	glm::ivec2 _dragPosition{0};
};
}
