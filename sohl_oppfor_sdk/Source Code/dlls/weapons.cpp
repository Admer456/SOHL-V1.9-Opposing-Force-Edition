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

/*

===== weapons.cpp ========================================================
  functions governing the selection/use of weapons for players
*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "monsters.h"
#include "weapons.h"
#include "decals.h"
#include "gamerules.h"

//extern CGraph	WorldGraph;
extern bool gEvilImpulse101;
extern bool gInfinitelyAmmo;

DLL_GLOBAL	short		g_sModelIndexLaser;// holds the index for the laser beam
DLL_GLOBAL  const char 	*g_pModelNameLaser = "sprites/laserbeam.spr";
DLL_GLOBAL	short    	g_sModelIndexLaserDot;// holds the index for the laser beam dot
DLL_GLOBAL	short    	g_sModelIndexFireball;// holds the index for the fireball
DLL_GLOBAL	short    	g_sModelIndexSmoke;// holds the index for the smoke cloud
DLL_GLOBAL	short    	g_sModelIndexWExplosion;// holds the index for the underwater explosion
DLL_GLOBAL	short		g_sModelIndexBubbles;// holds the index for the bubbles model
DLL_GLOBAL	short		g_sModelIndexBloodDrop;// holds the sprite index for the initial blood
DLL_GLOBAL	short		g_sModelIndexBloodSpray;// holds the sprite index for splattered blood
DLL_GLOBAL	short		g_sModelIndexNullModel; //null model index
DLL_GLOBAL	short		g_sModelIndexErrorModel;//error model index
DLL_GLOBAL	short		g_sModelIndexNullSprite;//null sprite index
DLL_GLOBAL	short		g_sModelIndexErrorSprite;//error sprite index
DLL_GLOBAL	short		g_sSoundIndexNullSound;//null sound index

DLL_GLOBAL	short		g_sModelIndexFireball_0;
DLL_GLOBAL	short		g_sModelIndexFireball_1;
DLL_GLOBAL	short		g_sModelIndexFireballFlash;

DLL_GLOBAL	short		g_sGrenadeGib;

DLL_GLOBAL	short		g_sModelIndexSpore1; // holds the index for the spore explosion 1
DLL_GLOBAL	short		g_sModelIndexSpore2; // holds the index for the spore explosion 2
DLL_GLOBAL	short		g_sModelIndexSpore3; // holds the index for the spore explosion 3
DLL_GLOBAL  short		g_sModelIndexBigSpit; // holds the index for the bullsquid big spit.
DLL_GLOBAL  short		g_sModelIndexTinySpit; // holds the index for the bullsquid tiny spit.

DLL_GLOBAL	unsigned short	g_usEventIndexNullEvent;//null event index
DLL_GLOBAL 	unsigned short 	m_usDecals;	//Decal event
DLL_GLOBAL 	unsigned short 	m_usEfx;		//special effects event (rocket trail, explosion e.t.c.)	
DLL_GLOBAL	unsigned short	m_usPlayEmptySound;	//play empty sound on client side

ItemInfo CBasePlayerItem::ItemInfoArray[MAX_WEAPONS];
AmmoInfo CBasePlayerItem::AmmoInfoArray[MAX_AMMO_SLOTS];

extern int gmsgCurWeapon;

MULTIDAMAGE gMultiDamage;

//=========================================================
// MaxAmmoCarry - pass in a name and this function will tell
// you the maximum amount of that type of ammunition that a 
// player can carry.
//=========================================================
int MaxAmmoCarry(int iszName)
{
	for (int i = 0; i < MAX_WEAPONS; i++)
	{
		if (CBasePlayerItem::ItemInfoArray[i].pszAmmo1 && !strcmp(STRING(iszName), CBasePlayerItem::ItemInfoArray[i].pszAmmo1))
			return CBasePlayerItem::ItemInfoArray[i].iMaxAmmo1;

		if (CBasePlayerItem::ItemInfoArray[i].pszAmmo2 && !strcmp(STRING(iszName), CBasePlayerItem::ItemInfoArray[i].pszAmmo2))
			return CBasePlayerItem::ItemInfoArray[i].iMaxAmmo2;
	}

	ALERT(at_debug, "MaxAmmoCarry() doesn't recognize '%s'!\n", STRING(iszName));
	return -1;
}

LINK_ENTITY_TO_CLASS(laser_spot, CLaserSpot);

//=========================================================
//=========================================================
CLaserSpot *CLaserSpot::CreateSpot()
{
	CLaserSpot *pSpot = GetClassPtr((CLaserSpot *)NULL);
	pSpot->Spawn();

	pSpot->pev->classname = MAKE_STRING("laser_spot");

	return pSpot;
}

//=========================================================
//=========================================================
CLaserSpot *CLaserSpot::CreateSpot(const char* spritename)
{
	CLaserSpot *pSpot = CreateSpot();
	SET_MODEL(ENT(pSpot->pev), spritename);
	return pSpot;
}

//=========================================================
//=========================================================
void CLaserSpot::Spawn()
{
	Precache();
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_NOT;

	pev->rendermode = kRenderGlow;
	pev->renderfx = kRenderFxNoDissipation;
	pev->renderamt = 255;

	SET_MODEL(ENT(pev), "sprites/laserdot.spr");
	UTIL_SetOrigin(this, pev->origin);
};

//=========================================================
// Suspend- make the laser sight invisible. 
//=========================================================
void CLaserSpot::Suspend(float flSuspendTime)
{
	pev->effects |= EF_NODRAW;

	//LRC: -1 means suspend indefinitely
	if (flSuspendTime == -1)
	{
		SetThink(NULL);
	}
	else
	{
		SetThink(&CLaserSpot::Revive);
		SetNextThink(flSuspendTime);
	}
}

//=========================================================
// Revive - bring a suspended laser sight back.
//=========================================================
void CLaserSpot::Revive()
{
	pev->effects &= ~EF_NODRAW;
	SetThink(NULL);
}

void CLaserSpot::Precache()
{
	PRECACHE_MODEL("sprites/laserdot.spr");
};

/*
==============================================================================

MULTI-DAMAGE

Collects multiple small damages into a single damage

==============================================================================
*/

//
// ClearMultiDamage - resets the global multi damage accumulator
//
void ClearMultiDamage()
{
	gMultiDamage.pEntity = NULL;
	gMultiDamage.amount = 0;
	gMultiDamage.type = 0;
}

//
// ApplyMultiDamage - inflicts contents of global multi damage register on gMultiDamage.pEntity
//
// GLOBALS USED:
//	gMultiDamage
void ApplyMultiDamage(entvars_t *pevInflictor, entvars_t *pevAttacker)
{
	Vector		vecSpot1;//where blood comes from
	Vector		vecDir;//direction blood should go
	TraceResult	tr;

	if (!gMultiDamage.pEntity)
		return;

	gMultiDamage.pEntity->TakeDamage(pevInflictor, pevAttacker, gMultiDamage.amount, gMultiDamage.type);
}

// GLOBALS USED:
//	gMultiDamage
void AddMultiDamage(entvars_t *pevInflictor, CBaseEntity *pEntity, float flDamage, int bitsDamageType)
{
	if (!pEntity)
		return;

	gMultiDamage.type |= bitsDamageType;

	if (pEntity != gMultiDamage.pEntity)
	{
		ApplyMultiDamage(pevInflictor, pevInflictor); // UNDONE: wrong attacker!
		gMultiDamage.pEntity = pEntity;
		gMultiDamage.amount = 0;
	}

	gMultiDamage.amount += flDamage;
}

/*
================
SpawnBlood
================
*/
void SpawnBlood(Vector vecSpot, int bloodColor, float flDamage)
{
	UTIL_BloodDrips(vecSpot, g_vecAttackDir, bloodColor, (int)flDamage);
}

