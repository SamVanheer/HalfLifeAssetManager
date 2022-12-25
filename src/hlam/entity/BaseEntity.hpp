#pragma once

#include <glm/vec3.hpp>

#include "graphics/GraphicsConstants.hpp"
#include "graphics/OpenGL.hpp"

#include "utility/Class.hpp"

class BaseEntity;
class EntityList;
class WorldTime;

namespace graphics
{
class SceneContext;
}

class ISoundSystem;

namespace sprite
{
class ISpriteRenderer;
}

namespace studiomdl
{
class IStudioModelRenderer;
}

namespace studiomodel
{
class StudioModelAsset;
}

class GeneralSettings;
class StudioModelSettings;

/**
*	Pointer to member function used for think methods.
*/
using ThinkFunction = void (BaseEntity::*)();

struct EntityContext final
{
	//TODO: improve this
	studiomodel::StudioModelAsset* Asset;
	WorldTime* const Time;
	studiomdl::IStudioModelRenderer* const StudioModelRenderer;
	sprite::ISpriteRenderer* const SpriteRenderer;
	ISoundSystem* const SoundSystem;
	GeneralSettings* const GeneralSettings;
	StudioModelSettings* const Settings;

	EntityContext(studiomodel::StudioModelAsset* asset,
		WorldTime* time,
		studiomdl::IStudioModelRenderer* studioModelRenderer,
		sprite::ISpriteRenderer* spriteRenderer,
		ISoundSystem* soundSystem,
		::GeneralSettings* generalSettings,
		StudioModelSettings* settings)
		: Asset(asset)
		, Time(time)
		, SpriteRenderer(spriteRenderer)
		, StudioModelRenderer(studioModelRenderer)
		, SoundSystem(soundSystem)
		, GeneralSettings(generalSettings)
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

	virtual void Draw(graphics::SceneContext& sc, RenderPasses renderPass) {}

	virtual void CreateDeviceObjects(graphics::SceneContext& sc) {}

	virtual void DestroyDeviceObjects(graphics::SceneContext& sc) {}

	virtual float GetRenderDistance(const glm::vec3& cameraOrigin) const { return 1000000000.0f; }

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
