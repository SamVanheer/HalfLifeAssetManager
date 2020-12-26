#pragma once

#include <glm/vec3.hpp>

#include "core/shared/Class.hpp"

#include "engine/shared/renderer/DrawConstants.hpp"

#include "game/entity/EHandle.hpp"
#include "game/entity/EntityConstants.hpp"
#include "game/entity/EntityDict.hpp"

//Windows defines this
#ifdef GetClassName
#undef GetClassName
#endif

class BaseEntity;
class BaseEntityList;
class EntityManager;
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
	BaseEntityList* const EntityList;
	::EntityManager* const EntityManager;
	soundsystem::ISoundSystem* const SoundSystem;

	EntityContext(WorldTime* time,
		studiomdl::IStudioModelRenderer* studioModelRenderer, sprite::ISpriteRenderer* spriteRenderer,
		BaseEntityList* entityList, ::EntityManager* entityManager,
		soundsystem::ISoundSystem* soundSystem)
		: Time(time)
		, SpriteRenderer(spriteRenderer)
		, StudioModelRenderer(studioModelRenderer)
		, EntityList(entityList)
		, EntityManager(entityManager)
		, SoundSystem(soundSystem)
	{
	}
};

/**
*	Base class for all entities.
*/
class BaseEntity
{
public:
	DECLARE_CLASS_NOBASE(BaseEntity);

public:
	BaseEntity();
	~BaseEntity();

	BaseEntity(const BaseEntity&) = delete;
	BaseEntity& operator=(const BaseEntity&) = delete;

	/**
	*	Called when the object has been fully constructed.
	*/
	virtual void OnCreate();

	/**
	*	Called right before the object is to be destroyed.
	*/
	virtual void OnDestroy();

	/**
	*	Called when the entity has finished initializing.
	*	@return true on success, false if the entity should be destroyed.
	*/
	virtual bool Spawn();

	/**
	*	Draws this entity.
	*/
	virtual void Draw(renderer::DrawFlags flags) {}

private:
	const char* _className = nullptr;
	EHandle _entHandle;

	EntityContext* _context{};

public:
	/**
	*	Called when the entity is constructed. Do not call directly.
	*	@param className The entity's class name
	*	@param context Entity context
	*/
	void Construct(const char* const className, EntityContext* context);

	/**
	*	Gets the entity's class name.
	*/
	const char* GetClassName() const { return _className; }

	/**
	*	Gets the handle that represents this entity.
	*/
	const EHandle& GetEntHandle() const { return _entHandle; }

	/**
	*	Sets the handle that represents this entity. Should only be used by the entity list.
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
	/**
	*	Gets the entity's flags.
	*/
	entity::Flags GetFlags() const { return _flags; }

	/**
	*	Returns if any of the given flags are set.
	*/
	bool AnyFlagsSet(const entity::Flags flags) const { return (_flags & flags) != 0; }

	/**
	*	Returns if all of the given flags are set.
	*/
	bool AllFlagsSet(const entity::Flags flags) const { return (_flags & flags) == flags; }

	/**
	*	Sets the entity's flags to the given flags.
	*/
	void InitFlags(const entity::Flags flags) { _flags = flags; }

	/**
	*	Sets the given flags on the entity. Existing flags are unaffected.
	*/
	void SetFlags(const entity::Flags flags) { _flags |= flags; }

	/**
	*	Clears the given flags from the entity's flags.
	*/
	void ClearFlags(const entity::Flags flags) { _flags &= ~flags; }

	/**
	*	Gets the entity's origin.
	*/
	const glm::vec3& GetOrigin() const { return _origin; }

	/**
	*	@copydoc GetOrigin() const
	*	The returned vector can be modified and affects the origin.
	*/
	glm::vec3& GetOrigin() { return _origin; }

	/**
	*	Sets the entity's origin.
	*/
	void SetOrigin(const glm::vec3& origin) { _origin = origin; }

	/**
	*	Gets the entity's angles.
	*/
	const glm::vec3& GetAngles() const { return _angles; }

	/**
	*	@copydoc GetAngles() const
	*	The returned vector can be modified and affects the angles.
	*/
	glm::vec3& GetAngles() { return _angles; }

	/**
	*	Sets the entity's angles.
	*/
	void SetAngles(const glm::vec3& angles) { _angles = angles; }

	/**
	*	Gets the entity's scale.
	*/
	const glm::vec3& GetScale() const { return _scale; }

	/**
	*	@copydoc GetScale() const
	*	The returned vector can be modified and affects the scale.
	*/
	glm::vec3& GetScale() { return _scale; }

	/**
	*	Sets the entity's scale.
	*/
	void SetScale(const glm::vec3& scale) { _scale = scale; }

	/**
	*	Gets the entity's transparency.
	*/
	float GetTransparency() const { return _transparency; }

	/**
	*	Sets the entity's transparency.
	*/
	void SetTransparency(const float transparency);

private:
	ThinkFunction _thinkFunc = nullptr;
	float _lastThinkTime = 0;
	float _nextThinkTime = 0;

public:
	/**
	*	Gets the think method.
	*/
	ThinkFunction GetThink() const { return _thinkFunc; }

	/**
	*	Sets the entity's think method.
	*/
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

	/**
	*	Gets the last think time.
	*/
	float GetLastThinkTime() const { return _lastThinkTime; }

	/**
	*	Sets the last think time. Should only be used by the entity manager.
	*/
	void SetLastThinkTime(const float flLastThink) { _lastThinkTime = flLastThink; }

	/**
	*	Gets the next think time.
	*/
	float GetNextThinkTime() const { return _nextThinkTime; }

	/**
	*	Sets the next think time.
	*/
	void SetNextThinkTime(const float flNextThink) { _nextThinkTime = flNextThink; }

	/**
	*	Runs the think method. NOTE: non-virtual.
	*/
	void Think()
	{
		if (_thinkFunc)
		{
			(this->*_thinkFunc)();
		}
	}
};