int DamageDecal(CBaseEntity *pEntity, int bitsDamageType)
{
	if (!pEntity)
		return (DECAL_GUNSHOT1 + RANDOM_LONG(0, 4));

	return pEntity->DamageDecal(bitsDamageType);
}

void DecalGunshot(TraceResult *pTrace, int iBulletType)
{
	// Is the entity valid
	if (!UTIL_IsValidEntity(pTrace->pHit))
		return;

	if (VARS(pTrace->pHit)->solid == SOLID_BSP || VARS(pTrace->pHit)->movetype == MOVETYPE_PUSHSTEP)
	{
		CBaseEntity *pEntity = NULL;
		// Decal the wall with a gunshot
		if (!FNullEnt(pTrace->pHit))
			pEntity = CBaseEntity::Instance(pTrace->pHit);

		switch (iBulletType)
		{
		case BULLET_PLAYER_9MM:
		case BULLET_MONSTER_9MM:
		case BULLET_PLAYER_MP5:
		case BULLET_MONSTER_MP5:
		case BULLET_PLAYER_556:
		case BULLET_MONSTER_556:
		case BULLET_PLAYER_BUCKSHOT:
		case BULLET_PLAYER_357:
		case BULLET_MONSTER_357:
		case BULLET_MONSTER_12MM:
		default:
			// smoke and decal
			UTIL_GunshotDecalTrace(pTrace, DamageDecal(pEntity, DMG_BULLET));
			break;
		case BULLET_PLAYER_CROWBAR:
			// wall decal
			UTIL_DecalTrace(pTrace, DamageDecal(pEntity, DMG_CLUB));
			break;
		}
	}
}

//
// WeaponFlash - tosses a brass shell from passed origin at passed velocity
//
void WeaponFlash(const Vector &vecOrigin)
{
	int m_iRadius = RANDOM_FLOAT(16, 12);
	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_DLIGHT);
	WRITE_COORD(vecOrigin.x); // origin
	WRITE_COORD(vecOrigin.y);
	WRITE_COORD(vecOrigin.z);
	WRITE_BYTE(m_iRadius);     // radius
	WRITE_BYTE(255);     // R
	WRITE_BYTE(255);     // G
	WRITE_BYTE(128);     // B
	WRITE_BYTE(0);     // life * 10
	WRITE_BYTE(0); // decay
	MESSAGE_END();
}

//
// EjectBrass - tosses a brass shell from passed origin at passed velocity
//
void EjectBrass(const Vector &vecOrigin, const Vector &vecVelocity, float rotation, int model, int soundtype)
{
	// FIX: when the player shoots, their gun isn't in the same position as it is on the model other players see.

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecOrigin);
	WRITE_BYTE(TE_MODEL);
	WRITE_COORD(vecOrigin.x);
	WRITE_COORD(vecOrigin.y);
	WRITE_COORD(vecOrigin.z);
	WRITE_COORD(vecVelocity.x);
	WRITE_COORD(vecVelocity.y);
	WRITE_COORD(vecVelocity.z);
	WRITE_ANGLE(rotation);
	WRITE_SHORT(model);
	WRITE_BYTE(soundtype);
	WRITE_BYTE(25);// 2.5 seconds
	MESSAGE_END();
}

int giAmmoIndex = 0;

// Precaches the ammo and queues the ammo info for sending to clients
void AddAmmoNameToAmmoRegistry(const char *szAmmoname)
{
	// make sure it's not already in the registry
	for (int i = 0; i < MAX_AMMO_SLOTS; i++)
	{
		if (!CBasePlayerItem::AmmoInfoArray[i].pszName)
			continue;

		if (_stricmp(CBasePlayerItem::AmmoInfoArray[i].pszName, szAmmoname) == 0)
			return; // ammo already in registry, just quite
	}


	giAmmoIndex++;
	ASSERT(giAmmoIndex < MAX_AMMO_SLOTS);
	if (giAmmoIndex >= MAX_AMMO_SLOTS)
		giAmmoIndex = 0;

	CBasePlayerItem::AmmoInfoArray[giAmmoIndex].pszName = szAmmoname;
	CBasePlayerItem::AmmoInfoArray[giAmmoIndex].iId = giAmmoIndex;   // yes, this info is redundant
}


// Precaches the weapon and queues the weapon info for sending to clients
void UTIL_PrecacheOtherWeapon(const char *szClassname)
{
	edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING(szClassname));
	if (FNullEnt(pent))
	{
		ALERT(at_debug, "NULL Ent in UTIL_PrecacheOtherWeapon\n");
		return;
	}

	CBaseEntity *pEntity = CBaseEntity::Instance(VARS(pent));

	if (pEntity)
	{
		ItemInfo II;
		pEntity->Precache();
		memset(&II, 0, sizeof II);
		if (static_cast<CBasePlayerItem*>(pEntity)->GetItemInfo(&II))
		{
			CBasePlayerItem::ItemInfoArray[II.iId] = II;

			if (II.pszAmmo1 && *II.pszAmmo1)
			{
				AddAmmoNameToAmmoRegistry(II.pszAmmo1);
			}

			if (II.pszAmmo2 && *II.pszAmmo2)
			{
				AddAmmoNameToAmmoRegistry(II.pszAmmo2);
			}

			memset(&II, 0, sizeof II);
		}
	}

	REMOVE_ENTITY(pent);
}

