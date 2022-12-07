#pragma once

#include <glm/vec3.hpp>

#include "entity/EntityConstants.hpp"

#include "graphics/GraphicsConstants.hpp"

#include "utility/Class.hpp"

class BaseEntity;
class EntityList;
class QOpenGLFunctions_1_1;
class WorldTime;

namespace graphics
{
class Scene;
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
	WorldTime* const Time;
	studiomdl::IStudioModelRenderer* const StudioModelRenderer;
	sprite::ISpriteRenderer* const SpriteRenderer;
	soundsystem::ISoundSystem* const SoundSystem;
	ui::settings::StudioModelSettings* const Settings;
	graphics::Scene* const Scene;

	EntityContext(WorldTime* time,
		studiomdl::IStudioModelRenderer* studioModelRenderer, sprite::ISpriteRenderer* spriteRenderer,
		soundsystem::ISoundSystem* soundSystem,
		ui::settings::StudioModelSettings* settings,
		graphics::Scene* scene)
		: Time(time)
		, SpriteRenderer(spriteRenderer)
		, StudioModelRenderer(studioModelRenderer)
		, SoundSystem(soundSystem)
		, Settings(settings)
		, Scene(scene)
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
	*	@details Set the FL_KILLME flag if you need to be destroyed after construction
	*/
	virtual void Spawn() {}

	virtual RenderPasses GetRenderPasses() const { return RenderPass::None; }

	virtual void Draw(QOpenGLFunctions_1_1* openglFunctions, RenderPasses renderPass) {}

private:
	EntityContext* _context{};
	EntityList* _entityList{};

public:
	EntityContext* GetContext() const { return _context; }

	void SetEntityContext(EntityContext* context);

	EntityList* GetEntityList() const { return _entityList; }

	void SetEntityList(EntityList* entityList);

private:
	entity::Flags _flags = entity::FL_NONE;
	glm::vec3 _origin{0};
	glm::vec3 _angles{0};
	glm::vec3 _scale{1.0f, 1.0f, 1.0f};

	float _transparency = 1.0f;

public:
	entity::Flags GetFlags() const { return _flags; }

	bool AnyFlagsSet(const entity::Flags flags) const { return (_flags & flags) != 0; }

	bool AllFlagsSet(const entity::Flags flags) const { return (_flags & flags) == flags; }

	/**
	*	@brief Sets the entity's flags to the given flags
	*/
	void InitFlags(const entity::Flags flags) { _flags = flags; }

	/**
	*	@brief Sets the given flags on the entity. Existing flags are unaffected.
	*/
	void SetFlags(const entity::Flags flags) { _flags |= flags; }

	/**
	*	@brief Clears the given flags from the entity's flags.
	*/
	void ClearFlags(const entity::Flags flags) { _flags &= ~flags; }

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
