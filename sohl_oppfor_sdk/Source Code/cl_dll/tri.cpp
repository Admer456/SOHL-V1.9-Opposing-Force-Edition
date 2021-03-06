/***
*
*   SPIRIT OF HALF-LIFE 1.9: OPPOSING-FORCE EDITION
*
*   Half-Life and their logos are the property of their respective owners.
*   Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*   This product contains software technology licensed from Id
*   Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
*	Spirit of Half-Life, by Laurie R. Cheers. (LRC)
*   Modified by Lucas Brucksch (Code merge & Effects)
*   Modified by Andrew J Hamilton (AJH)
*   Modified by XashXT Group (g-cont...)
*
*   Code used from Battle Grounds Team and Contributors.
*   Code used from SamVanheer (Opposing Force code)
*   Code used from FWGS Team (Fixes for SOHL)
*   Code used from LevShisterov (Bugfixed and improved HLSDK)
*	Code used from Fograin (Half-Life: Update MOD)
*
***/

// Triangle rendering, if any

#include "hud.h"
#include "cl_util.h"
#include "windows.h"
#include "gl/gl.h"
#include <gl/glaux.h>

// Triangle rendering apis are in gEngfuncs.pTriAPI

#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "triangleapi.h"
#include "exports.h"
#include "tri.h"
#include "particlemgr.h"
#include "rain.h" 
#include "com_model.h"
#include "studio_util.h"
#define HULL_PLANE_SKIP
#include "particle_header.h"

#define DLLEXPORT __declspec( dllexport )

extern "C"
{
	void DLLEXPORT HUD_DrawNormalTriangles( void );
	void DLLEXPORT HUD_DrawTransparentTriangles( void );
};

extern float g_fFogColor[4];
extern float g_fStartDist;
extern float g_fEndDist;
extern vec3_t FogColor;
extern vec3_t v_origin;
int g_iWaterLevel;

int UseTexture(HL_HSPRITE &hsprSpr, char * str)
{
	if (hsprSpr == 0)
	{
		char sz[256];
		snprintf( sz, 256, str );
		hsprSpr = SPR_Load( sz );
	}

	return gEngfuncs.pTriAPI->SpriteTexture( (struct model_s *)gEngfuncs.GetSpritePointer( hsprSpr ), 0 );
}


//-----------------------------------------------------

void SetPoint( float x, float y, float z, float (*matrix)[4])
{
	vec3_t point, result;
	point[0] = x;
	point[1] = y;
	point[2] = z;

	VectorTransform(point, matrix, result);

	gEngfuncs.pTriAPI->Vertex3f(result[0], result[1], result[2]);
}

//LRC - code for CShinySurface, declared in hud.h
CShinySurface::CShinySurface( float fScale, float fAlpha, float fMinX, float fMaxX, float fMinY, float fMaxY, float fZ, char *szSprite)
{
	m_fScale = fScale; m_fAlpha = fAlpha;
	m_fMinX = fMinX; m_fMinY = fMinY;
	m_fMaxX = fMaxX; m_fMaxY = fMaxY;
	m_fZ = fZ;
	m_hsprSprite = 0;
	snprintf( m_szSprite, 128, szSprite );
	m_pNext = NULL;
}

CShinySurface::~CShinySurface()
{
	if (m_pNext)
		delete m_pNext;
}

void CShinySurface::DrawAll(const vec3_t &org)
{
	gEngfuncs.pTriAPI->RenderMode( kRenderTransAdd ); //kRenderTransTexture );
	gEngfuncs.pTriAPI->CullFace( TRI_NONE );

	for(CShinySurface *pCurrent = this; pCurrent; pCurrent = pCurrent->m_pNext)
	{
		pCurrent->Draw(org);
	}

	gEngfuncs.pTriAPI->RenderMode( kRenderNormal );
}