// called by worldspawn FIXME. Move this to client.cpp
void WeaponsPrecache()
{
	memset(CBasePlayerItem::ItemInfoArray, 0, sizeof(CBasePlayerItem::ItemInfoArray));
	memset(CBasePlayerItem::AmmoInfoArray, 0, sizeof(CBasePlayerItem::AmmoInfoArray));

	giAmmoIndex = 0;

	//g-cont. init safe precaching system
	//WARNING!!! this is critical stuff! do not edit this
	g_sSoundIndexNullSound = PRECACHE_SOUND("common/null.wav");
	g_sModelIndexNullModel = PRECACHE_MODEL("models/null.mdl");
	g_sModelIndexErrorModel = PRECACHE_MODEL("models/error.mdl");
	g_sModelIndexNullSprite = PRECACHE_MODEL("sprites/null.spr");
	g_sModelIndexErrorSprite = PRECACHE_MODEL("sprites/error.spr");

	// common world objects
	UTIL_PrecacheOther("item_suit");
	UTIL_PrecacheOther("item_battery");
	UTIL_PrecacheOther("item_antidote");
	UTIL_PrecacheOther("item_security");
	UTIL_PrecacheOther("item_longjump");
	UTIL_PrecacheOther("item_healthkit");
	UTIL_PrecacheOther("item_camera");
	UTIL_PrecacheOther("item_flare");
	UTIL_PrecacheOther("item_antirad");
	UTIL_PrecacheOther("item_medicalkit");

	// shotgun
	UTIL_PrecacheOtherWeapon("weapon_shotgun");
	UTIL_PrecacheOther("ammo_buckshot");

	// crowbar
	UTIL_PrecacheOtherWeapon("weapon_crowbar");

	// pipewrench
	UTIL_PrecacheOtherWeapon("weapon_pipewrench");

	// knife
	UTIL_PrecacheOtherWeapon("weapon_knife");

	// grapple
	UTIL_PrecacheOtherWeapon("weapon_grapple");

	//shockrifle
	UTIL_PrecacheOtherWeapon("weapon_shockrifle");

	// glock
	UTIL_PrecacheOtherWeapon("weapon_9mmhandgun");
	UTIL_PrecacheOther("ammo_9mmclip");
	UTIL_PrecacheOther("ammo_9mmbox"); //LRC

	// mp5
	UTIL_PrecacheOtherWeapon("weapon_9mmAR");
	UTIL_PrecacheOther("ammo_9mmAR");
	UTIL_PrecacheOther("ammo_ARgrenades");

	// eagle
	UTIL_PrecacheOtherWeapon("weapon_eagle");

	// python
	UTIL_PrecacheOtherWeapon("weapon_357");
	UTIL_PrecacheOther("ammo_357");

	// gauss
	UTIL_PrecacheOtherWeapon("weapon_gauss");
	UTIL_PrecacheOther("ammo_gaussclip");

	// rpg
	UTIL_PrecacheOtherWeapon("weapon_rpg");
	UTIL_PrecacheOther("ammo_rpgclip");

	// crossbow
	UTIL_PrecacheOtherWeapon("weapon_crossbow");
	UTIL_PrecacheOther("ammo_crossbow");

	// egon
	UTIL_PrecacheOtherWeapon("weapon_egon");

	// tripmine
	UTIL_PrecacheOtherWeapon("weapon_tripmine");

	// satchel charge
	UTIL_PrecacheOtherWeapon("weapon_satchel");

	// hand grenade
	UTIL_PrecacheOtherWeapon("weapon_handgrenade");

	// squeak grenade
	UTIL_PrecacheOtherWeapon("weapon_snark");

	// hornetgun
	UTIL_PrecacheOtherWeapon("weapon_hornetgun");

	// cycler
	UTIL_PrecacheOtherWeapon("cycler_weapon");

	// saw
	UTIL_PrecacheOtherWeapon("weapon_m249");
	UTIL_PrecacheOther("ammo_556");

	// sporelauncher
	UTIL_PrecacheOtherWeapon("weapon_sporelauncher");
	UTIL_PrecacheOther("ammo_spore");

	// sniperrifle
	UTIL_PrecacheOtherWeapon("weapon_sniperrifle");
	UTIL_PrecacheOther("ammo_762");

	// displacer
	UTIL_PrecacheOtherWeapon("weapon_displacer");
	UTIL_PrecacheOther("portal");

	if (g_pGameRules->IsDeathmatch()) {
		UTIL_PrecacheOther("weaponbox");// container for dropped deathmatch weapons
	}

	//Explosion Effects
	PRECACHE_MODEL("sprites/explosion_0.spr");
	PRECACHE_MODEL("sprites/explosion_1.spr");
	PRECACHE_MODEL("sprites/explosion_2.spr");
	PRECACHE_MODEL("sprites/explosion_3.spr");

	g_sModelIndexFireball = PRECACHE_MODEL("sprites/zerogxplode.spr");// fireball
	g_sModelIndexWExplosion = PRECACHE_MODEL("sprites/WXplo1.spr");// underwater fireball
	g_sModelIndexSmoke = PRECACHE_MODEL("sprites/steam1.spr");// smoke
	g_sModelIndexBubbles = PRECACHE_MODEL("sprites/bubble.spr");//bubbles
	g_sModelIndexBloodSpray = PRECACHE_MODEL("sprites/bloodspray.spr"); // initial blood
	g_sModelIndexBloodDrop = PRECACHE_MODEL("sprites/blood.spr"); // splattered blood 

	g_sModelIndexFireball_0 = PRECACHE_MODEL("sprites/explosion_ext0.spr");
	g_sModelIndexFireball_1 = PRECACHE_MODEL("sprites/explosion_ext1.spr");
	g_sModelIndexFireballFlash = PRECACHE_MODEL("sprites/explosion_flash.spr");
	g_sGrenadeGib = PRECACHE_MODEL("models/grenade_gibs.mdl");

	g_sModelIndexSpore1 = PRECACHE_MODEL("sprites/spore_exp_01.spr");
	g_sModelIndexSpore2 = PRECACHE_MODEL("sprites/spore_exp_b_01.spr");
	g_sModelIndexSpore3 = PRECACHE_MODEL("sprites/spore_exp_c_01.spr");
	g_sModelIndexBigSpit = PRECACHE_MODEL("sprites/bigspit.spr");
	g_sModelIndexTinySpit = PRECACHE_MODEL("sprites/tinyspit.spr");

	g_sModelIndexLaser = PRECACHE_MODEL((char *)g_pModelNameLaser);
	g_sModelIndexLaserDot = PRECACHE_MODEL("sprites/laserdot.spr");
	m_usPlayEmptySound = PRECACHE_EVENT(1, "events/tripfire.sc");
	m_usDecals = PRECACHE_EVENT(1, "events/decals.sc");
	m_usEfx = PRECACHE_EVENT(1, "events/explode.sc");

	// used by explosions
	PRECACHE_MODEL("models/grenade.mdl");
	PRECACHE_MODEL("models/spore.mdl");
	PRECACHE_MODEL("sprites/explode1.spr");

	PRECACHE_SOUND("weapons/debris1.wav");// explosion aftermaths
	PRECACHE_SOUND("weapons/debris2.wav");// explosion aftermaths
	PRECACHE_SOUND("weapons/debris3.wav");// explosion aftermaths

	PRECACHE_SOUND("weapons/grenade_hit1.wav");//grenade
	PRECACHE_SOUND("weapons/grenade_hit2.wav");//grenade
	PRECACHE_SOUND("weapons/grenade_hit3.wav");//grenade

	PRECACHE_SOUND("weapons/bullet_hit1.wav");	// hit by bullet
	PRECACHE_SOUND("weapons/bullet_hit2.wav");	// hit by bullet

	PRECACHE_SOUND("items/9mmclip1.wav");
	PRECACHE_SOUND("items/9mmclip2.wav");

	PRECACHE_SOUND("items/weapondrop1.wav");// weapon falls to the ground

	PRECACHE_SOUND("weapons/splauncher_impact.wav");//explosion aftermaths
	PRECACHE_SOUND("weapons/spore_hit1.wav");//sporegrenade
	PRECACHE_SOUND("weapons/spore_hit2.wav");//sporegrenade
	PRECACHE_SOUND("weapons/spore_hit3.wav");//sporegrenade

	PRECACHE_MODEL("sprites/metal_glow.spr");
}

void CBasePlayerItem::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "master"))
	{
		m_sMaster = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseDelay::KeyValue(pkvd);
}


TYPEDESCRIPTION	CBasePlayerItem::m_SaveData[] =
{
	DEFINE_FIELD(CBasePlayerItem, m_pPlayer, FIELD_CLASSPTR),
	DEFINE_FIELD(CBasePlayerItem, m_pNext, FIELD_CLASSPTR),
	DEFINE_FIELD(CBasePlayerItem, m_iId, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayerWeapon, m_sMaster, FIELD_STRING),
}; IMPLEMENT_SAVERESTORE(CBasePlayerItem, CBaseAnimating);

TYPEDESCRIPTION	CBasePlayerWeapon::m_SaveData[] =
{
	DEFINE_FIELD(CBasePlayerWeapon, m_flNextPrimaryAttack, FIELD_TIME),
	DEFINE_FIELD(CBasePlayerWeapon, m_flNextSecondaryAttack, FIELD_TIME),
	DEFINE_FIELD(CBasePlayerWeapon, m_flTimeWeaponIdle, FIELD_TIME),
	DEFINE_FIELD(CBasePlayerWeapon, m_flTimeUpdate, FIELD_TIME),
	DEFINE_FIELD(CBasePlayerWeapon, m_iPrimaryAmmoType, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayerWeapon, m_iSecondaryAmmoType, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayerWeapon, m_iClip, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayerWeapon, m_iDefaultAmmo, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayerWeapon, m_iChargeLevel, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayerWeapon, m_iOverloadLevel, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayerWeapon, m_fInAttack, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayerWeapon, m_iBody, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CBasePlayerWeapon, CBasePlayerItem);

void CBasePlayerItem::SetObjectCollisionBox()
{
	pev->absmin = pev->origin + Vector(-24, -24, 0);
	pev->absmax = pev->origin + Vector(24, 24, 16);
}

//=========================================================
// Sets up movetype, size, solidtype for a new weapon. 
//=========================================================
void CBasePlayerItem::FallInit()
{
	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_BBOX;

	UTIL_SetOrigin(this, pev->origin);
	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));//pointsize until it lands on the ground.

	SetTouch(&CBasePlayerItem::DefaultTouch);
	SetThink(&CBasePlayerItem::FallThink);

	SetNextThink(0.1);
}

