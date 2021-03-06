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
//=========================================================
// NPC: Barney * http://half-life.wikia.com/wiki/Barney_Calhoun
//=========================================================

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"monsters.h"
#include	"schedule.h"
#include	"defaultai.h"
#include	"scripted.h"
#include	"weapons.h"
#include	"soundent.h"
#include    "monster_barney.h"

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define	BARNEY_AE_DRAW		( 2 )
#define	BARNEY_AE_SHOOT		( 3 )
#define	BARNEY_AE_DISARM	( 4 )

#define	NUM_BARNEY_HEADS	3

#define	GUN_GROUP			1
#define	HEAD_GROUP			2

#define	HEAD_NORMAL			0
#define	HEAD_BLACK			1
#define	HEAD_JOE			2

#define	GUN_NONE			0
#define	GUN_DRAWN			1
#define	GUN_NO_GUN			2

//=========================================================
// Monster's link to Class & Saverestore Begins
//=========================================================
LINK_ENTITY_TO_CLASS(monster_barney, CBarney);

TYPEDESCRIPTION	CBarney::m_SaveData[] = {
	DEFINE_FIELD(CBarney, m_iBaseBody, FIELD_INTEGER), //LRC
	DEFINE_FIELD(CBarney, m_fGunDrawn, FIELD_BOOLEAN),
	DEFINE_FIELD(CBarney, m_painTime, FIELD_TIME),
	DEFINE_FIELD(CBarney, m_checkAttackTime, FIELD_TIME),
	DEFINE_FIELD(CBarney, m_lastAttackCheck, FIELD_BOOLEAN),
	DEFINE_FIELD(CBarney, m_flPlayerDamage, FIELD_FLOAT),
	DEFINE_FIELD(CBarney, head, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CBarney, CTalkMonster);

//=========================================================
// KeyValue
//=========================================================
void CBarney::KeyValue(KeyValueData *pkvd) {
	if (FStrEq(pkvd->szKeyName, "head")) {
		head = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CTalkMonster::KeyValue(pkvd);
}

//=========================================================
// Monster Sounds
//=========================================================
const char *CBarney::pPainSounds[] = {
	"barney/ba_pain1.wav",
	"barney/ba_pain2.wav",
	"barney/ba_pain3.wav"
};

const char *CBarney::pDeathSounds[] = {
	"barney/ba_die1.wav",
	"barney/ba_die2.wav",
	"barney/ba_die3.wav"
};

const char *CBarney::pAttackSounds[] = {
	"barney/ba_attack1.wav",
	"barney/ba_attack2.wav",
	"weapons/357_shot1.wav",
	"weapons/357_shot2.wav"
};

//=========================================================
// Spawn Barney
//=========================================================
void CBarney::Spawn() {
	Precache();

	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/barney.mdl");

	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BLOOD_COLOR_RED;

	if (pev->health == 0) //LRC
		pev->health = gSkillData.barneyHealth;

	pev->view_ofs = Vector(0, 0, 50);// position of the eyes relative to monster's origin.
	m_flFieldOfView = VIEW_FIELD_WIDE; // NOTE: we need a wide field of view so npc will notice player and say hello
	m_MonsterState = MONSTERSTATE_NONE;

	m_fGunDrawn = FALSE;

	m_afCapability = bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP;

	// Make sure hands are white.
	if (m_iBaseBody) {
		SetBodygroup(HEAD_GROUP, m_iBaseBody);
	}
	else {
		SetBodygroup(HEAD_GROUP, RANDOM_LONG(0, NUM_BARNEY_HEADS - 1));
	}

	if (head != -1 && !m_iBaseBody) {
		SetBodygroup(HEAD_GROUP, head);
	}

	m_flDebug = false; //Debug Massages

	m_flHitgroupHead = gSkillData.barneyHead;
	m_flHitgroupChest = gSkillData.barneyChest;
	m_flHitgroupStomach = gSkillData.barneyStomach;
	m_flHitgroupArm = gSkillData.barneyArm;
	m_flHitgroupLeg = gSkillData.barneyLeg;

	MonsterInit();
	SetUse(&CBarney::FollowerUse);
}

//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int	CBarney::Classify() {
	return m_iClass ? m_iClass : CLASS_PLAYER_ALLY;
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CBarney::Precache() {
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/barney.mdl");

	m_iBrassShell = PRECACHE_MODEL("models/shell.mdl");// brass shell

	PRECACHE_SOUND_ARRAY(pAttackSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);
	PRECACHE_SOUND_ARRAY(pDeathSounds);

	TalkInit();
	CTalkMonster::Precache();
}

//=========================================================
// TakeDamage
//=========================================================
int CBarney::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) {
	if (pev->spawnflags & SF_MONSTER_SPAWNFLAG_64) {
		if (m_flDebug)
			ALERT(at_console, "%s:TakeDamage:SF_MONSTER_SPAWNFLAG_64\n", STRING(pev->classname));

		CBaseEntity *pEnt = CBaseEntity::Instance(pevAttacker);
		if (pEnt->IsPlayer()) {
			pev->health = pev->max_health / 2;
			if (flDamage > 0) //Override all damage
				SetConditions(bits_COND_LIGHT_DAMAGE);

			if (flDamage >= 20) //Override all damage
				SetConditions(bits_COND_HEAVY_DAMAGE);

			return CTalkMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
		}

		if (pevAttacker->owner) {
			pEnt = CBaseEntity::Instance(pevAttacker->owner);
			if (pEnt->IsPlayer()) {
				pev->health = pev->max_health / 2;
				if (flDamage > 0) //Override all damage
					SetConditions(bits_COND_LIGHT_DAMAGE);

				if (flDamage >= 20) //Override all damage
					SetConditions(bits_COND_HEAVY_DAMAGE);

				return CTalkMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
			}
		}
	}

	if (!IsAlive() || pev->deadflag == DEAD_DYING || m_iPlayerReact) {
		return CTalkMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
	}

	if (m_MonsterState != MONSTERSTATE_PRONE && (pevAttacker->flags & FL_CLIENT)) {
		m_flPlayerDamage += flDamage;
		if (m_hEnemy == NULL) {
			if ((m_afMemory & bits_MEMORY_SUSPICIOUS) || UTIL_IsFacing(pevAttacker, pev->origin)) {
				if (m_iszSpeakAs) {
					char szBuf[32];
					strcpy(szBuf, STRING(m_iszSpeakAs));
					strcat(szBuf, "_MAD");
					PlaySentence(szBuf, 4, VOL_NORM, ATTN_NORM);
				}
				else {
					PlaySentence("BA_MAD", 4, VOL_NORM, ATTN_NORM);
				}

				Remember(bits_MEMORY_PROVOKED);
				StopFollowing(true);
			}
			else {
				if (m_iszSpeakAs) {
					char szBuf[32];
					strcpy(szBuf, STRING(m_iszSpeakAs));
					strcat(szBuf, "_SHOT");
					PlaySentence(szBuf, 4, VOL_NORM, ATTN_NORM);
				}
				else {
					PlaySentence("BA_SHOT", 4, VOL_NORM, ATTN_NORM);
				}
				Remember(bits_MEMORY_SUSPICIOUS);
			}
		}
		else if (!(m_hEnemy->IsPlayer()) && pev->deadflag == DEAD_NO) {
			if (m_iszSpeakAs) {
				char szBuf[32];
				strcpy(szBuf, STRING(m_iszSpeakAs));
				strcat(szBuf, "_SHOT");
				PlaySentence(szBuf, 4, VOL_NORM, ATTN_NORM);
			}
			else {
				PlaySentence("BA_SHOT", 4, VOL_NORM, ATTN_NORM);
			}
		}
	}

	return CTalkMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

//=========================================================
// TraceAttack - Damage based on Hitgroups
//=========================================================
void CBarney::TraceAttack(entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType) {
	if (!IsAlive()) {
		CTalkMonster::TraceAttack(pevAttacker, flDamage, vecDir, ptr, bitsDamageType);
		return;
	}

	if (pev->takedamage) {
		if (IsAlive() && RANDOM_LONG(0, 4) <= 2) { PainSound(); }
		if (pev->spawnflags & SF_MONSTER_SPAWNFLAG_64) {
			CBaseEntity *pEnt = CBaseEntity::Instance(pevAttacker);
			if (pEnt->IsPlayer()) { return; }
			if (pevAttacker->owner) {
				pEnt = CBaseEntity::Instance(pevAttacker->owner);
				if (pEnt->IsPlayer()) { return; }
			}
		}

		switch (ptr->iHitgroup) {
		case HITGROUP_HEAD_HELMET_BN:
		case HITGROUP_HEAD:
			if (m_flDebug)
				ALERT(at_console, "%s:TraceAttack:HITGROUP_HEAD\n", STRING(pev->classname));
			if (bitsDamageType & (DMG_BULLET | DMG_SLASH | DMG_CLUB)) {
				flDamage -= 20;
				if (flDamage <= 0) {
					UTIL_Ricochet(ptr->vecEndPos, 1.0);
					flDamage = 0.01;
				}
			}
			else {
				flDamage = m_flHitgroupHead * flDamage;
			}
			ptr->iHitgroup = HITGROUP_HEAD;
			break;
		case HITGROUP_CHEST:
			if (m_flDebug)
				ALERT(at_console, "%s:TraceAttack:HITGROUP_CHEST\n", STRING(pev->classname));
			if (bitsDamageType & (DMG_BULLET | DMG_SLASH | DMG_BLAST)) {
				flDamage = (m_flHitgroupChest*flDamage) / 2;
			}
			break;
		case HITGROUP_STOMACH:
			if (m_flDebug)
				ALERT(at_console, "%s:TraceAttack:HITGROUP_STOMACH\n", STRING(pev->classname));
			if (bitsDamageType & (DMG_BULLET | DMG_SLASH | DMG_BLAST)) {
				flDamage = (m_flHitgroupStomach*flDamage) / 2;
			}
			break;
		case HITGROUP_LEFTARM:
		case HITGROUP_RIGHTARM:
			if (m_flDebug)
				ALERT(at_console, "%s:TraceAttack:HITGROUP_ARM\n", STRING(pev->classname));
			flDamage = m_flHitgroupArm * flDamage;
			break;
		case HITGROUP_LEFTLEG:
		case HITGROUP_RIGHTLEG:
			if (m_flDebug)
				ALERT(at_console, "%s:TraceAttack:HITGROUP_LEG\n", STRING(pev->classname));
			flDamage = m_flHitgroupLeg * flDamage;
			break;
		}
	}

	SpawnBlood(ptr->vecEndPos, BloodColor(), flDamage);// a little surface blood.
	TraceBleed(flDamage, vecDir, ptr, bitsDamageType);
	CTalkMonster::TraceAttack(pevAttacker, flDamage, vecDir, ptr, bitsDamageType);
}

//=========================================================
// PainSound
//=========================================================
void CBarney::PainSound() {
	if (UTIL_GlobalTimeBase() < m_painTime)
		return;

	m_painTime = UTIL_GlobalTimeBase() + RANDOM_FLOAT(0.5, 0.75);
	EMIT_SOUND_ARRAY_DYN(CHAN_VOICE, pPainSounds);
}

//=========================================================
// DeathSound 
//=========================================================
void CBarney::DeathSound() {
	EMIT_SOUND_ARRAY_DYN(CHAN_VOICE, pDeathSounds);
}

//=========================================================
// AlertSound
//=========================================================
void CBarney::AlertSound() {
	if (m_hEnemy != NULL) {
		if (FOkToSpeak()) {
			if (m_iszSpeakAs) {
				char szBuf[32];
				strcpy(szBuf, STRING(m_iszSpeakAs));
				strcat(szBuf, "_ATTACK");
				PlaySentence(szBuf, RANDOM_FLOAT(2.8, 3.2), VOL_NORM, ATTN_IDLE);
			}
			else {
				PlaySentence("BA_ATTACK", RANDOM_FLOAT(2.8, 3.2), VOL_NORM, ATTN_IDLE);
			}
		}
	}
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CBarney::HandleAnimEvent(MonsterEvent_t *pEvent) {
	switch (pEvent->event) {
	case BARNEY_AE_SHOOT:
		FirePistol();
		break;
	case BARNEY_AE_DRAW:
		SetBodygroup(GUN_GROUP, GUN_DRAWN);
		m_fGunDrawn = TRUE;
		break;
	case BARNEY_AE_DISARM:
		SetBodygroup(GUN_GROUP, GUN_NONE);
		m_fGunDrawn = FALSE;
		break;
	default:
		CTalkMonster::HandleAnimEvent(pEvent);
		break;
	}
}

//=========================================================
// CheckRangeAttack1
//=========================================================
bool CBarney::CheckRangeAttack1(float flDot, float flDist) {
	if (flDist <= 1024 && flDot >= 0.5) {
		if (UTIL_GlobalTimeBase() > m_checkAttackTime) {
			TraceResult tr;
			Vector shootOrigin = pev->origin + Vector(0, 0, 55);
			CBaseEntity *pEnemy = m_hEnemy;
			Vector shootTarget = ((pEnemy->BodyTarget(shootOrigin) - pEnemy->pev->origin) + m_vecEnemyLKP);
			UTIL_TraceLine(shootOrigin, shootTarget, dont_ignore_monsters, ENT(pev), &tr);
			m_checkAttackTime = UTIL_GlobalTimeBase() + 1;

			if (tr.flFraction == 1.0 || (tr.pHit != NULL && CBaseEntity::Instance(tr.pHit) == pEnemy))
				m_lastAttackCheck = TRUE;
			else
				m_lastAttackCheck = FALSE;

			m_checkAttackTime = UTIL_GlobalTimeBase() + 1.5;
		}

		return (bool)m_lastAttackCheck;
	}

	return false;
}

//=========================================================
// Barney shoots one round from the pistol at 9mm
//=========================================================
void CBarney::FirePistol() {
	UTIL_MakeVectors(pev->angles);
	Vector vecShootOrigin = pev->origin + Vector(0, 0, 55);
	Vector vecShootDir = ShootAtEnemy(vecShootOrigin);

	Vector angDir = UTIL_VecToAngles(vecShootDir);
	SetBlending(0, angDir.x);
	pev->effects = EF_MUZZLEFLASH;

	int pitchShift = RANDOM_LONG(0, 20);
	if (pitchShift > 10)
		pitchShift = 0;
	else
		pitchShift -= 5;

	if (pev->frags) {
		FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_2DEGREES, 1024, BULLET_MONSTER_357);
		switch (RANDOM_LONG(0, 1)) {
		case 0: EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/357_shot1.wav", VOL_NORM, ATTN_NORM, 0, 100 + pitchShift); break;
		case 1: EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/357_shot2.wav", VOL_NORM, ATTN_NORM, 0, 100 + pitchShift); break;
		}
	}
	else {
		FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_2DEGREES, 1024, BULLET_MONSTER_9MM);
		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "barney/ba_attack2.wav", VOL_NORM, ATTN_NORM, 0, 100 + pitchShift);
	}

	Vector vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40, 90) + gpGlobals->v_up * RANDOM_FLOAT(75, 100) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
	EjectBrass(vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, m_iBrassShell, TE_BOUNCE_SHELL);
	WeaponFlash(vecShootOrigin);

	CSoundEnt::InsertSound(bits_SOUND_COMBAT, pev->origin, 384, 0.3);
	m_cAmmoLoaded--;
}