void CShinySurface::Draw(const vec3_t &org)
{
	// add 5 to the view height, so that we don't get an ugly repeating texture as it approaches 0.
	float fHeight = org.z - m_fZ + 5;

	// only visible from above
//	if (fHeight < 0) return;
	if (fHeight < 5) return;

	// fade out if we're really close to the surface, so they don't see an ugly repeating texture
//	if (fHeight < 15)
//		gEngfuncs.pTriAPI->Color4f( 1.0, 1.0, 1.0, (fHeight - 5)*0.1*m_fAlpha );
//	else
		gEngfuncs.pTriAPI->Color4f( 1.0, 1.0, 1.0, m_fAlpha );

	// check whether the texture is valid
	if (!UseTexture(m_hsprSprite, m_szSprite)) return;

//	CONPRINT("minx %f, maxx %f, miny %f, maxy %f\n", m_fMinX, m_fMaxX, m_fMinY, m_fMaxY);

	float fFactor = 1/(m_fScale*fHeight);
	float fMinTX = (org.x - m_fMinX)*fFactor;
	float fMaxTX = (org.x - m_fMaxX)*fFactor;
	float fMinTY = (org.y - m_fMinY)*fFactor;
	float fMaxTY = (org.y - m_fMaxY)*fFactor;
//	gEngfuncs.pTriAPI->Color4f( 1.0, 1.0, 1.0, m_fAlpha );
	gEngfuncs.pTriAPI->Begin( TRI_QUADS );
		gEngfuncs.pTriAPI->TexCoord2f(	fMinTX,		fMinTY					);
		gEngfuncs.pTriAPI->Vertex3f  (	m_fMinX,	m_fMinY,	m_fZ+0.02	); // add 0.02 to avoid z-buffer problems
		gEngfuncs.pTriAPI->TexCoord2f(	fMinTX,		fMaxTY					);
		gEngfuncs.pTriAPI->Vertex3f  (	m_fMinX,	m_fMaxY,	m_fZ+0.02	);
		gEngfuncs.pTriAPI->TexCoord2f(	fMaxTX,		fMaxTY					);
		gEngfuncs.pTriAPI->Vertex3f  (	m_fMaxX,	m_fMaxY,	m_fZ+0.02	);
		gEngfuncs.pTriAPI->TexCoord2f(	fMaxTX,		fMinTY					);
		gEngfuncs.pTriAPI->Vertex3f  (	m_fMaxX,	m_fMinY,	m_fZ+0.02	);
	gEngfuncs.pTriAPI->End();
}

//
//-----------------------------------------------------
//

void RenderFog()
{
	float g_fFogColor[4] = { FogColor.x, FogColor.y, FogColor.z, 1.0 };
	bool bFog = g_iWaterLevel < 2 && g_fStartDist > 0 && g_fEndDist > 0;
	if (bFog)
	{
		glEnable(GL_FOG);
		//		glFogf(GL_FOG_DENSITY, 0.0025);
		glFogi(GL_FOG_MODE, GL_LINEAR);
		glFogfv(GL_FOG_COLOR, g_fFogColor);
		glFogf(GL_FOG_START, g_fStartDist);
		glFogf(GL_FOG_END, g_fEndDist);
		glHint(GL_FOG_HINT, GL_NICEST);
	}
}

/*
=================================
DrawRain

draw raindrips and snowflakes
=================================
*/
extern cl_drip FirstChainDrip;
extern rain_properties Rain;

