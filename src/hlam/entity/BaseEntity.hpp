#pragma once

#include <glm/vec3.hpp>

#include "graphics/GraphicsConstants.hpp"

#include "utility/Class.hpp"

class BaseEntity;
class EntityList;
class QOpenGLFunctions_1_1;
class WorldTime;

namespace graphics
{
class SceneContext;
class TextureLoader;
}

namespace soundsystem
{
class ISoundSystem;
}

namespace sprite
{
class ISpriteRenderer;
}

namespace studiomdl
{
class IStudioModelRenderer;
}

namespace ui::assets::studiomodel
{
class StudioModelAsset;
}

namespace ui::settings
{
class StudioModelSettings;
}

/**
*	Pointer to member function used for think methods.
*/
using ThinkFunction = void (BaseEntity::*)();

struct EntityContext final
{
	//TODO: improve this
	ui::assets::studiomodel::StudioModelAsset* Asset;
	WorldTime* const Time;
	studiomdl::IStudioModelRenderer* const StudioModelRenderer;
	sprite::ISpriteRenderer* const SpriteRenderer;
	soundsystem::ISoundSystem* const SoundSystem;
	ui::settings::StudioModelSettings* const Settings;

	EntityContext(ui::assets::studiomodel::StudioModelAsset* asset,
		WorldTime* time,
		studiomdl::IStudioModelRenderer* studioModelRenderer, sprite::ISpriteRenderer* spriteRenderer,
		soundsystem::ISoundSystem* soundSystem,
		ui::settings::StudioModelSettings* settings)
		: Asset(asset)
		, Time(time)
		, SpriteRenderer(spriteRenderer)
		, StudioModelRenderer(studioModelRenderer)
		, SoundSystem(soundSystem)
		, Settings(settings)
	{
	}
};

/**
*	@brief Base class for all entities
*/
class BaseEntity
{
public:
	DECLARE_CLASS_NOBASE(BaseEntity);

public:
	BaseEntity() = default;
	virtual ~BaseEntity() = default;

	BaseEntity(const BaseEntity&) = delete;
	BaseEntity& operator=(const BaseEntity&) = delete;

	/**
	*	@brief Called when the entity has finished initializing.
	*/
	virtual void Spawn() {}

	virtual RenderPasses GetRenderPasses() const { return RenderPass::None; }

	virtual void Draw(QOpenGLFunctions_1_1* openglFunctions, graphics::SceneContext& sc, RenderPasses renderPass) {}

	virtual void CreateDeviceObjects(QOpenGLFunctions_1_1* openglFunctions, graphics::TextureLoader& textureLoader) {}

	virtual void DestroyDeviceObjects(QOpenGLFunctions_1_1* openglFunctions, graphics::TextureLoader& textureLoader) {}

private:
	EntityContext* _context{};
	EntityList* _entityList{};

public:
	EntityContext* GetContext() const { return _context; }

	void SetEntityContext(EntityContext* context);

	EntityList* GetEntityList() const { return _entityList; }

	void SetEntityList(EntityList* entityList);

private:
	glm::vec3 _origin{0};
	glm::vec3 _angles{0};
	glm::vec3 _scale{1.0f, 1.0f, 1.0f};

	float _transparency = 1.0f;

public:
	bool AlwaysThink = false;

	const glm::vec3& GetOrigin() const { return _origin; }

	void SetOrigin(const glm::vec3& origin) { _origin = origin; }

	const glm::vec3& GetAngles() const { return _angles; }

	void SetAngles(const glm::vec3& angles) { _angles = angles; }

	const glm::vec3& GetScale() const { return _scale; }

	/**
	*	@brief The returned vector can be modified and affects the scale.
	*/
	glm::vec3& GetScale() { return _scale; }

	void SetScale(const glm::vec3& scale) { _scale = scale; }

	float GetTransparency() const { return _transparency; }

	void SetTransparency(const float transparency);

private:
	ThinkFunction _thinkFunc = nullptr;
	float _lastThinkTime = 0;
	float _nextThinkTime = 0;

public:
	ThinkFunction GetThink() const { return _thinkFunc; }

	template<typename T>
	void SetThink(void (T::* func)())
	{
		_thinkFunc = static_cast<ThinkFunction>(func);
	}

	/**
	*	@see SetThink(void (T::* func)())
	*	Overload used to set the think method to null.
	*/
	void SetThink(nullptr_t)
	{
		_thinkFunc = nullptr;
	}

	float GetLastThinkTime() const { return _lastThinkTime; }

	/**
	*	@brief Sets the last think time. Should only be used by the entity manager.
	*/
	void SetLastThinkTime(const float flLastThink) { _lastThinkTime = flLastThink; }

	float GetNextThinkTime() const { return _nextThinkTime; }

	void SetNextThinkTime(const float flNextThink) { _nextThinkTime = flNextThink; }

	void Think()
	{
		if (_thinkFunc)
		{
			(this->*_thinkFunc)();
		}
	}
};