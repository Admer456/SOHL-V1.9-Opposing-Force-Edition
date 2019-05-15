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
#include "monsters.h"
#include "weapons.h"
#include "player.h"
#include "effects.h"
#include "proj_grenade.h"

extern bool gInfinitelyAmmo;

#define	TRIPMINE_PRIMARY_VOLUME		450

enum tripmine_e {
	TRIPMINE_IDLE1 = 0,
	TRIPMINE_IDLE2,
	TRIPMINE_ARM1,
	TRIPMINE_ARM2,
	TRIPMINE_FIDGET,
	TRIPMINE_HOLSTER,
	TRIPMINE_DRAW,
};

class CTripmineGrenade : public CGrenade
{
	void Spawn();
	void Precache();

	virtual int		Save(CSave &save);
	virtual int		Restore(CRestore &restore);

	static	TYPEDESCRIPTION m_SaveData[];

	int TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType);

	void DLLEXPORT WarningThink();
	void DLLEXPORT PowerupThink();
	void DLLEXPORT BeamBreakThink();
	void DLLEXPORT DelayDeathThink();
	void Killed(entvars_t *pevAttacker, int iGib);

	void MakeBeam();
	void KillBeam();

	float		m_flPowerUp;
	Vector		m_vecDir;
	Vector		m_vecEnd;
	float		m_flBeamLength;

	EHANDLE		m_hOwner;
	CBeam		*m_pBeam;
	CBeam		*m_pMirBeam;
	Vector		m_posOwner;
	Vector		m_angleOwner;
	edict_t		*m_pRealOwner;// tracelines don't hit PEV->OWNER, which means a player couldn't detonate his own trip mine, so we store the owner here.
};

class CTripmine : public CBasePlayerWeapon
{
public:
	void Spawn();
	void Precache();
	int GetItemInfo(ItemInfo *p);
	void PrimaryAttack();
	BOOL Deploy();
	void Holster();
	void WeaponIdle();
};

LINK_ENTITY_TO_CLASS(monster_tripmine, CTripmineGrenade);

TYPEDESCRIPTION	CTripmineGrenade::m_SaveData[] =
{
	DEFINE_FIELD(CTripmineGrenade, m_flPowerUp, FIELD_TIME),
	DEFINE_FIELD(CTripmineGrenade, m_vecDir, FIELD_VECTOR),
	DEFINE_FIELD(CTripmineGrenade, m_vecEnd, FIELD_POSITION_VECTOR),
	DEFINE_FIELD(CTripmineGrenade, m_flBeamLength, FIELD_FLOAT),
	DEFINE_FIELD(CTripmineGrenade, m_hOwner, FIELD_EHANDLE),
	DEFINE_FIELD(CTripmineGrenade, m_pBeam, FIELD_CLASSPTR),
	DEFINE_FIELD(CTripmineGrenade, m_pMirBeam, FIELD_CLASSPTR),
	DEFINE_FIELD(CTripmineGrenade, m_posOwner, FIELD_POSITION_VECTOR),
	DEFINE_FIELD(CTripmineGrenade, m_angleOwner, FIELD_VECTOR),
	DEFINE_FIELD(CTripmineGrenade, m_pRealOwner, FIELD_EDICT),
};

IMPLEMENT_SAVERESTORE(CTripmineGrenade, CGrenade);


void CTripmineGrenade::Spawn()
{
	Precache();
	// motor
	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_NOT;

	SET_MODEL(ENT(pev), "models/w_tripmine.mdl");

	UTIL_AutoSetSize();
	UTIL_SetOrigin(this, pev->origin);

	if (pev->spawnflags & 1)
	{
		// power up quickly
		m_flPowerUp = UTIL_GlobalTimeBase() + 1.0;
	}
	else
	{
		// power up in 2.5 seconds
		m_flPowerUp = UTIL_GlobalTimeBase() + 2.5;
	}

	SetThink(&CTripmineGrenade::PowerupThink);
	SetNextThink(0.2);

	pev->takedamage = DAMAGE_YES;
	pev->dmg = gSkillData.plrDmgTripmine;
	pev->health = 1; // don't let die normally

	if (pev->owner != NULL)
	{
		// play deploy sound
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/mine_deploy.wav", VOL_NORM, ATTN_NORM);
		EMIT_SOUND(ENT(pev), CHAN_BODY, "weapons/mine_charge.wav", VOL_LOWER, ATTN_NORM); // chargeup

		m_pRealOwner = pev->owner;// see CTripmineGrenade for why.
	}

	UTIL_MakeAimVectors(pev->angles);

	m_vecDir = gpGlobals->v_forward;
	m_vecEnd = pev->origin + m_vecDir * 8192; //make beam across level
}