//=========================================================
// FallThink - Items that have just spawned run this think
// to catch them when they hit the ground. Once we're sure
// that the object is grounded, we change its solid type
// to trigger and set it in a large box that helps the
// player get it.
//=========================================================
void CBasePlayerItem::FallThink()
{
	SetNextThink(0.1);

	if (pev->flags & FL_ONGROUND)
	{
		// clatter if we have an owner (i.e., dropped by someone)
		// don't clatter if the gun is waiting to respawn (if it's waiting, it is invisible!)
		if (!FNullEnt(pev->owner))
		{
			int pitch = 95 + RANDOM_LONG(0, 29);
			EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "items/weapondrop1.wav", VOL_NORM, ATTN_NORM, 0, pitch);
		}

		// lie flat
		pev->angles.x = 0;
		pev->angles.z = 0;

		Materialize();
	}
}

//=========================================================
// Materialize - make a CBasePlayerItem visible and tangible
//=========================================================
void CBasePlayerItem::Materialize()
{
	if (pev->effects & EF_NODRAW)
	{
		// changing from invisible state to visible.
		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "items/suitchargeok1.wav", VOL_NORM, ATTN_NORM, 0, 150);
		pev->effects &= ~EF_NODRAW;
		pev->effects |= EF_MUZZLEFLASH;
	}

	pev->solid = SOLID_TRIGGER;

	UTIL_SetOrigin(this, pev->origin);// link into world.
	SetTouch(&CBasePlayerItem::DefaultTouch);
	SetThink(NULL);

}

//=========================================================
// AttemptToMaterialize - the item is trying to rematerialize,
// should it do so now or wait longer?
//=========================================================
void CBasePlayerItem::AttemptToMaterialize()
{
	float time = g_pGameRules->FlWeaponTryRespawn(this);

	if (time == 0)
	{
		Materialize();
		return;
	}

	SetNextThink(time);
}

//=========================================================
// CheckRespawn - a player is taking this weapon, should 
// it respawn?
//=========================================================
void CBasePlayerItem::CheckRespawn()
{
	switch (g_pGameRules->WeaponShouldRespawn(this))
	{
	case GR_WEAPON_RESPAWN_YES:
		Respawn();
		break;
	case GR_WEAPON_RESPAWN_NO:
		return;
		break;
	}
}

//=========================================================
// Respawn- this item is already in the world, but it is
// invisible and intangible. Make it visible and tangible.
//=========================================================
CBaseEntity* CBasePlayerItem::Respawn()
{
	// make a copy of this weapon that is invisible and inaccessible to players (no touch function). The weapon spawn/respawn code
	// will decide when to make the weapon visible and touchable.
	CBaseEntity *pNewWeapon = CBaseEntity::Create((char *)STRING(pev->classname), g_pGameRules->VecWeaponRespawnSpot(this), pev->angles, pev->owner);

	if (pNewWeapon)
	{
		pNewWeapon->pev->effects |= EF_NODRAW;// invisible for now
		pNewWeapon->SetTouch(NULL);// no touch
		pNewWeapon->SetThink(&CBasePlayerItem::AttemptToMaterialize);

		DROP_TO_FLOOR(ENT(pev));

		// not a typo! We want to know when the weapon the player just picked up should respawn! This new entity we created is the replacement,
		// but when it should respawn is based on conditions belonging to the weapon that was taken.
		pNewWeapon->AbsoluteNextThink(g_pGameRules->FlWeaponRespawnTime(this));
	}
	else
	{
		ALERT(at_debug, "Respawn failed to create %s!\n", STRING(pev->classname));
	}

	return pNewWeapon;
}

void CBasePlayerItem::DefaultTouch(CBaseEntity *pOther)
{
	// if it's not a player, ignore
	if (!pOther->IsPlayer())
		return;

	CBasePlayer *pPlayer = static_cast<CBasePlayer *>(pOther);

	// can I have this?
	if (!g_pGameRules->CanHavePlayerItem(pPlayer, this))
	{
		if (gEvilImpulse101)
		{
			UTIL_Remove(this);
		}

		return;
	}

	if (pOther->AddPlayerItem(this))
	{
		AttachToPlayer(pPlayer);
		EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, "items/gunpickup2.wav", VOL_NORM, ATTN_NORM);

		if (!gEvilImpulse101)
		{
			int i;
			char sample[32];
			char weapon_name[32];
			strcpy(weapon_name, STRING(pev->classname));

			if (strncmp(weapon_name, "weapon_", 7) == 0)
				i = 7;
			else if (strncmp(weapon_name, "item_", 5) == 0)
				i = 5;
			else i = 0; // for cycler_weapon

			snprintf(sample, 32, "!%s", weapon_name + i);
			pPlayer->SetSuitUpdate(sample, FALSE, SUIT_NEXT_IN_30SEC);
		}
	}

	SUB_UseTargets(pOther, USE_TOGGLE, 0); // UNDONE: when should this happen?

	if (m_pfnThink == &CBasePlayerItem::FallThink)
		SetThink(NULL);
}

void CBasePlayerItem::Spawn()
{
	pev->animtime = UTIL_GlobalTimeBase() + 0.1;
	CBaseAnimating::Spawn();
}

BOOL CanAttack(float attack_time, float curtime, BOOL isPredicted)
{
	if (curtime != 0.0)
		return (attack_time <= curtime) ? TRUE : FALSE;

	return (attack_time <= 0.0) ? TRUE : FALSE;
}

void CBasePlayerWeapon::ItemPostFrame()
{
	if ((m_fInReload) && (m_pPlayer->m_flNextAttack <= UTIL_GlobalTimeBase()))
	{
		// complete the reload. 
		int j = min(iMaxClip() - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);

		// Add them to the clip
		m_iClip += j;
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= j;

		m_pPlayer->TabulateAmmo();

		m_fInReload = FALSE;
	}

	if ((m_pPlayer->pev->button & IN_ATTACK2) && CanAttack(m_flNextSecondaryAttack, UTIL_GlobalTimeBase(), 0))
	{
		if (pszAmmo2() && !m_pPlayer->m_rgAmmo[SecondaryAmmoIndex()])
		{
			m_fFireOnEmpty = TRUE;
		}

		m_pPlayer->TabulateAmmo();
		SecondaryAttack();
		m_pPlayer->pev->button &= ~IN_ATTACK2;
	}
	else if ((m_pPlayer->pev->button & IN_ATTACK) && CanAttack(m_flNextPrimaryAttack, UTIL_GlobalTimeBase(), 0))
	{
		if ((m_iClip == 0 && pszAmmo1()) || (iMaxClip() == -1 && !m_pPlayer->m_rgAmmo[PrimaryAmmoIndex()]))
		{
			m_fFireOnEmpty = TRUE;
		}

		m_pPlayer->TabulateAmmo();
		PrimaryAttack();
	}
	else if (m_pPlayer->pev->button & IN_RELOAD && iMaxClip() != WEAPON_NOCLIP && !m_fInReload)
	{
		// reload when reload is pressed, or if no buttons are down and weapon is empty.
		Reload();
	}
	else if (!(m_pPlayer->pev->button & (IN_ATTACK | IN_ATTACK2)))
	{
		// no fire buttons down
		m_fFireOnEmpty = FALSE;

		if (!IsUseable() && m_flNextPrimaryAttack < UTIL_GlobalTimeBase())
		{
			// weapon isn't useable, switch.
			if (!(iFlags() & ITEM_FLAG_NOAUTOSWITCHEMPTY) && g_pGameRules->GetNextBestWeapon(m_pPlayer, this))
			{
				m_flNextPrimaryAttack = UTIL_GlobalTimeBase() + 0.3;
				return;
			}
		}
		else
		{
			// weapon is useable. Reload if empty and weapon has waited as long as it has to after firing
			if (m_iClip == 0 && !(iFlags() & ITEM_FLAG_NOAUTORELOAD) && m_flNextPrimaryAttack < UTIL_GlobalTimeBase())
			{
				Reload();
				return;
			}
		}

		ResetEmptySound();
		WeaponIdle();
		return;
	}

	// catch all
	if (ShouldWeaponIdle())
	{
		WeaponIdle();
	}
}

