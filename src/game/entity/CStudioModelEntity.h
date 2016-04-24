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
	typedef std::vector<const mstudiomesh_t*> MeshList_t;

public:
	DECLARE_CLASS( CStudioModelEntity, CBaseAnimating );

public:
	virtual void OnCreate() override;

	virtual void OnDestroy() override;

	virtual bool Spawn() override;

	virtual void Draw( entity::DrawFlags_t flags ) override;

	float	AdvanceFrame( float dt = 0.0f );

	virtual void HandleAnimEvent( const CAnimEvent& event );

	int		GetAnimationEvent( CAnimEvent& event, float flStart, float flEnd, int index, const bool bAllowClientEvents );
	void	DispatchAnimEvents( const bool bAllowClientEvents );

	void AnimThink();

public:
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
	studiomodel::CStudioModel* GetModel() const { return m_pModel; }
	void SetModel( studiomodel::CStudioModel* pModel );

	int GetNumFrames() const;

	int GetSequence() const { return m_iSequence; }
	int SetSequence( const int iSequence );

	void GetSequenceInfo( float& flFrameRate, float& flGroundSpeed ) const;

	int GetBodygroup() const { return m_iBodygroup; }
	int SetBodygroup( const int iBodygroup, const int iValue );

	int GetSkin() const { return m_iSkin; }
	int SetSkin( const int iSkin );

	byte GetControllerByIndex( const int iController ) const;
	float GetControllerValue( const int iController ) const;
	float SetController( const int iController, float flValue );

	byte GetMouth() const { return m_uiMouth; }
	float SetMouth( float flValue );

	byte GetBlendingByIndex( const int iBlender ) const;
	float GetBlendingValue( const int iBlender ) const;
	float SetBlending( const int iBlender, float flValue );

	float GetLastEventCheck() const { return m_flLastEventCheck; }

	float GetAnimTime() const { return m_flAnimTime; }

	void ExtractBbox( glm::vec3& vecMins, glm::vec3& vecMaxs ) const;

	mstudiomodel_t* GetModelByBodyPart( const int iBodyPart ) const;

	MeshList_t ComputeMeshList( const int iTexture ) const;
};

#endif //GAME_CSTUDIOMODELENTITY_H