void CTripmineGrenade::Precache()
{
	PRECACHE_MODEL("models/w_tripmine.mdl");
	PRECACHE_SOUND("weapons/mine_deploy.wav");
	PRECACHE_SOUND("weapons/mine_activate.wav");
	PRECACHE_SOUND("weapons/mine_charge.wav");
}


void CTripmineGrenade::WarningThink()
{
	// play warning sound
	EMIT_SOUND(ENT(pev), CHAN_VOICE, "buttons/Blip2.wav", VOL_NORM, ATTN_NORM);

	// set to power up
	SetThink(&CTripmineGrenade::PowerupThink);
	SetNextThink(1.0);
}


void CTripmineGrenade::PowerupThink()
{
	TraceResult tr;

	if (m_hOwner == NULL)
	{
		// find an owner
		edict_t *oldowner = pev->owner;
		pev->owner = NULL;
		UTIL_TraceLine(pev->origin + m_vecDir * 8, pev->origin - m_vecDir * 32, dont_ignore_monsters, ENT(pev), &tr);
		if (tr.fStartSolid || (oldowner && tr.pHit == oldowner))
		{
			pev->owner = oldowner;
			m_flPowerUp += 0.1;
			SetNextThink(0.1);
			return;
		}
		if (tr.flFraction < 1.0)
		{
			pev->owner = tr.pHit;
			m_hOwner = CBaseEntity::Instance(pev->owner);
			m_posOwner = m_hOwner->pev->origin;
			m_angleOwner = m_hOwner->pev->angles;
		}
		else
		{
			STOP_SOUND(ENT(pev), CHAN_VOICE, "weapons/mine_deploy.wav");
			STOP_SOUND(ENT(pev), CHAN_BODY, "weapons/mine_charge.wav");
			SetThink(&CTripmineGrenade::SUB_Remove);
			SetNextThink(0.1);
			ALERT(at_debug, "WARNING:Tripmine at %.0f, %.0f, %.0f removed\n", pev->origin.x, pev->origin.y, pev->origin.z);
			KillBeam();
			return;
		}
	}
	else if (m_posOwner != m_hOwner->pev->origin || m_angleOwner != m_hOwner->pev->angles)
	{
		// disable
		STOP_SOUND(ENT(pev), CHAN_VOICE, "weapons/mine_deploy.wav");
		STOP_SOUND(ENT(pev), CHAN_BODY, "weapons/mine_charge.wav");
		CBaseEntity *pMine = Create("weapon_tripmine", pev->origin + m_vecDir * 24, pev->angles);
		pMine->pev->spawnflags |= SF_NORESPAWN;

		SetThink(&CTripmineGrenade::SUB_Remove);
		KillBeam();
		SetNextThink(0.1);
		return;
	}
	if (UTIL_GlobalTimeBase() > m_flPowerUp)
	{
		// make solid
		pev->solid = SOLID_BBOX;
		UTIL_SetOrigin(this, pev->origin);

		MakeBeam();

		// play enabled sound
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "weapons/mine_activate.wav", VOL_LOWER, ATTN_NORM, 1.0, 75);
	}
	SetNextThink(0.1);
}


void CTripmineGrenade::KillBeam()
{
	if (m_pBeam)
	{
		UTIL_Remove(m_pBeam);
		m_pBeam = NULL;
	}
	if (m_pMirBeam)
	{
		UTIL_Remove(m_pMirBeam);
		m_pMirBeam = NULL;
	}
}