void CBasePlayerItem::DestroyItem()
{
	if (m_pPlayer)
	{
		// if attached to a player, remove. 
		m_pPlayer->RemovePlayerItem(this);
	}

	Kill();
}

int CBasePlayerItem::AddToPlayer(CBasePlayer *pPlayer)
{
	m_pPlayer = pPlayer;
	return TRUE;
}

void CBasePlayerItem::Drop()
{
	SetTouch(NULL);
	SetThink(&CBasePlayerItem::SUB_Remove);
	SetNextThink(0.1);
}

void CBasePlayerItem::Kill()
{
	SetTouch(NULL);
	SetThink(&CBasePlayerItem::SUB_Remove);
	SetNextThink(0.1);
}

void CBasePlayerItem::Holster()
{
	m_pPlayer->pev->viewmodel = 0;
	m_pPlayer->pev->weaponmodel = 0;
}

void CBasePlayerItem::AttachToPlayer(CBasePlayer *pPlayer)
{
	pev->movetype = MOVETYPE_FOLLOW;
	pev->solid = SOLID_NOT;
	pev->aiment = pPlayer->edict();
	pev->effects = EF_NODRAW; // ??
	pev->modelindex = 0;// server won't send down to clients if modelindex == 0
	pev->model = iStringNull;
	pev->owner = pPlayer->edict();
	SetNextThink(0.1);
	SetTouch(NULL);
	SetThink(NULL);
}

//LRC
void CBasePlayerWeapon::SetNextThink(float delay)
{
	m_fNextThink = UTIL_GlobalTimeBase() + delay;
	pev->nextthink = m_fNextThink;
}

/// CALLED THROUGH the newly-touched weapon's instance. The existing player weapon is pOriginal
bool CBasePlayerWeapon::AddDuplicate(CBasePlayerItem *pOriginal)
{
	if (!UTIL_IsMasterTriggered(m_sMaster, m_pPlayer))		//
		return false;										// AJH allows for locked weapons 

	if (m_iDefaultAmmo)
		return ExtractAmmo((CBasePlayerWeapon *)pOriginal);

	// a dead player dropped this.
	return (bool)ExtractClipAmmo((CBasePlayerWeapon *)pOriginal);
}


int CBasePlayerWeapon::AddToPlayer(CBasePlayer *pPlayer)
{
	if (!UTIL_IsMasterTriggered(m_sMaster, pPlayer))		//
		return FALSE;										// AJH allows for locked weapons

	int bResult = CBasePlayerItem::AddToPlayer(pPlayer);

	pPlayer->pev->weapons |= (1 << m_iId);

	if (!m_iPrimaryAmmoType)
	{
		m_iPrimaryAmmoType = pPlayer->GetAmmoIndex(pszAmmo1());
		m_iSecondaryAmmoType = pPlayer->GetAmmoIndex(pszAmmo2());
	}


	if (bResult)//generic message for all weapons
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev);
		WRITE_BYTE(m_iId);
		MESSAGE_END();

		return AddWeapon();
	}
	return FALSE;
}

int CBasePlayerWeapon::UpdateClientData(CBasePlayer *pPlayer)
{
	BOOL bSend = FALSE;
	int state = 0;
	if (pPlayer->m_pActiveItem == this)
	{
		if (pPlayer->m_fOnTarget)
			state = WEAPON_IS_ONTARGET;
		else
			state = 1;
	}

	// Forcing send of all data!
	if (!pPlayer->m_fWeapon)
	{
		bSend = TRUE;
	}

	// This is the current or last weapon, so the state will need to be updated
	if (this == pPlayer->m_pActiveItem ||
		this == pPlayer->m_pClientActiveItem)
	{
		if (pPlayer->m_pActiveItem != pPlayer->m_pClientActiveItem)
		{
			bSend = TRUE;
		}
	}

	// If the ammo, state, or fov has changed, update the weapon
	if (m_iClip != m_iClientClip || state != m_iClientWeaponState || pPlayer->m_iFOV != pPlayer->m_iClientFOV)
	{
		bSend = TRUE;
	}

	if (bSend)
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgCurWeapon, NULL, pPlayer->pev);
		WRITE_BYTE(state);
		WRITE_BYTE(m_iId);
		WRITE_BYTE(m_iClip);
		MESSAGE_END();

		m_iClientClip = m_iClip;
		m_iClientWeaponState = state;
		pPlayer->m_fWeapon = TRUE;
	}

	if (m_pNext)
		m_pNext->UpdateClientData(pPlayer);

	return 1;
}

void CBasePlayerWeapon::SendWeaponAnim(const int &iAnim)
{
	if (m_pPlayer == NULL) return;
	m_pPlayer->pev->weaponanim = iAnim;

	MESSAGE_BEGIN(MSG_ONE, SVC_WEAPONANIM, NULL, m_pPlayer->pev);
	WRITE_BYTE(iAnim);						// sequence number
	WRITE_BYTE(pev->body);					// weaponmodel bodygroup.
	MESSAGE_END();
}

bool CBasePlayerWeapon::AddPrimaryAmmo(int iCount, char *szName, int iMaxClip, int iMaxCarry)
{
	int iIdAmmo;
	if (iMaxClip < 1)
	{
		m_iClip = -1;
		iIdAmmo = m_pPlayer->GiveAmmo(iCount, szName, iMaxCarry);
	}
	else if (m_iClip == 0)
	{
		int i;
		i = min(m_iClip + iCount, iMaxClip) - m_iClip;
		m_iClip += i;
		iIdAmmo = m_pPlayer->GiveAmmo(iCount - i, szName, iMaxCarry);
	}
	else
	{
		iIdAmmo = m_pPlayer->GiveAmmo(iCount, szName, iMaxCarry);
	}

	// m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = iMaxCarry; // hack for testing

	if (iIdAmmo > 0)
	{
		m_iPrimaryAmmoType = iIdAmmo;
		if (m_pPlayer->HasPlayerItem(this))
		{
			// play the "got ammo" sound only if we gave some ammo to a player that already had this gun.
			// if the player is just getting this gun for the first time, DefaultTouch will play the "picked up gun" sound for us.
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
		}
	}

	return iIdAmmo > 0 ? TRUE : FALSE;
}


bool CBasePlayerWeapon::AddSecondaryAmmo(int iCount, char *szName, int iMax)
{
	int iIdAmmo = m_pPlayer->GiveAmmo(iCount, szName, iMax);

	//m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = iMax; // hack for testing

	if (iIdAmmo > 0)
	{
		m_iSecondaryAmmoType = iIdAmmo;
		EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	}

	return iIdAmmo > 0 ? true : false;
}

//=========================================================
// IsUseable - this function determines whether or not a 
// weapon is useable by the player in its current state. 
// (does it have ammo loaded? do I have any ammo for the 
// weapon?, etc)
//=========================================================
BOOL CBasePlayerWeapon::IsUseable()
{
	return CanDeploy();
}