//=========================================================
// AI Schedules Specific to this monster
//=========================================================
Task_t	tlBaFollow[] = {
	{ TASK_MOVE_TO_TARGET_RANGE,(float)128		},	// Move within 128 of target ent (client)
	{ TASK_SET_SCHEDULE,		(float)SCHED_TARGET_FACE },
};

Schedule_t	slBaFollow[] = {
	{
		tlBaFollow,
		HL_ARRAYSIZE(tlBaFollow),
		bits_COND_NEW_ENEMY |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_HEAR_SOUND |
		bits_COND_PROVOKED,
		bits_SOUND_DANGER,
		"Follow"
	},
};

//=========================================================
// BarneyDraw- much better looking draw schedule for when
// barney knows who he's gonna attack.
//=========================================================
Task_t	tlBarneyEnemyDraw[] = {
	{ TASK_STOP_MOVING,					0				},
	{ TASK_FACE_ENEMY,					0				},
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,	(float)ACT_ARM },
};

Schedule_t slBarneyEnemyDraw[] = {
	{
		tlBarneyEnemyDraw,
		HL_ARRAYSIZE(tlBarneyEnemyDraw),
		0,
		0,
		"Barney Enemy Draw"
	}
};

Task_t	tlBaFaceTarget[] = {
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_FACE_TARGET,			(float)0		},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_SET_SCHEDULE,		(float)SCHED_TARGET_CHASE },
};

