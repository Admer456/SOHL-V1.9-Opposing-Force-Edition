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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "monsters.h"
#include "weapons.h"
#include "effects.h"
#include "customentity.h"
#include "weapon_egon.h"

extern bool gInfinitelyAmmo;

#define EGON_PRIMARY_VOLUME		450
#define EGON_BEAM_SPRITE		"sprites/xbeam1.spr"
#define EGON_FLARE_SPRITE		"sprites/XSpark1.spr"
#define EGON_SOUND_OFF		"weapons/egon_off1.wav"
#define EGON_SOUND_RUN		"weapons/egon_run3.wav"
#define EGON_SOUND_STARTUP		"weapons/egon_windup2.wav"

#define EGON_SWITCH_NARROW_TIME		0.75			// Time it takes to switch fire modes
#define EGON_SWITCH_WIDE_TIME			1.5
#define PRIMARY_CHARGE_VOLUME			256
#define PRIMARY_FIRE_VOLUME			450

LINK_ENTITY_TO_CLASS(weapon_egon, CEgon);

TYPEDESCRIPTION	CEgon::m_SaveData[] =
{
	DEFINE_FIELD(CEgon, m_pBeam, FIELD_CLASSPTR),
	DEFINE_FIELD(CEgon, m_pNoise, FIELD_CLASSPTR),
	DEFINE_FIELD(CEgon, m_pSprite, FIELD_CLASSPTR),
	DEFINE_FIELD(CEgon, m_shootTime, FIELD_TIME),
	DEFINE_FIELD(CEgon, m_fireState, FIELD_INTEGER),
	DEFINE_FIELD(CEgon, m_fireMode, FIELD_INTEGER),
	DEFINE_FIELD(CEgon, m_shakeTime, FIELD_TIME),
	DEFINE_FIELD(CEgon, m_flAmmoUseTime, FIELD_TIME),
};
IMPLEMENT_SAVERESTORE(CEgon, CBasePlayerWeapon);

void CEgon::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_egon.mdl");

	m_iDefaultAmmo = EGON_DEFAULT_GIVE;
	m_fireMode = FIRE_WIDE;//place it here
	FallInit();// get ready to fall down.
}

void CEgon::Precache()
{
	PRECACHE_MODEL("models/w_egon.mdl");
	PRECACHE_MODEL("models/v_egon.mdl");
	PRECACHE_MODEL("models/p_egon.mdl");
	PRECACHE_MODEL(EGON_BEAM_SPRITE);
	PRECACHE_MODEL(EGON_FLARE_SPRITE);

	m_usEgonFire = PRECACHE_EVENT(1, "events/egon_fire.sc");
	m_usEgonStop = PRECACHE_EVENT(1, "events/egon_stop.sc");
}

BOOL CEgon::Deploy()
{
	m_fireState = FIRE_OFF;

	AnimRestore = TRUE;
	return DefaultDeploy("models/v_egon.mdl", "models/p_egon.mdl", (int)EGON_DRAW::sequence, "egon", 0.7);
}

void CEgon::Holster()
{
	if (m_fireState != FIRE_OFF) EndAttack();

	ShutdownScreen();
	m_pPlayer->m_flNextAttack = UTIL_GlobalTimeBase() + 0.5;
	SendWeaponAnim((int)EGON_HOLSTER::sequence);
}

int CEgon::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "uranium";
	p->iMaxAmmo1 = URANIUM_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 3;
	p->iPosition = 2;
	p->iId = m_iId = WEAPON_EGON;
	p->iFlags = 0;
	p->iWeight = EGON_WEIGHT;

	return 1;
}

#define EGON_PULSE_INTERVAL			0.1
#define EGON_DISCHARGE_INTERVAL		0.1

BOOL CEgon::HasAmmo()
{
	if (gInfinitelyAmmo)
		return TRUE;

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0) return FALSE;
	return TRUE;
}

void CEgon::UseAmmo(int count)
{
	if (!gInfinitelyAmmo) {
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] >= count)
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= count;
		else	
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = 0;
	}
}