BOOL CBasePlayerWeapon::CanDeploy()
{
	BOOL bHasAmmo = 0;

	if (!pszAmmo1())
	{
		// this weapon doesn't use ammo, can always deploy.
		return TRUE;
	}

	if (pszAmmo1())
	{
		bHasAmmo |= (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] != 0);
	}
	if (pszAmmo2())
	{
		bHasAmmo |= (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] != 0);
	}
	if (m_iClip > 0)
	{
		bHasAmmo |= 1;
	}
	if (!bHasAmmo)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CBasePlayerWeapon::DefaultDeploy(char *szViewModel, char *szWeaponModel, int iAnim, char *szAnimExt, float fDrawTime)
{
	if (IsMultiplayer() && !CanDeploy()) return FALSE;
	m_iLastSkin = -1;//reset last skin info for new weapon
	b_Restored = TRUE;//no need update if deploy
	m_pPlayer->TabulateAmmo();
	m_pPlayer->pev->viewmodel = MAKE_STRING(szViewModel);
	m_pPlayer->pev->weaponmodel = MAKE_STRING(szWeaponModel);
	strcpy(m_pPlayer->m_szAnimExtention, szAnimExt);
	SendWeaponAnim(iAnim);

	m_pPlayer->m_flNextAttack = UTIL_GlobalTimeBase() + fDrawTime;//Custom time for deploy
	m_flTimeWeaponIdle = UTIL_GlobalTimeBase() + fDrawTime + 0.5; //Make half-second delay beetwen draw and idle animation

	return TRUE;
}

BOOL CBasePlayerWeapon::DefaultDeploy(string_t iViewModel, string_t iWeaponModel, int iAnim, char *szAnimExt, float fDrawTime)
{
	if (IsMultiplayer() && !CanDeploy()) return FALSE;
	m_iLastSkin = -1;//reset last skin info for new weapon
	b_Restored = TRUE;//no need update if deploy
	m_pPlayer->TabulateAmmo();
	m_pPlayer->pev->viewmodel = iViewModel;
	m_pPlayer->pev->weaponmodel = iWeaponModel;
	strcpy(m_pPlayer->m_szAnimExtention, szAnimExt);
	SendWeaponAnim(iAnim);

	m_pPlayer->m_flNextAttack = UTIL_GlobalTimeBase() + fDrawTime;//Custom time for deploy
	m_flTimeWeaponIdle = UTIL_GlobalTimeBase() + fDrawTime + 0.5; //Make half-second delay beetwen draw and idle animation

	return TRUE;
}

BOOL CBasePlayerWeapon::DefaultReload(int iClipSize, int iAnim, float fDelay)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 && !gInfinitelyAmmo)
		return FALSE;

	int j = min (iClipSize - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);

	if (j == 0 && !gInfinitelyAmmo) return FALSE;

	m_pPlayer->m_flNextAttack = UTIL_GlobalTimeBase() + fDelay;

	SendWeaponAnim(iAnim);

	m_fInReload = TRUE;

	m_flTimeWeaponIdle = UTIL_GlobalTimeBase() + fDelay + 0.5;
	return TRUE;
}

BOOL CBasePlayerWeapon::PlayEmptySound(int iSoundType)
{
	//play custom empty sound
	//0 - default cock sound
	//1 - electro sound
	//2 - electro sound 2
	//3 - flashlight sound
	//4 - no sound
	//5 - shockroach angry

	if (m_iPlayEmptySound) {
		PLAYBACK_EVENT_FULL(0, m_pPlayer->edict(), m_usPlayEmptySound, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, iSoundType, 0, 0, 0);
		m_iPlayEmptySound = 0;
		return FALSE;
	}
	return FALSE;
}

//restore pev->body and thirdperson animation after save\load
void CBasePlayerWeapon::RestoreBody()
{
	//Global function. restored and just set pev->body & pev->skin 
	//after each save\load for weapons. For use - insert her in WeaponIdle()
	//e.g. see in glock.cpp - RestoreBody ("onehanded");

	if (!b_Restored)
	{         //calculate additional body for special effects
		MESSAGE_BEGIN(MSG_ONE, gmsgSetBody, NULL, m_pPlayer->pev);
		WRITE_BYTE(pev->body); //weaponmodel body
		MESSAGE_END();

		//restore idle animation and hands position
		m_flTimeWeaponIdle = UTIL_GlobalTimeBase();

		//saved in CBasePlayer
		//strcpy( m_pPlayer->m_szAnimExtention, szAnimExt );

		b_Restored = TRUE;//reset after next save/load
	}

	//update weapon skin
	if (m_iLastSkin != pev->skin)
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgSetSkin, NULL, m_pPlayer->pev);
		WRITE_BYTE(pev->skin); //weaponmodel skin.
		MESSAGE_END();
		m_iLastSkin = pev->skin;
	}
}

void CBasePlayerWeapon::ResetEmptySound()
{
	m_iPlayEmptySound = 1;//reset empty sound
}

//=========================================================
//=========================================================
int CBasePlayerWeapon::PrimaryAmmoIndex()
{
	return m_iPrimaryAmmoType;
}

//=========================================================
//=========================================================
int CBasePlayerWeapon::SecondaryAmmoIndex()
{
	return -1;
}

void CBasePlayerWeapon::Holster()
{
	m_fInReload = FALSE; // cancel any reload in progress.
	m_pPlayer->pev->viewmodel = 0;
	m_pPlayer->pev->weaponmodel = 0;
}

void CBasePlayerAmmo::Spawn()
{
	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_TRIGGER;
	UTIL_SetSize(pev, Vector(-16, -16, 0), Vector(16, 16, 16));
	UTIL_SetOrigin(this, pev->origin);

	SetTouch(&CBasePlayerAmmo::DefaultTouch);
}

CBaseEntity* CBasePlayerAmmo::Respawn()
{
	pev->effects |= EF_NODRAW;
	SetTouch(NULL);

	UTIL_SetOrigin(this, g_pGameRules->VecAmmoRespawnSpot(this));// move to wherever I'm supposed to repawn.

	SetThink(&CBasePlayerAmmo::Materialize);
	AbsoluteNextThink(g_pGameRules->FlAmmoRespawnTime(this));

	return this;
}

void CBasePlayerAmmo::Materialize()
{
	if (pev->effects & EF_NODRAW)
	{
		// changing from invisible state to visible.
		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "items/suitchargeok1.wav", VOL_NORM, ATTN_NORM, 0, 150);
		pev->effects &= ~EF_NODRAW;
		pev->effects |= EF_MUZZLEFLASH;
	}

	SetTouch(&CBasePlayerAmmo::DefaultTouch);
}

void CBasePlayerAmmo::DefaultTouch(CBaseEntity *pOther)
{
	if (!pOther->IsPlayer())
	{
		return;
	}

	if (!UTIL_IsMasterTriggered(m_sMaster, m_pPlayer))	//
		return;										// AJH allows for locked weapons

	if (AddAmmo(pOther))
	{
		if (g_pGameRules->AmmoShouldRespawn(this) == GR_AMMO_RESPAWN_YES)
		{
			Respawn();
		}
		else
		{
			SetTouch(NULL);
			SetThink(&CBasePlayerAmmo::SUB_Remove);
			SetNextThink(0.1);
		}
		SUB_UseTargets(pOther, USE_TOGGLE, 0);	//AJH now ammo can trigger stuff too
	}
	else if (gEvilImpulse101)
	{
		// evil impulse 101 hack, kill always
		SetTouch(NULL);
		SetThink(&CBasePlayerAmmo::SUB_Remove);
		SetNextThink(0.1);
	}
}

//=========================================================
// called by the new item with the existing item as parameter
//
// if we call ExtractAmmo(), it's because the player is picking up this type of weapon for 
// the first time. If it is spawned by the world, m_iDefaultAmmo will have a default ammo amount in it.
// if  this is a weapon dropped by a dying player, has 0 m_iDefaultAmmo, which means only the ammo in 
// the weapon clip comes along. 
//=========================================================
bool CBasePlayerWeapon::ExtractAmmo(CBasePlayerWeapon *pWeapon)
{
	bool iReturn = false;

	if (pszAmmo1() != NULL)
	{
		// blindly call with m_iDefaultAmmo. It's either going to be a value or zero. If it is zero,
		// we only get the ammo in the weapon's clip, which is what we want. 
		iReturn = pWeapon->AddPrimaryAmmo(m_iDefaultAmmo, (char *)pszAmmo1(), iMaxClip(), iMaxAmmo1());
		m_iDefaultAmmo = 0;
	}

	if (pszAmmo2() != NULL)
		iReturn = pWeapon->AddSecondaryAmmo(0, (char *)pszAmmo2(), iMaxAmmo2());

	return iReturn;
}