Schedule_t	slBaFaceTarget[] = {
	{
		tlBaFaceTarget,
		HL_ARRAYSIZE(tlBaFaceTarget),
		bits_COND_CLIENT_PUSH |
		bits_COND_NEW_ENEMY |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_HEAR_SOUND |
		bits_COND_PROVOKED,
		bits_SOUND_DANGER,
		"FaceTarget"
	},
};

Task_t	tlIdleBaStand[] = {
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT,				(float)2		}, // repick IDLESTAND every two seconds.
	{ TASK_TLK_HEADRESET,		(float)0		}, // reset head position
};

Schedule_t	slIdleBaStand[] = {
	{
		tlIdleBaStand,
		HL_ARRAYSIZE(tlIdleBaStand),
		bits_COND_NEW_ENEMY |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_HEAR_SOUND |
		bits_COND_SMELL |
		bits_COND_PROVOKED,
		bits_SOUND_COMBAT |
		bits_SOUND_DANGER |
		bits_SOUND_MEAT |
		bits_SOUND_CARCASS |
		bits_SOUND_GARBAGE,
		"IdleStand"
	},
};

DEFINE_CUSTOM_SCHEDULES(CBarney) {
	slBaFollow,
		slBarneyEnemyDraw,
		slBaFaceTarget,
		slIdleBaStand,
};