void DrawRain()
{
	if (FirstChainDrip.p_Next == NULL)
		return; // no drips to draw

	HL_HSPRITE hsprTexture;
	const model_s *pTexture;
	float visibleHeight = Rain.globalHeight - SNOWFADEDIST;

	if (Rain.weatherMode == 0)
		hsprTexture = LoadSprite("sprites/hi_rain.spr"); // load rain sprite
	else
		hsprTexture = LoadSprite("sprites/snowflake.spr"); // load snow sprite

	if (!hsprTexture) return;

	// usual triapi stuff
	pTexture = gEngfuncs.GetSpritePointer(hsprTexture);
	gEngfuncs.pTriAPI->SpriteTexture((struct model_s *)pTexture, 0);
	gEngfuncs.pTriAPI->RenderMode(kRenderTransAdd);
	gEngfuncs.pTriAPI->CullFace(TRI_NONE);

	// go through drips list
	cl_drip* Drip = FirstChainDrip.p_Next;
	cl_entity_t *player = gEngfuncs.GetLocalPlayer();

	if (Rain.weatherMode == 0) // draw rain
	{
		while (Drip != NULL)
		{
			cl_drip* nextdDrip = Drip->p_Next;

			Vector2D toPlayer;
			toPlayer.x = player->origin[0] - Drip->origin[0];
			toPlayer.y = player->origin[1] - Drip->origin[1];
			toPlayer = toPlayer.Normalize();

			toPlayer.x *= DRIP_SPRITE_HALFWIDTH;
			toPlayer.y *= DRIP_SPRITE_HALFWIDTH;

			float shiftX = (Drip->xDelta / DRIPSPEED) * DRIP_SPRITE_HALFHEIGHT;
			float shiftY = (Drip->yDelta / DRIPSPEED) * DRIP_SPRITE_HALFHEIGHT;

			// --- draw triangle --------------------------
			gEngfuncs.pTriAPI->Color4f(1.0, 1.0, 1.0, Drip->alpha);
			gEngfuncs.pTriAPI->Begin(TRI_TRIANGLES);

			gEngfuncs.pTriAPI->TexCoord2f(0, 0);
			gEngfuncs.pTriAPI->Vertex3f(Drip->origin[0] - toPlayer.y - shiftX, Drip->origin[1] + toPlayer.x - shiftY, Drip->origin[2] + DRIP_SPRITE_HALFHEIGHT);

			gEngfuncs.pTriAPI->TexCoord2f(0.5, 1);
			gEngfuncs.pTriAPI->Vertex3f(Drip->origin[0] + shiftX, Drip->origin[1] + shiftY, Drip->origin[2] - DRIP_SPRITE_HALFHEIGHT);

			gEngfuncs.pTriAPI->TexCoord2f(1, 0);
			gEngfuncs.pTriAPI->Vertex3f(Drip->origin[0] + toPlayer.y - shiftX, Drip->origin[1] - toPlayer.x - shiftY, Drip->origin[2] + DRIP_SPRITE_HALFHEIGHT);

			gEngfuncs.pTriAPI->End();
			// --- draw triangle end ----------------------

			Drip = nextdDrip;
		}
	}

	else	// draw snow
	{
		vec3_t normal;
		gEngfuncs.GetViewAngles((float*)normal);

		float matrix[3][4];
		AngleMatrix(normal, matrix);	// calc view matrix

		while (Drip != NULL)
		{
			cl_drip* nextdDrip = Drip->p_Next;

			matrix[0][3] = Drip->origin[0]; // write origin to matrix
			matrix[1][3] = Drip->origin[1];
			matrix[2][3] = Drip->origin[2];

			// apply start fading effect
			float alpha = (Drip->origin[2] <= visibleHeight) ? Drip->alpha : ((Rain.globalHeight - Drip->origin[2]) / (float)SNOWFADEDIST) * Drip->alpha;

			// --- draw quad --------------------------
			gEngfuncs.pTriAPI->Color4f(1.0, 1.0, 1.0, alpha);
			gEngfuncs.pTriAPI->Begin(TRI_QUADS);

			gEngfuncs.pTriAPI->TexCoord2f(0, 0);
			SetPoint(0, SNOW_SPRITE_HALFSIZE, SNOW_SPRITE_HALFSIZE, matrix);

			gEngfuncs.pTriAPI->TexCoord2f(0, 1);
			SetPoint(0, SNOW_SPRITE_HALFSIZE, -SNOW_SPRITE_HALFSIZE, matrix);

			gEngfuncs.pTriAPI->TexCoord2f(1, 1);
			SetPoint(0, -SNOW_SPRITE_HALFSIZE, -SNOW_SPRITE_HALFSIZE, matrix);

			gEngfuncs.pTriAPI->TexCoord2f(1, 0);
			SetPoint(0, -SNOW_SPRITE_HALFSIZE, SNOW_SPRITE_HALFSIZE, matrix);

			gEngfuncs.pTriAPI->End();
			// --- draw quad end ----------------------

			Drip = nextdDrip;
		}
	}
}

