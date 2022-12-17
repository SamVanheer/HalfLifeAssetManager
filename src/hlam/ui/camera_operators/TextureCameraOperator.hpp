#pragma once

#include <memory>

#include <glm/vec2.hpp>

#include "ui/camera_operators/CameraOperator.hpp"

class TextureEntity;

/**
*	@brief Camera operator that allows the user to view a texture and move the camera along the major axes
*/
class TextureCameraOperator final : public CameraOperator
{
	Q_OBJECT

public:
	static constexpr float DefaultFirstPersonFieldOfView = 74.f;

	TextureCameraOperator(std::shared_ptr<TextureEntity> textureEntity)
		: _textureEntity(textureEntity)
	{
		// TODO: not actually used.
		_camera.SetOrigin(glm::vec3{0});
		_camera.SetFieldOfView(DefaultFirstPersonFieldOfView);
	}

	void MouseEvent(QMouseEvent& event) override;

signals:
	void ScaleChanged(float adjust);

private:
	const std::shared_ptr<TextureEntity> _textureEntity;

	Qt::MouseButtons _trackedMouseButtons;
	glm::ivec2 _dragPosition{0};
};