void CTripmineGrenade::MakeBeam()
{
	TraceResult tr;
	Vector mirpos, mirend;

	UTIL_TraceLine(pev->origin, m_vecEnd, dont_ignore_monsters, ENT(pev), &tr);

	m_flBeamLength = tr.flFraction;

	// set to follow laser spot
	SetThink(&CTripmineGrenade::BeamBreakThink);
	SetNextThink(0.1);

	Vector vecTmpEnd = pev->origin + m_vecDir * 8192 * m_flBeamLength;

	m_pBeam = CBeam::BeamCreate(g_pModelNameLaser, 5);
	m_pBeam->PointsInit(vecTmpEnd + gpGlobals->v_up * 1.4, pev->origin + gpGlobals->v_up * 1.4);
	m_pBeam->SetColor(0, 214, 198);
	m_pBeam->SetScrollRate(255);
	m_pBeam->SetBrightness(50);

	mirpos = UTIL_MirrorPos(vecTmpEnd + gpGlobals->v_up * 1.4);
	mirend = UTIL_MirrorPos(pev->origin + gpGlobals->v_up * 1.4);
	if (mirpos != mirend)
	{
		m_pMirBeam = CBeam::BeamCreate(g_pModelNameLaser, 5);
		m_pMirBeam->PointsInit(mirpos, mirend);
		m_pMirBeam->SetColor(0, 214, 198);
		m_pMirBeam->SetScrollRate(255);
		m_pMirBeam->SetBrightness(50);
	}
}


void CTripmineGrenade::BeamBreakThink()
{
	BOOL bBlowup = 0;

	TraceResult tr;

	// HACKHACK Set simple box using this really nice global!
	gpGlobals->trace_flags = FTRACE_SIMPLEBOX;
	UTIL_TraceLine(pev->origin, m_vecEnd, dont_ignore_monsters, ENT(pev), &tr);

	// ALERT( at_console, "%f : %f\n", tr.flFraction, m_flBeamLength );

	// respawn detect. 
	if (!m_pBeam)
	{
		MakeBeam();
		if (tr.pHit)
			m_hOwner = CBaseEntity::Instance(tr.pHit);	// reset owner too
	}

	if (fabs(m_flBeamLength - tr.flFraction) > 0.001f)
	{
		bBlowup = 1;
	}
	else
	{
		if (m_hOwner == NULL) bBlowup = 1;
		else if (m_posOwner != m_hOwner->pev->origin) bBlowup = 1;
		else if (m_angleOwner != m_hOwner->pev->angles) bBlowup = 1;
	}

	if (bBlowup)
	{
		// a bit of a hack, but all CGrenade code passes pev->owner along to make sure the proper player gets credit for the kill
		// so we have to restore pev->owner from pRealOwner, because an entity's tracelines don't strike it's pev->owner which meant
		// that a player couldn't trigger his own tripmine. Now that the mine is exploding, it's safe the restore the owner so the 
		// CGrenade code knows who the explosive really belongs to.
		pev->owner = m_pRealOwner;
		pev->health = 0;
		Killed(VARS(pev->owner), GIB_NORMAL);
		return;
	}

	SetNextThink(0.1);
}

int CTripmineGrenade::TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType)
{
	if (UTIL_GlobalTimeBase() < m_flPowerUp && flDamage < pev->health)
	{
		// disable
		// Create( "weapon_tripmine", pev->origin + m_vecDir * 24, pev->angles );
		SetThink(&CTripmineGrenade::SUB_Remove);
		SetNextThink(0.1);
		KillBeam();
		return FALSE;
	}
	return CGrenade::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

void CTripmineGrenade::Killed(entvars_t *pevAttacker, int iGib)
{
	pev->takedamage = DAMAGE_NO;

	if (pevAttacker && (pevAttacker->flags & FL_CLIENT))
	{
		// some client has destroyed this mine, he'll get credit for any kills
		pev->owner = ENT(pevAttacker);
	}

	SetThink(&CTripmineGrenade::DelayDeathThink);
	SetNextThink(RANDOM_FLOAT(0.1, 0.3));

	EMIT_SOUND(ENT(pev), CHAN_BODY, "common/null.wav", VOL_NORM, ATTN_NORM); // shut off chargeup
}


void CTripmineGrenade::DelayDeathThink()
{
	KillBeam();
	TraceResult tr;
	UTIL_TraceLine(pev->origin + m_vecDir * 8, pev->origin - m_vecDir * 64, dont_ignore_monsters, ENT(pev), &tr);

	Explode(&tr, DMG_BLAST);
}
LINK_ENTITY_TO_CLASS(weapon_tripmine, CTripmine);

void CTripmine::Spawn()
{
	Precache();
	m_iId = WEAPON_TRIPMINE;
	SET_MODEL(ENT(pev), "models/w_tripmine.mdl");
	FallInit();// get ready to fall down

	m_iDefaultAmmo = TRIPMINE_DEFAULT_GIVE;
}

void CTripmine::Precache()
{
	PRECACHE_MODEL("models/v_tripmine.mdl");
	PRECACHE_MODEL("models/p_tripmine.mdl");
	UTIL_PrecacheOther("monster_tripmine");
}

int CTripmine::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "Trip Mine";
	p->iMaxAmmo1 = TRIPMINE_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 4;
	p->iPosition = 2;
	p->iId = m_iId = WEAPON_TRIPMINE;
	p->iWeight = TRIPMINE_WEIGHT;
	p->iFlags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;

	return 1;
}