void CEgon::PrimaryAttack()
{
	UpdateScreen();

	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3) {
		if (m_fireState != FIRE_OFF || m_pBeam) EndAttack();
		else PlayEmptySound(2);

		m_flNextPrimaryAttack = UTIL_GlobalTimeBase() + 0.08;
		return;
	}

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	Vector vecAiming = gpGlobals->v_forward;
	Vector vecSrc = m_pPlayer->GetGunPosition();

	switch (m_fireState)
	{
	case FIRE_OFF:
	{
		if (!HasAmmo())
		{
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_GlobalTimeBase() + 0.08;
			PlayEmptySound(1);
			return;
		}

		m_flAmmoUseTime = UTIL_GlobalTimeBase();// start using ammo ASAP.

		PLAYBACK_EVENT_FULL(flags(), m_pPlayer->edict(), m_usEgonFire, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, pev->body, m_fireMode, 1, m_fireState);
		SendWeaponAnim((int)EGON_FIRECYCLE::sequence);
		m_shakeTime = 0;

		m_pPlayer->m_iWeaponVolume = PRIMARY_FIRE_VOLUME;
		m_flTimeWeaponIdle = UTIL_GlobalTimeBase() + 0.1;
		m_shootTime = UTIL_GlobalTimeBase() + 2;

		pev->dmgtime = UTIL_GlobalTimeBase() + EGON_PULSE_INTERVAL;
		m_fireState = FIRE_CHARGE;
	}
	break;

	case FIRE_CHARGE:
	{
		Fire(vecSrc, vecAiming);
		m_pPlayer->m_iWeaponVolume = PRIMARY_FIRE_VOLUME;

		if (m_shootTime != 0 && UTIL_GlobalTimeBase() > m_shootTime)
		{
			PLAYBACK_EVENT_FULL(flags(), m_pPlayer->edict(), m_usEgonFire, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, pev->body, m_fireMode, 0, m_fireState);
			SendWeaponAnim((int)EGON_FIRECYCLE::sequence);
			m_shootTime = 0;
		}
		if (!AnimRestore)
		{
			PLAYBACK_EVENT_FULL(flags(), m_pPlayer->edict(), m_usEgonFire, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, pev->body, m_fireMode, 0, m_fireState);
			SendWeaponAnim((int)EGON_FIRECYCLE::sequence);
			m_shootTime = 0;
			m_fireMode = (EGON_FIREMODE)m_iBody;
			AnimRestore = TRUE;
		}
		if (!HasAmmo())
		{
			EndAttack();
			m_fireState = FIRE_OFF;
			m_flNextSecondaryAttack = UTIL_GlobalTimeBase() + 1.0;
		}

	}
	break;
	}
}

void CEgon::SecondaryAttack()
{
	UpdateScreen();
	if (m_fireState != FIRE_OFF) EndAttack();
	if (m_iBody == 0)
	{
		SendWeaponAnim((int)EGON_ALTFIREON::sequence);
		m_fireMode = FIRE_NARROW;
		m_iBody = 1;
		m_flTimeUpdate = UTIL_GlobalTimeBase() + 1.1;
	}
	else
	{
		m_iBody = 0;
		SendWeaponAnim((int)EGON_ALTFIREOFF::sequence);
		m_fireMode = FIRE_WIDE;
		m_flTimeUpdate = UTIL_GlobalTimeBase() + 0.7;//off animation is faster than on
	}

	m_flTimeWeaponIdle = UTIL_GlobalTimeBase() + 3;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_GlobalTimeBase() + 1.5;
}

