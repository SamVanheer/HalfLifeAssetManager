#pragma once

#include <glm/vec3.hpp>

#include "engine/shared/renderer/DrawConstants.hpp"

#include "entity/EHandle.hpp"
#include "entity/EntityConstants.hpp"

#include "utility/Class.hpp"

class BaseEntity;
class EntityList;
class WorldTime;

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
	::EntityList* const EntityList;
	soundsystem::ISoundSystem* const SoundSystem;

	EntityContext(WorldTime* time,
		studiomdl::IStudioModelRenderer* studioModelRenderer, sprite::ISpriteRenderer* spriteRenderer,
		::EntityList* entityList,
		soundsystem::ISoundSystem* soundSystem)
		: Time(time)
		, SpriteRenderer(spriteRenderer)
		, StudioModelRenderer(studioModelRenderer)
		, EntityList(entityList)
		, SoundSystem(soundSystem)
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
	~BaseEntity() = default;

	BaseEntity(const BaseEntity&) = delete;
	BaseEntity& operator=(const BaseEntity&) = delete;

	/**
	*	@brief Called when the entity has finished initializing.
	*	@details Set the FL_KILLME flag if you need to be destroyed after construction
	*/
	virtual void Spawn() {}

	virtual void Draw(renderer::DrawFlags flags) {}

private:
	EHandle _entHandle;

	EntityContext* _context{};

public:
	/**
	*	@brief Sets the entity context
	*/
	void SetEntityContext(EntityContext* context);

	/**
	*	@brief Gets the handle that represents this entity.
	*/
	constexpr const EHandle& GetEntHandle() const { return _entHandle; }

	/**
	*	@brief Sets the handle that represents this entity. Should only be used by the entity list.
	*/
	void SetEntHandle(const EHandle& handle) { _entHandle = handle; }

	EntityContext* GetContext() const { return _context; }

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
