#include "graphics/OpenGL.h"

#include "shared/CWorldTime.h"

#include "engine/shared/sprite/sprite.h"
#include "engine/shared/sprite/CSprite.h"

#include "CSpriteEntity.h"

LINK_ENTITY_TO_CLASS( sprite, CSpriteEntity );

void CSpriteEntity::OnDestroy()
{
	sprite::FreeSprite( m_pSprite );

	BaseClass::OnDestroy();
}

bool CSpriteEntity::Spawn()
{
	SetThink( &ThisClass::AnimThink );

	SetFlags( entity::FL_ALWAYSTHINK );

	return true;
}

void CSpriteEntity::Draw( entity::DrawFlags_t flags )
{
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	glOrtho( 0.0f, ( float ) 1920, ( float ) 730, 0.0f, 1.0f, -1.0f );

	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();

	const auto& framedesc = m_pSprite->frames[ static_cast<int>( m_flFrame ) ];
	const auto& frame = framedesc.frameptr;

	glEnable( GL_TEXTURE_2D );
	glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
	glBindTexture( GL_TEXTURE_2D, frame->gl_texturenum );

	float x = 100;
	float y = 100;
	float w = frame->width * 4;
	float h = frame->height * 4;

	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_CULL_FACE );
	glEnable( GL_DEPTH_TEST );
	glShadeModel( GL_SMOOTH );

	glBegin( GL_TRIANGLE_STRIP );

	glTexCoord2f( 0, 0 );
	glVertex2f( x, y );

	glTexCoord2f( 1, 0 );
	glVertex2f( x + w, y );

	glTexCoord2f( 0, 1 );
	glVertex2f( x, y + h );

	glTexCoord2f( 1, 1 );
	glVertex2f( x + w, y + h );

	glEnd();

	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	glDisable( GL_TEXTURE_2D );
	glDisable( GL_CULL_FACE );
	glDisable( GL_DEPTH_TEST );

	glBegin( GL_TRIANGLE_STRIP );

	glVertex2f( x, y );

	glVertex2f( x + w, y );

	glVertex2f( x, y + h );

	glVertex2f( x + w, y + h );

	glEnd();

	glPopMatrix();
}

void CSpriteEntity::AnimThink()
{
	m_flFrame += WorldTime.GetFrameTime() * 10;

	if( m_flFrame >= m_pSprite->numframes )
		m_flFrame = 0;
}

void CSpriteEntity::SetSprite( sprite::msprite_t* pSprite )
{
	//TODO: release old sprite
	m_pSprite = pSprite;
}