void CEgon::Fire(const Vector &vecOrigSrc, const Vector &vecDir)
{
	Vector vecDest = vecOrigSrc + vecDir * 2048;
	edict_t		*pentIgnore;
	TraceResult tr;

	pentIgnore = m_pPlayer->edict();
	Vector tmpSrc = vecOrigSrc + gpGlobals->v_up * -8 + gpGlobals->v_right * 3;

	UTIL_TraceLine(vecOrigSrc, vecDest, dont_ignore_monsters, pentIgnore, &tr);

	if (tr.fAllSolid) return;

	CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);

	if (pEntity == NULL) return;

	if (IsMultiplayer())
	{
		if (m_pSprite && pEntity->pev->takedamage)
			m_pSprite->pev->effects &= ~EF_NODRAW;
		else if (m_pSprite)
			m_pSprite->pev->effects |= EF_NODRAW;
	}

	float timedist;

	switch (m_fireMode)
	{
	case FIRE_NARROW:
		if (pev->dmgtime < UTIL_GlobalTimeBase())
		{
			// Narrow mode only does damage to the entity it hits
			ClearMultiDamage();
			if (pEntity->pev->takedamage)
			{
				pEntity->TraceAttack(m_pPlayer->pev, gSkillData.plrDmgEgonNarrow, vecDir, &tr, DMG_ENERGYBEAM);
			}
			ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

			if (IsMultiplayer())
			{
				// multiplayer uses 1 ammo every 1/10th second
				if (UTIL_GlobalTimeBase() >= m_flAmmoUseTime)
				{
					UseAmmo(1);
					m_flAmmoUseTime = UTIL_GlobalTimeBase() + 0.1;
				}
			}
			else
			{
				// single player, use 3 ammo/second
				if (UTIL_GlobalTimeBase() >= m_flAmmoUseTime)
				{
					UseAmmo(1);
					m_flAmmoUseTime = UTIL_GlobalTimeBase() + 0.2;
				}
			}

			pev->dmgtime = UTIL_GlobalTimeBase() + EGON_PULSE_INTERVAL;
		}
		timedist = (pev->dmgtime - UTIL_GlobalTimeBase()) / EGON_PULSE_INTERVAL;
		break;

	case FIRE_WIDE:
		if (pev->dmgtime < UTIL_GlobalTimeBase())
		{
			// wide mode does damage to the ent, and radius damage
			ClearMultiDamage();
			if (pEntity->pev->takedamage)
			{
				pEntity->TraceAttack(m_pPlayer->pev, gSkillData.plrDmgEgonWide, vecDir, &tr, DMG_ENERGYBEAM | DMG_ALWAYSGIB);
			}
			ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

			if (IsMultiplayer())
			{
				// radius damage a little more potent in multiplayer.
				::RadiusDamage(tr.vecEndPos, pev, m_pPlayer->pev, gSkillData.plrDmgEgonWide / 4, 128, CLASS_NONE, DMG_ENERGYBEAM | DMG_BLAST | DMG_ALWAYSGIB);
			}

			if (!m_pPlayer->IsAlive()) return;

			if (IsMultiplayer())
			{
				//multiplayer uses 5 ammo/second
				if (UTIL_GlobalTimeBase() >= m_flAmmoUseTime)
				{
					UseAmmo(1);
					m_flAmmoUseTime = UTIL_GlobalTimeBase() + 0.2;
				}
			}
			else
			{
				// Wide mode uses 10 charges per second in single player
				if (UTIL_GlobalTimeBase() >= m_flAmmoUseTime)
				{
					UseAmmo(1);
					m_flAmmoUseTime = UTIL_GlobalTimeBase() + 0.1;
				}
			}

			pev->dmgtime = UTIL_GlobalTimeBase() + EGON_DISCHARGE_INTERVAL;
			if (m_shakeTime < UTIL_GlobalTimeBase())
			{
				UTIL_ScreenShake(tr.vecEndPos, 5.0, 150.0, 0.75, 250.0);
				m_shakeTime = UTIL_GlobalTimeBase() + 1.5;
			}
		}
		timedist = (pev->dmgtime - UTIL_GlobalTimeBase()) / EGON_DISCHARGE_INTERVAL;
		break;
	}

	if (timedist < 0)      timedist = 0;
	else if (timedist > 1) timedist = 1;
	timedist = 1 - timedist;

	UpdateEffect(tmpSrc, tr.vecEndPos, timedist);
	UpdateScreen();
}


void CEgon::UpdateEffect(const Vector &startPoint, const Vector &endPoint, float timeBlend)
{
	if (!m_pBeam) CreateEffect();

	m_pBeam->SetStartPos(endPoint);
	m_pBeam->SetBrightness(255 - (timeBlend * 180));
	m_pBeam->SetWidth(40 - (timeBlend * 20));

	if (m_fireMode == FIRE_WIDE)
		m_pBeam->SetColor(30 + (25 * timeBlend), 30 + (30 * timeBlend), 64 + 80 * fabs(sin(UTIL_GlobalTimeBase() * 10)));
	else
		m_pBeam->SetColor(60 + (25 * timeBlend), 120 + (30 * timeBlend), 64 + 80 * fabs(sin(UTIL_GlobalTimeBase() * 10)));

	UTIL_SetOrigin(m_pSprite, endPoint);
	m_pSprite->pev->frame += 8 * gpGlobals->frametime;

	if (m_pSprite->pev->frame > m_pSprite->Frames()) m_pSprite->pev->frame = 0;
	m_pNoise->SetStartPos(endPoint);
}

