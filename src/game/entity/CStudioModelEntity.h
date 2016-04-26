#ifndef GAME_CSTUDIOMODELENTITY_H
#define GAME_CSTUDIOMODELENTITY_H

#include <vector>

#include "game/studiomodel/CStudioModel.h"

#include "game/CAnimEvent.h"
#include "game/Events.h"

#include "CBaseAnimating.h"

/**
*	Studio model entity.
*/
class CStudioModelEntity : public CBaseAnimating
{
public:
	DECLARE_CLASS( CStudioModelEntity, CBaseAnimating );

public:
	typedef std::vector<const mstudiomesh_t*> MeshList_t;

public:
	virtual void OnDestroy() override;

	virtual bool Spawn() override;

	virtual void Draw( entity::DrawFlags_t flags ) override;

	/**
	*	Advances the frame. If dt is 0, advances to current time, otherwise, advances by the given amount of time.
	*	TODO: clamp dt to positive?
	*	@param dt Delta time.
	*	@param flMax Maximum amount of time to advance by. If -1, no limit.
	*	@return Delta time that was used to advance the frame. Can be 0.
	*/
	float	AdvanceFrame( float dt = 0.0f, const float flMax = -1.f );

	/**
	*	Gets an animation event for the current sequence for the given time range.
	*	@param event Output. Event data.
	*	@param flStart Start of the range of frames to check.
	*	@param flEnd End of the range of frames to check.
	*	@param index Event index to start checking at.
	*	@param bAllowClientEvents Whether to process client events or not.
	*	@return Next event index to use as the index parameter. If 0, no more events are left.
	*/
	int		GetAnimationEvent( CAnimEvent& event, float flStart, float flEnd, int index, const bool bAllowClientEvents );

	/**
	*	Dispatches events for the current sequence and frame. This will dispatch events between the frame number during last call to DispatchAnimEvents and the current frame.
	*	@param bAllowClientEvents Whether to process client events or not.
	*/
	void	DispatchAnimEvents( const bool bAllowClientEvents );

	/**
	*	Method to handle animation events. Override to handle events.
	*/
	virtual void HandleAnimEvent( const CAnimEvent& event );

public:
	/**
	*	Sets the frame for this model.
	*	@param iFrame Frame to use.
	*	@return Frame that is currently used.
	*/
	int SetFrame( const int iFrame );

private:
	studiomodel::CStudioModel* m_pModel = nullptr;

	int		m_iSequence			= 0;				// sequence index
	int		m_iBodygroup		= 0;				// bodypart selection	
	int		m_iSkin				= 0;				// skin group selection
	byte	m_uiController[ STUDIO_MAX_CONTROLLERS ] = { 0, 0, 0, 0 };	// bone controllers
	byte	m_uiMouth			= 0;				// mouth position
	byte	m_uiBlending[ STUDIO_MAX_BLENDERS ]	= { 0, 0 };			// animation blending

	float	m_flLastEventCheck	= 0;				//Last time we checked for animation events.
	float	m_flAnimTime		= 0;				//Time when the frame was set.

public:
	/**
	*	Gets the model.
	*/
	studiomodel::CStudioModel* GetModel() const { return m_pModel; }

	/**
	*	Sets the model.
	*/
	void SetModel( studiomodel::CStudioModel* pModel );

	/**
	*	Gets the number of frames that the current sequence has.
	*/
	int GetNumFrames() const;

	/**
	*	Gets the current sequence index.
	*/
	int GetSequence() const { return m_iSequence; }

	/**
	*	Sets the current sequence.
	*	@param iSequence Sequence to use.
	*	@return Sequence that is currently used.
	*/
	int SetSequence( const int iSequence );

	/**
	*	Gets info from the current sequence.
	*	@param flFrameRate The sequence's frame rate.
	*	@param flGroundSpeed How fast the entity should move on the ground during this sequence.
	*/
	void GetSequenceInfo( float& flFrameRate, float& flGroundSpeed ) const;

	/**
	*	Gets the bodygroup.
	*/
	int GetBodygroup() const { return m_iBodygroup; }

	/**
	*	Sets the value for the given bodygroup.
	*	@param iBodygroup Bodygroup to set.
	*	@param iValue Value to set.
	*	@return Value that is currently used, or -1 if the bodygroup was invalid, or if the value was not changed.
	*/
	int SetBodygroup( const int iBodygroup, const int iValue );

	/**
	*	Gets the current skin.
	*/
	int GetSkin() const { return m_iSkin; }

	/**
	*	Sets the current skin.
	*	@param iSkin Skin to use.
	*	@return Skin that is currently used.
	*/
	int SetSkin( const int iSkin );

	/**
	*	Gets the given controller by index. This is the stored value, not the computed value.
	*	@param iController Controller to get.
	*/
	byte GetControllerByIndex( const int iController ) const;

	/**
	*	Gets the given controller value. The value is computed for the associated bone controller.
	*	@param iController Controller value to get.
	*/
	float GetControllerValue( const int iController ) const;

	/**
	*	Sets the controller value. The value is processed into a value that is in the range [0, 255].
	*	@param iController Controller to set.
	*	@param flValue Value to set.
	*	@return The value that was set, or flValue if the controller index is out of bounds.
	*/
	float SetController( const int iController, float flValue );

	/**
	*	Gets the mouth controller. This is the stored value, not the computed value.
	*/
	byte GetMouth() const { return m_uiMouth; }

	/**
	*	Sets the mouth controller value. The value is processed into a value that is in the range [0, 255]
	*	@param flValue Value to set.
	*	@return The value that was set.
	*/
	float SetMouth( float flValue );

	/**
	*	Gets the given blender by index. This is the stored value, not the computed value.
	*	@param iBlender Blender to get.
	*/
	byte GetBlendingByIndex( const int iBlender ) const;

	/**
	*	Gets the given blender value. The value is computed for the associated blender.
	*	@param iBlender Blender value to get.
	*/
	float GetBlendingValue( const int iBlender ) const;

	/**
	*	Sets the given blender's value.
	*	@param iBlender Blender to set.
	*	@param flValue Value to set.
	*	@return The value that was set, or 0 if the blender index is out of bounds.
	*/
	float SetBlending( const int iBlender, float flValue );

	/**
	*	Gets the last event check. This is the end of the range used to check for animation events the last time DispatchAnimEvents was called.
	*/
	float GetLastEventCheck() const { return m_flLastEventCheck; }

	/**
	*	Gets the last time this entity advanced its frame.
	*/
	float GetAnimTime() const { return m_flAnimTime; }

	/**
	*	Extracts the bounding box from the current sequence.
	*/
	void ExtractBbox( glm::vec3& vecMins, glm::vec3& vecMaxs ) const;

	/**
	*	Gets a model by body part.
	*/
	mstudiomodel_t* GetModelByBodyPart( const int iBodyPart ) const;

	/**
	*	Computes a list of meshes that use the given texture.
	*/
	MeshList_t ComputeMeshList( const int iTexture ) const;
};

#endif //GAME_CSTUDIOMODELENTITY_H