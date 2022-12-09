#include "entity/PlayerHitboxEntity.hpp"

#include "graphics/GraphicsUtils.hpp"

#include "ui/settings/StudioModelSettings.hpp"

void PlayerHitboxEntity::Draw(QOpenGLFunctions_1_1* openglFunctions, graphics::SceneContext& sc, RenderPasses renderPass)
{
	if (GetContext()->Settings->ShowPlayerHitbox)
	{
		//Draw a transparent green box to display the player hitbox
		const glm::vec3 bbmin{-16, -16, 0};
		const glm::vec3 bbmax{16, 16, 72};

		auto v = graphics::CreateBoxFromBounds(bbmin, bbmax);

		graphics::DrawOutlinedBox(openglFunctions, v, {0.0f, 1.0f, 0.0f, 0.5f}, {0.0f, 0.5f, 0.0f, 1.f});
	}
}