IMPLEMENT_CUSTOM_SCHEDULES(CBarney, CTalkMonster);

void CBarney::StartTask(Task_t *pTask) {
	CTalkMonster::StartTask(pTask);
}

void CBarney::RunTask(Task_t *pTask) {
	switch (pTask->iTask) {
	case TASK_RANGE_ATTACK1:
		if (m_hEnemy != NULL && (m_hEnemy->IsPlayer())) {
			pev->framerate = 1.5;
		}
		CTalkMonster::RunTask(pTask);
		break;
	default:
		CTalkMonster::RunTask(pTask);
		break;
	}
}

//=========================================================
// ISoundMask - returns a bit mask indicating which types
// of sounds this monster regards. 
//=========================================================
int CBarney::ISoundMask() {
	return	bits_SOUND_WORLD |
		bits_SOUND_COMBAT |
		bits_SOUND_CARCASS |
		bits_SOUND_MEAT |
		bits_SOUND_GARBAGE |
		bits_SOUND_DANGER |
		bits_SOUND_PLAYER;
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CBarney::SetYawSpeed() {
	int ys = 0;
	switch (m_Activity) {
	case ACT_IDLE:
		ys = 70;
		break;
	case ACT_WALK:
		ys = 70;
		break;
	case ACT_RUN:
		ys = 90;
		break;
	default:
		ys = 70;
		break;
	}

	pev->yaw_speed = ys;
}

//=========================================================
// Init talk data
//=========================================================
void CBarney::TalkInit() {
	CTalkMonster::TalkInit();

	if (!m_iszSpeakAs) {
		m_szGrp[TLK_ANSWER] = "BA_ANSWER";
		m_szGrp[TLK_QUESTION] = "BA_QUESTION";
		m_szGrp[TLK_IDLE] = "BA_IDLE";
		m_szGrp[TLK_STARE] = "BA_STARE";

		if (pev->spawnflags & SF_MONSTER_SPAWNFLAG_256) //LRC
			m_szGrp[TLK_USE] = "BA_PFOLLOW";
		else
			m_szGrp[TLK_USE] = "BA_OK";

		if (pev->spawnflags & SF_MONSTER_SPAWNFLAG_256)
			m_szGrp[TLK_UNUSE] = "BA_PWAIT";
		else
			m_szGrp[TLK_UNUSE] = "BA_WAIT";

		if (pev->spawnflags & SF_MONSTER_SPAWNFLAG_256)
			m_szGrp[TLK_DECLINE] = "BA_POK";
		else
			m_szGrp[TLK_DECLINE] = "BA_NOTOK";

		m_szGrp[TLK_STOP] = "BA_STOP";
		m_szGrp[TLK_NOSHOOT] = "BA_SCARED";
		m_szGrp[TLK_HELLO] = "BA_HELLO";
		m_szGrp[TLK_PLHURT1] = "!BA_CUREA";
		m_szGrp[TLK_PLHURT2] = "!BA_CUREB";
		m_szGrp[TLK_PLHURT3] = "!BA_CUREC";
		m_szGrp[TLK_PHELLO] = NULL;
		m_szGrp[TLK_PIDLE] = NULL;
		m_szGrp[TLK_PQUESTION] = "BA_PQUEST";
		m_szGrp[TLK_SMELL] = "BA_SMELL";
		m_szGrp[TLK_WOUND] = "BA_WOUND";
		m_szGrp[TLK_MORTAL] = "BA_MORTAL";
	}

	m_voicePitch = (95 + RANDOM_LONG(0, 10));
}

//=========================================================
// Monster is Killed, change body and drop weapon
//=========================================================
void CBarney::Killed(entvars_t *pevAttacker, int iGib) {
	if (pev->body < m_iBaseBody + GUN_DRAWN && !(pev->spawnflags & SF_MONSTER_SPAWNFLAG_1024)) {
		Vector vecGunPos, vecGunAngles;
		SetBodygroup(GUN_GROUP, GUN_NO_GUN);
		GetAttachment(0, vecGunPos, vecGunAngles);
		CBaseEntity *pGun = DropItem((pev->frags ? "weapon_357" : "weapon_9mmhandgun"), vecGunPos, vecGunAngles);
	}

	SetUse(NULL);
	CTalkMonster::Killed(pevAttacker, iGib);
}

//=========================================================
// AI Schedules Specific to this monster
//=========================================================
Schedule_t* CBarney::GetScheduleOfType(int Type) {
	Schedule_t *psched;
	switch (Type) {
	case SCHED_ARM_WEAPON:
		if (m_hEnemy != NULL) {
			return slBarneyEnemyDraw;
		}
		break;
	case SCHED_TARGET_FACE:
		psched = CTalkMonster::GetScheduleOfType(Type);
		if (psched == slIdleStand)
			return slBaFaceTarget;
		else
			return psched;
		break;
	case SCHED_TARGET_CHASE:
		return slBaFollow;
		break;
	case SCHED_IDLE_STAND:
		psched = CTalkMonster::GetScheduleOfType(Type);
		if (psched == slIdleStand) {
			return slIdleBaStand;
		}
		else
			return psched;
		break;
	}

	return CTalkMonster::GetScheduleOfType(Type);
}

//=========================================================
// GetSchedule - Decides which type of schedule best suits
// the monster's current state and conditions. Then calls
// monster's member function to get a pointer to a schedule
// of the proper type.
//=========================================================
Schedule_t *CBarney::GetSchedule() {
	if (HasConditions(bits_COND_HEAR_SOUND)) {
		CSound *pSound;
		pSound = PBestSound();
		ASSERT(pSound != NULL);
		if (pSound && (pSound->m_iType & bits_SOUND_DANGER))
			return GetScheduleOfType(SCHED_TAKE_COVER_FROM_BEST_SOUND);
	}

	if (HasConditions(bits_COND_ENEMY_DEAD) && FOkToSpeak()) {
		if (m_iszSpeakAs) {
			char szBuf[32];
			strcpy(szBuf, STRING(m_iszSpeakAs));
			strcat(szBuf, "_KILL");
			PlaySentence(szBuf, 4, VOL_NORM, ATTN_NORM);
		}
		else {
			PlaySentence("BA_KILL", 4, VOL_NORM, ATTN_NORM);
		}
	}

	switch (m_MonsterState) {
	case MONSTERSTATE_COMBAT:
		if (HasConditions(bits_COND_ENEMY_DEAD)) {
			return CBaseMonster::GetSchedule();
		}

		if (HasConditions(bits_COND_NEW_ENEMY) && HasConditions(bits_COND_LIGHT_DAMAGE))
			return GetScheduleOfType(SCHED_SMALL_FLINCH);

		if (!m_fGunDrawn)
			return GetScheduleOfType(SCHED_ARM_WEAPON);

		if (HasConditions(bits_COND_HEAVY_DAMAGE))
			return GetScheduleOfType(SCHED_TAKE_COVER_FROM_ENEMY);
		break;
	case MONSTERSTATE_ALERT:
	case MONSTERSTATE_IDLE:
		if (HasConditions(bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE)) {
			return GetScheduleOfType(SCHED_SMALL_FLINCH);
		}

		if (m_hEnemy == NULL && IsFollowing()) {
			if (!m_hTargetEnt->IsAlive()) {
				StopFollowing(false);
				break;
			}
			else {
				if (HasConditions(bits_COND_CLIENT_PUSH)) {
					return GetScheduleOfType(SCHED_MOVE_AWAY_FOLLOW);
				}

				return GetScheduleOfType(SCHED_TARGET_FACE);
			}
		}

		if (HasConditions(bits_COND_CLIENT_PUSH)) {
			return GetScheduleOfType(SCHED_MOVE_AWAY);
		}

		TrySmellTalk();
		break;
	}

	return CTalkMonster::GetSchedule();
}

//=========================================================
// Get IdealState for Monster
//=========================================================
MONSTERSTATE CBarney::GetIdealState() {
	return CTalkMonster::GetIdealState();
}

//=========================================================
// Decline Following from Monster
//=========================================================
void CBarney::DeclineFollowing() {
	PlaySentence(m_szGrp[TLK_DECLINE], 2, VOL_NORM, ATTN_NORM); //LRC
}