BOOL CTripmine::Deploy()
{
	m_iBody = 0;//show tripmine
	return DefaultDeploy("models/v_tripmine.mdl", "models/p_tripmine.mdl", TRIPMINE_DRAW, "trip");
}


void CTripmine::Holster()
{
	//don't play holster animation if ammo is out
	if (m_iBody)m_pPlayer->m_flNextAttack = UTIL_GlobalTimeBase();
	else m_pPlayer->m_flNextAttack = UTIL_GlobalTimeBase() + 0.5;

	if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		// out of mines
		m_pPlayer->pev->weapons &= ~(1 << WEAPON_TRIPMINE);
		SetThink(&CTripmine::DestroyItem);
		SetNextThink(0.1);
	}

	SendWeaponAnim(TRIPMINE_HOLSTER);
	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "common/null.wav", VOL_NORM, ATTN_NORM);
}

void CTripmine::PrimaryAttack()
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0) return;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = gpGlobals->v_forward;

	TraceResult tr;

	UTIL_TraceLine(vecSrc, vecSrc + vecAiming * 45, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

	if (tr.flFraction < 1.0)
	{
		CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);
		if (pEntity && !(pEntity->pev->flags & FL_CONVEYOR))
		{
			if(!gInfinitelyAmmo)
				m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;

			// player "shoot" animation
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);

			m_iBody = 1;
			SendWeaponAnim(TRIPMINE_ARM2);

			Vector angles = UTIL_VecToAngles(tr.vecPlaneNormal);
			CBaseEntity *pEnt = CBaseEntity::Create("monster_tripmine", tr.vecEndPos + tr.vecPlaneNormal * 5 + gpGlobals->v_up * -6, angles, m_pPlayer->edict());
			CTripmineGrenade *pMine = (CTripmineGrenade *)pEnt;
		}
	}

	m_flNextPrimaryAttack = UTIL_GlobalTimeBase() + 0.3;
	m_flTimeWeaponIdle = UTIL_GlobalTimeBase() + RANDOM_FLOAT(10, 15);
	m_flTimeUpdate = UTIL_GlobalTimeBase() + RANDOM_FLOAT(0.5, 1.0); //time to deploy next tripmine
}

void CTripmine::WeaponIdle()
{
	if (m_flTimeUpdate < UTIL_GlobalTimeBase() && m_iBody)
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
		{
			m_iBody = 0;
			SendWeaponAnim(TRIPMINE_DRAW);
		}
		else
		{
			RetireWeapon();
			return;
		}
	}

	if (m_flTimeWeaponIdle > UTIL_GlobalTimeBase())return;
	int iAnim;
	float flRand = RANDOM_FLOAT(0, 1);
	if (flRand <= 0.25)
	{
		iAnim = TRIPMINE_IDLE1;
		m_flTimeWeaponIdle = UTIL_GlobalTimeBase() + 90.0 / 30.0;
	}
	else if (flRand <= 0.75)
	{
		iAnim = TRIPMINE_IDLE2;
		m_flTimeWeaponIdle = UTIL_GlobalTimeBase() + 60.0 / 30.0;
	}
	else if (flRand <= 0.85)
	{
		iAnim = TRIPMINE_FIDGET;
		m_flTimeWeaponIdle = UTIL_GlobalTimeBase() + 100.0 / 30.0;
	}
	else
	{
		iAnim = TRIPMINE_ARM1;
		m_flTimeWeaponIdle = UTIL_GlobalTimeBase() + 45.0 / 15.0;
	}
	SendWeaponAnim(iAnim);
}