//=========================================================
// called by the new item's class with the existing item as parameter
//=========================================================
int CBasePlayerWeapon::ExtractClipAmmo(CBasePlayerWeapon *pWeapon)
{
	int	iAmmo;

	if (m_iClip == WEAPON_NOCLIP)
	{
		iAmmo = 0;// guns with no clips always come empty if they are second-hand
	}
	else
	{
		iAmmo = m_iClip;
	}

	return pWeapon->m_pPlayer->GiveAmmo(iAmmo, (char *)pszAmmo1(), iMaxAmmo1()); // , &m_iPrimaryAmmoType
}

//=========================================================
// RetireWeapon - no more ammo for this gun, put it away.
//=========================================================
void CBasePlayerWeapon::RetireWeapon()
{
	Holster();
	// first, no viewmodel at all.
	m_pPlayer->pev->viewmodel = iStringNull;
	m_pPlayer->pev->weaponmodel = iStringNull;
	//m_pPlayer->pev->viewmodelindex = NULL;

	g_pGameRules->GetNextBestWeapon(m_pPlayer, this);
}

//=========================================================
// LRC - remove the specified ammo from this gun
//=========================================================
void CBasePlayerWeapon::DrainClip(CBasePlayer* pPlayer, BOOL keep, int i9mm, int i357, int iBuck, int iBolt, int iARGren, int iRock, int iUranium, int iSatchel, int iSnark, int iTrip, int iGren, int iShock, int iSpore)
{
	int iPAI = PrimaryAmmoIndex();
	int iAmt;
	if (iPAI == -1) return;
	else if (iPAI == pPlayer->GetAmmoIndex("9mm"))			iAmt = i9mm;
	else if (iPAI == pPlayer->GetAmmoIndex("357"))			iAmt = i357;
	else if (iPAI == pPlayer->GetAmmoIndex("buckshot"))		iAmt = iBuck;
	else if (iPAI == pPlayer->GetAmmoIndex("bolts"))		iAmt = iBolt;
	else if (iPAI == pPlayer->GetAmmoIndex("ARgrenades"))	iAmt = iARGren;
	else if (iPAI == pPlayer->GetAmmoIndex("uranium"))		iAmt = iUranium;
	else if (iPAI == pPlayer->GetAmmoIndex("rockets"))		iAmt = iRock;
	else if (iPAI == pPlayer->GetAmmoIndex("Satchel Charge")) iAmt = iSatchel;
	else if (iPAI == pPlayer->GetAmmoIndex("Snarks"))		iAmt = iSnark;
	else if (iPAI == pPlayer->GetAmmoIndex("Trip Mine"))	iAmt = iTrip;
	else if (iPAI == pPlayer->GetAmmoIndex("Hand Grenade")) iAmt = iGren;
	else if (iPAI == pPlayer->GetAmmoIndex("shocks"))		iAmt = iShock;
	else if (iPAI == pPlayer->GetAmmoIndex("spore"))		iAmt = iSpore;
	else return;

	if (iAmt > 0)
	{
		m_iClip -= iAmt;
		if (m_iClip < 0) m_iClip = 0;
	}
	else if (iAmt >= -1)
	{
		m_iClip = 0;
	}

	// if we're not keeping the gun, transfer the remainder of its clip
	// into the main ammo store
	if (!keep)
		pPlayer->m_rgAmmo[iPAI] = m_iClip;
}

//*********************************************************
// weaponbox code:
//*********************************************************

LINK_ENTITY_TO_CLASS(weaponbox, CWeaponBox);

