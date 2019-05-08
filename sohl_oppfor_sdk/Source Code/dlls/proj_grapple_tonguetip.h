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

#ifndef GRAPPLE_TONGUETIP_H
#define GRAPPLE_TONGUETIP_H

class CGrappleHook : public CBaseEntity
{
public:
	void Spawn(void);
	void Precache(void);
	void EXPORT Move(void);
	void EXPORT Hit(CBaseEntity*);
	void Killed(entvars_t *pev, int gib);

	static	CGrappleHook* Create(Vector Pos, Vector Aim, CBasePlayer* Owner);

	int		m_Chain;
	int		m_iIsMoving;
	int		m_iTrail_Length;
	int		m_iHitMonster;	// Fograin92: Used to handle what monster type we did hit
	BOOL	bPullBack;		// Fograin92: Used to "pull-back" tongue after miss or release

	CBasePlayer *myowner;
	CBaseEntity *myHitMonster;	// Fograin92: Pointer to our monster
	CBeam		*m_pTongue;		// Fograin92: New tongue
};

#endif // GRAPPLE_TONGUETIP_H