void CEgon::CreateEffect()
{
	DestroyEffect();

	m_pBeam = CBeam::BeamCreate(EGON_BEAM_SPRITE, 40);
	m_pBeam->PointEntInit(pev->origin, m_pPlayer->entindex());
	m_pBeam->SetFlags(BEAM_FSINE);
	m_pBeam->SetEndAttachment(1);
	m_pBeam->pev->spawnflags |= SF_BEAM_TEMPORARY;// Flag these to be destroyed on save/restore or level transition
	m_pBeam->pev->flags |= FL_SKIPLOCALHOST;
	m_pBeam->pev->owner = m_pPlayer->edict();

	m_pNoise = CBeam::BeamCreate(EGON_BEAM_SPRITE, 55);
	m_pNoise->PointEntInit(pev->origin, m_pPlayer->entindex());
	m_pNoise->SetScrollRate(25);
	m_pNoise->SetBrightness(100);
	m_pNoise->SetEndAttachment(1);
	m_pNoise->pev->spawnflags |= SF_BEAM_TEMPORARY;
	m_pNoise->pev->flags |= FL_SKIPLOCALHOST;
	m_pNoise->pev->owner = m_pPlayer->edict();

	m_pSprite = CSprite::SpriteCreate(EGON_FLARE_SPRITE, pev->origin, FALSE);
	m_pSprite->pev->scale = 1.0;
	m_pSprite->SetTransparency(kRenderGlow, 255, 255, 255, 255, kRenderFxNoDissipation);
	m_pSprite->pev->spawnflags |= SF_SPRITE_TEMPORARY;
	m_pSprite->pev->flags |= FL_SKIPLOCALHOST;
	m_pSprite->pev->owner = m_pPlayer->edict();

	if (m_fireMode == FIRE_WIDE)
	{
		m_pBeam->SetScrollRate(50);
		m_pBeam->SetNoise(20);
		m_pNoise->SetColor(50, 50, 255);
		m_pNoise->SetNoise(8);
	}
	else
	{
		m_pBeam->SetScrollRate(110);
		m_pBeam->SetNoise(5);
		m_pNoise->SetColor(80, 120, 255);
		m_pNoise->SetNoise(2);
	}

}


void CEgon::DestroyEffect()
{
	if (m_pBeam)
	{
		UTIL_Remove(m_pBeam);
		m_pBeam = NULL;
	}
	if (m_pNoise)
	{
		UTIL_Remove(m_pNoise);
		m_pNoise = NULL;
	}
	if (m_pSprite)
	{
		if (m_fireMode == FIRE_WIDE)
			m_pSprite->Expand(10, 500);
		else
			UTIL_Remove(m_pSprite);
		m_pSprite = NULL;
	}
}

void CEgon::EndAttack()
{
	PLAYBACK_EVENT_FULL(FEV_GLOBAL | FEV_RELIABLE, m_pPlayer->edict(), m_usEgonStop, 0, (float *)&m_pPlayer->pev->origin, (float *)&m_pPlayer->pev->angles, 0.0, 0.0, m_fireState, pev->body, 0, 0);
	m_fireState = FIRE_OFF;

	m_flTimeWeaponIdle = UTIL_GlobalTimeBase() + 2.0;
	m_flNextSecondaryAttack = UTIL_GlobalTimeBase() + 0.5;

	DestroyEffect();
}

void CEgon::UpdateScreen()
{
	if (m_flTimeUpdate > UTIL_GlobalTimeBase()) return;

	if (HasAmmo() && m_fireState == FIRE_CHARGE)
	{
		if (m_fireMode == FIRE_NARROW)
		{
			if (pev->skin < 9) pev->skin++;
			else pev->skin = RANDOM_LONG(9, 13);
		}
		else
		{
			if (pev->skin < 3) pev->skin++;
			else pev->skin = RANDOM_LONG(3, 5);
		}
	}
	else
	{
		if (m_fireMode == FIRE_NARROW)
		{
			if (pev->skin > 6)pev->skin--;
			else		pev->skin = 6;
		}
		else
		{
			if (pev->skin > 0 && pev->skin != 6)pev->skin--;
			else pev->skin = 0;
		}
	}
	MESSAGE_BEGIN(MSG_ONE, gmsgSetSkin, NULL, m_pPlayer->pev);
	WRITE_BYTE(pev->skin); //weaponmodel skin.
	MESSAGE_END();

	m_flTimeUpdate = UTIL_GlobalTimeBase() + 0.08;//refresh rate
}

void CEgon::ShutdownScreen()
{
	//shutdown screen
	if (m_fireMode == FIRE_NARROW) pev->skin = 6;
	else pev->skin = 0;

	MESSAGE_BEGIN(MSG_ONE, gmsgSetSkin, NULL, m_pPlayer->pev);
	WRITE_BYTE(pev->skin); //weaponmodel skin.
	MESSAGE_END();
}

void CEgon::WeaponIdle()
{
	UpdateScreen();

	if (m_flTimeWeaponIdle > UTIL_GlobalTimeBase()) return;
	if (m_fireState != FIRE_OFF) EndAttack();

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
	{
		int iAnim;
		float flRand = RANDOM_FLOAT(0, 1);

		if (flRand > 0.8)
		{
			iAnim = (int)EGON_IDLE1::sequence;
			m_flTimeWeaponIdle = UTIL_GlobalTimeBase() + RANDOM_FLOAT(10, 15);
		}
		else if ((m_iBody == 0) && (flRand < 0.2))
		{
			iAnim = (int)EGON_FIDGET1::sequence;
			m_flTimeWeaponIdle = UTIL_GlobalTimeBase() + 3;
		}
		SendWeaponAnim(iAnim);
	}
}