TYPEDESCRIPTION	CWeaponBox::m_SaveData[] =
{
	DEFINE_ARRAY(CWeaponBox, m_rgAmmo, FIELD_INTEGER, MAX_AMMO_SLOTS),
	DEFINE_ARRAY(CWeaponBox, m_rgiszAmmo, FIELD_STRING, MAX_AMMO_SLOTS),
	DEFINE_ARRAY(CWeaponBox, m_rgpPlayerItems, FIELD_CLASSPTR, MAX_ITEM_TYPES),
	DEFINE_FIELD(CWeaponBox, m_cAmmoTypes, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CWeaponBox, CBaseEntity);

//=========================================================
//
//=========================================================
void CWeaponBox::Precache()
{
	PRECACHE_MODEL("models/w_weaponbox.mdl");
}

//=========================================================
//=========================================================
void CWeaponBox::KeyValue(KeyValueData *pkvd)
{
	if (m_cAmmoTypes < MAX_AMMO_SLOTS)
	{
		PackAmmo(ALLOC_STRING(pkvd->szKeyName), atoi(pkvd->szValue));
		m_cAmmoTypes++;// count this new ammo type.

		pkvd->fHandled = TRUE;
	}
	else
	{
		ALERT(at_debug, "WeaponBox too full! only %d ammotypes allowed\n", MAX_AMMO_SLOTS);
	}
}

//=========================================================
// CWeaponBox - Spawn 
//=========================================================
void CWeaponBox::Spawn()
{
	Precache();

	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_TRIGGER;

	UTIL_SetSize(pev, g_vecZero, g_vecZero);

	SET_MODEL(ENT(pev), "models/w_weaponbox.mdl");
}

//=========================================================
// CWeaponBox - Kill - the think function that removes the
// box from the world.
//=========================================================
void CWeaponBox::Kill()
{
	// destroy the weapons
	for (int i = 0; i < MAX_ITEM_TYPES; i++)
	{
		CBasePlayerItem* pWeapon = m_rgpPlayerItems[i];

		while (pWeapon)
		{
			pWeapon->SetThink(&CBasePlayerItem::SUB_Remove);
			pWeapon->SetNextThink(0.1);
			pWeapon = pWeapon->m_pNext;
		}
	}

	// remove the box
	UTIL_Remove(this);
}

//=========================================================
// CWeaponBox - Touch: try to add my contents to the toucher
// if the toucher is a player.
//=========================================================
void CWeaponBox::Touch(CBaseEntity *pOther)
{
	if (!(pev->flags & FL_ONGROUND))
	{
		return;
	}

	if (!pOther->IsPlayer())
	{
		// only players may touch a weaponbox.
		return;
	}

	if (!pOther->IsAlive())
	{
		// no dead guys.
		return;
	}

	CBasePlayer *pPlayer = (CBasePlayer *)pOther;
	int i;

	// dole out ammo
	for (i = 0; i < MAX_AMMO_SLOTS; i++)
	{
		if (!FStringNull(m_rgiszAmmo[i]))
		{
			// there's some ammo of this type. 
			pPlayer->GiveAmmo(m_rgAmmo[i], (char *)STRING(m_rgiszAmmo[i]), MaxAmmoCarry(m_rgiszAmmo[i]));

			//ALERT ( at_console, "Gave %d rounds of %s\n", m_rgAmmo[i], STRING(m_rgiszAmmo[i]) );

			// now empty the ammo from the weaponbox since we just gave it to the player
			m_rgiszAmmo[i] = iStringNull;
			m_rgAmmo[i] = 0;
		}
	}

	// go through my weapons and try to give the usable ones to the player. 
	// it's important the the player be given ammo first, so the weapons code doesn't refuse 
	// to deploy a better weapon that the player may pick up because he has no ammo for it.
	for (i = 0; i < MAX_ITEM_TYPES; i++)
	{
		if (m_rgpPlayerItems[i])
		{
			CBasePlayerItem *pItem;

			// have at least one weapon in this slot
			while (m_rgpPlayerItems[i])
			{
				//ALERT ( at_console, "trying to give %s\n", STRING( m_rgpPlayerItems[ i ]->pev->classname ) );

				pItem = m_rgpPlayerItems[i];
				m_rgpPlayerItems[i] = m_rgpPlayerItems[i]->m_pNext;// unlink this weapon from the box

				if (pPlayer->AddPlayerItem(pItem))
				{
					pItem->AttachToPlayer(pPlayer);
				}
			}
		}
	}

	EMIT_SOUND(pOther->edict(), CHAN_ITEM, "items/gunpickup2.wav", VOL_NORM, ATTN_NORM);
	SetTouch(NULL);
	UTIL_Remove(this);
}

//=========================================================
// CWeaponBox - PackWeapon: Add this weapon to the box
//=========================================================
BOOL CWeaponBox::PackWeapon(CBasePlayerItem *pWeapon)
{
	// is one of these weapons already packed in this box?
	if (HasWeapon(pWeapon))
	{
		return FALSE;// box can only hold one of each weapon type
	}

	if (pWeapon->m_pPlayer)
	{
		if (!pWeapon->m_pPlayer->RemovePlayerItem(pWeapon))
		{
			// failed to unhook the weapon from the player!
			return FALSE;
		}
	}

	int iWeaponSlot = pWeapon->iItemSlot();

	if (m_rgpPlayerItems[iWeaponSlot])
	{
		// there's already one weapon in this slot, so link this into the slot's column
		pWeapon->m_pNext = m_rgpPlayerItems[iWeaponSlot];
		m_rgpPlayerItems[iWeaponSlot] = pWeapon;
	}
	else
	{
		// first weapon we have for this slot
		m_rgpPlayerItems[iWeaponSlot] = pWeapon;
		pWeapon->m_pNext = NULL;
	}

	pWeapon->pev->spawnflags |= SF_NORESPAWN;// never respawn
	pWeapon->pev->movetype = MOVETYPE_NONE;
	pWeapon->pev->solid = SOLID_NOT;
	pWeapon->pev->effects = EF_NODRAW;
	pWeapon->pev->modelindex = 0;
	pWeapon->pev->model = iStringNull;
	pWeapon->pev->owner = edict();
	pWeapon->SetThink(NULL);// crowbar may be trying to swing again, etc.
	pWeapon->SetTouch(NULL);
	pWeapon->m_pPlayer = NULL;

	//ALERT ( at_console, "packed %s\n", STRING(pWeapon->pev->classname) );

	return TRUE;
}

//=========================================================
// CWeaponBox - PackAmmo
//=========================================================
BOOL CWeaponBox::PackAmmo(int iszName, int iCount)
{
	int iMaxCarry;

	if (FStringNull(iszName))
	{
		// error here
		ALERT(at_debug, "NULL String in PackAmmo!\n");
		return FALSE;
	}

	iMaxCarry = MaxAmmoCarry(iszName);

	if (iMaxCarry != -1 && iCount > 0)
	{
		//ALERT ( at_console, "Packed %d rounds of %s\n", iCount, STRING(iszName) );
		GiveAmmo(iCount, (char *)STRING(iszName), iMaxCarry);
		return TRUE;
	}

	return FALSE;
}

//=========================================================
// CWeaponBox - GiveAmmo
//=========================================================
int CWeaponBox::GiveAmmo(int iCount, char *szName, int iMax, int *pIndex/* = NULL*/)
{
	int i;

	for (i = 1; i < MAX_AMMO_SLOTS && !FStringNull(m_rgiszAmmo[i]); i++)
	{
		if (_stricmp(szName, STRING(m_rgiszAmmo[i])) == 0)
		{
			if (pIndex)
				*pIndex = i;

			int iAdd = min(iCount, iMax - m_rgAmmo[i]);
			if (iCount == 0 || iAdd > 0)
			{
				m_rgAmmo[i] += iAdd;

				return i;
			}
			return -1;
		}
	}
	if (i < MAX_AMMO_SLOTS)
	{
		if (pIndex)
			*pIndex = i;

		m_rgiszAmmo[i] = MAKE_STRING(szName);
		m_rgAmmo[i] = iCount;

		return i;
	}
	ALERT(at_debug, "out of named ammo slots\n");
	return i;
}

//=========================================================
// CWeaponBox::HasWeapon - is a weapon of this type already
// packed in this box?
//=========================================================
BOOL CWeaponBox::HasWeapon(CBasePlayerItem *pCheckItem)
{
	CBasePlayerItem *pItem = m_rgpPlayerItems[pCheckItem->iItemSlot()];

	while (pItem)
	{
		if (FClassnameIs(pItem->pev, STRING(pCheckItem->pev->classname)))
		{
			return TRUE;
		}
		pItem = pItem->m_pNext;
	}

	return FALSE;
}

//=========================================================
// CWeaponBox::IsEmpty - is there anything in this box?
//=========================================================
BOOL CWeaponBox::IsEmpty()
{
	int i;

	for (i = 0; i < MAX_ITEM_TYPES; i++)
	{
		if (m_rgpPlayerItems[i])
		{
			return FALSE;
		}
	}

	for (i = 0; i < MAX_AMMO_SLOTS; i++)
	{
		if (!FStringNull(m_rgiszAmmo[i]))
		{
			// still have a bit of this type of ammo
			return FALSE;
		}
	}

	return TRUE;
}

//=========================================================
//=========================================================
void CWeaponBox::SetObjectCollisionBox()
{
	pev->absmin = pev->origin + Vector(-16, -16, 0);
	pev->absmax = pev->origin + Vector(16, 16, 16);
}


void CBasePlayerWeapon::PrintState()
{
	ALERT(at_debug, "primary:  %f\n", m_flNextPrimaryAttack);
	ALERT(at_debug, "idle   :  %f\n", m_flTimeWeaponIdle);

	//	ALERT( at_debug, "nextrl :  %f\n", m_flNextReload );
	//	ALERT( at_debug, "nextpum:  %f\n", m_flPumpTime );

	//	ALERT( at_debug, "m_frt  :  %f\n", m_fReloadTime );
	ALERT(at_debug, "m_finre:  %i\n", m_fInReload);
	//	ALERT( at_debug, "m_finsr:  %i\n", m_fInSpecialReload );

	ALERT(at_debug, "m_iclip:  %i\n", m_iClip);
}

//Used by crowbar,knife & pipewrench
void FindHullIntersection(const Vector &vecSrc, TraceResult &tr, float *mins, float *maxs, edict_t *pEntity) {
	int			i, j, k;
	float		distance;
	float		*minmaxs[2] = { mins, maxs };
	TraceResult tmpTrace;
	Vector		vecHullEnd = tr.vecEndPos;
	Vector		vecEnd;

	distance = 1e6f;

	vecHullEnd = vecSrc + ((vecHullEnd - vecSrc) * 2);
	UTIL_TraceLine(vecSrc, vecHullEnd, dont_ignore_monsters, pEntity, &tmpTrace);
	if (tmpTrace.flFraction < 1.0) {
		tr = tmpTrace;
		return;
	}

	for (i = 0; i < 2; i++) {
		for (j = 0; j < 2; j++) {
			for (k = 0; k < 2; k++) {
				vecEnd.x = vecHullEnd.x + minmaxs[i][0];
				vecEnd.y = vecHullEnd.y + minmaxs[j][1];
				vecEnd.z = vecHullEnd.z + minmaxs[k][2];

				UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, pEntity, &tmpTrace);
				if (tmpTrace.flFraction < 1.0) {
					float thisDistance = (tmpTrace.vecEndPos - vecSrc).Length();
					if (thisDistance < distance) {
						tr = tmpTrace;
						distance = thisDistance;
					}
				}
			}
		}
	}
}

//Calculate the Weapons sequence length in frames / fps
float CalculateWeaponTime(float frames, float fps) {
	float CalculateTime = 0.0;
	return CalculateTime = (frames / fps);
}