/*
=================================
DrawFXObjects
=================================
*/
extern cl_rainfx FirstChainFX;

void DrawFXObjects( void )
{
	if (FirstChainFX.p_Next == NULL)
		return; // no objects to draw

	float curtime = gEngfuncs.GetClientTime();

	// usual triapi stuff
	HL_HSPRITE hsprTexture;
	const model_s *pTexture;
	hsprTexture = LoadSprite( "sprites/waterring.spr" ); // load water ring sprite
	pTexture = gEngfuncs.GetSpritePointer( hsprTexture );
	gEngfuncs.pTriAPI->SpriteTexture( (struct model_s *)pTexture, 0 );
	gEngfuncs.pTriAPI->RenderMode( kRenderTransAdd );
	gEngfuncs.pTriAPI->CullFace( TRI_NONE );
	
	// go through objects list
	cl_rainfx* curFX = FirstChainFX.p_Next;
	while (curFX != NULL)
	{
		cl_rainfx* nextFX = curFX->p_Next;
					
		// fadeout
		float alpha = ((curFX->birthTime + curFX->life - curtime) / curFX->life) * curFX->alpha;
		float size = (curtime - curFX->birthTime) * MAXRINGHALFSIZE;

		// --- draw quad --------------------------
		gEngfuncs.pTriAPI->Color4f( 1.0, 1.0, 1.0, alpha );
		gEngfuncs.pTriAPI->Begin( TRI_QUADS );

			gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );
			gEngfuncs.pTriAPI->Vertex3f(curFX->origin[0] - size, curFX->origin[1] - size, curFX->origin[2]);

			gEngfuncs.pTriAPI->TexCoord2f( 0, 1 );
			gEngfuncs.pTriAPI->Vertex3f(curFX->origin[0] - size, curFX->origin[1] + size, curFX->origin[2]);

			gEngfuncs.pTriAPI->TexCoord2f( 1, 1 );
			gEngfuncs.pTriAPI->Vertex3f(curFX->origin[0] + size, curFX->origin[1] + size, curFX->origin[2]);

			gEngfuncs.pTriAPI->TexCoord2f( 1, 0 );
			gEngfuncs.pTriAPI->Vertex3f(curFX->origin[0] + size, curFX->origin[1] - size, curFX->origin[2]);
	
		gEngfuncs.pTriAPI->End();
		// --- draw quad end ----------------------

		curFX = nextFX;
	}
}

/*
=================
HUD_DrawNormalTriangles

Non-transparent triangles-- add them here
=================
*/
void CL_DLLEXPORT HUD_DrawNormalTriangles( void )
{
	gHUD.m_Spectator.DrawOverview();
}

/*
=================
HUD_DrawTransparentTriangles

Render any triangles with transparent rendermode needs here
=================
*/
extern ParticleSystemManager* g_pParticleSystems; // LRC
class CException;

void CL_DLLEXPORT HUD_DrawTransparentTriangles( void )
{
	try {
		pParticleManager->UpdateSystems();
	} catch (CException *e) {
		CONPRINT("There was a serious error within the particle engine. Particles will return on map change\n");
		delete pParticleManager;
		pParticleManager = NULL;
	}

   	//22/03/03 LRC: shiny surfaces
	if (gHUD.m_pShinySurface)
		gHUD.m_pShinySurface->DrawAll(v_origin);

	// LRC: find out the time elapsed since the last redraw
	static float fOldTime, fTime;
	fOldTime = fTime;
	fTime = gEngfuncs.GetClientTime();

	// LRC: draw and update particle systems
	g_pParticleSystems->UpdateSystems(fTime - fOldTime);

	ProcessFXObjects();
	ProcessRain();
	DrawRain();
	DrawFXObjects();

	if ( g_pParticleMan )
		 g_pParticleMan->Update();
}
