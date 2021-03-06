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
// skill.h - skill level concerns
//=========================================================

struct skilldata_t
{

	int iSkillLevel; // game skill level

// Monster Health & Damage
	float	agruntHealth;
	float agruntDmgPunch;

	float apacheHealth;
	float ospreyHealth;

	float apacheBlkopHealth;
	float ospreyBlkopHealth;

	//Otis
	float otisHealth;
	float otisHead;
	float otisChest;
	float otisStomach;
	float otisLeg;
	float otisArm;

	//Barney
	float barneyHealth;
	float barneyHead;
	float barneyChest;
	float barneyStomach;
	float barneyLeg;
	float barneyArm;

	//Barniel
	float barnielHealth;
	float barnielHead;
	float barnielChest;
	float barnielStomach;
	float barnielLeg;
	float barnielArm;

	float bigmommaHealthFactor;		// Multiply each node's health by this
	float bigmommaDmgSlash;			// melee attack damage
	float bigmommaDmgBlast;			// mortar attack damage
	float bigmommaRadiusBlast;		// mortar attack radius

	float bullsquidHealth;
	float bullsquidDmgBite;
	float bullsquidDmgWhip;
	float bullsquidDmgSpit;

	float gargantuaHealth;
	float gargantuaDmgSlash;
	float gargantuaDmgFire;
	float gargantuaDmgStomp;

	float hassassinHealth;

	float headcrabHealth;
	float headcrabDmgBite;

	float hgruntHealth;
	float hgruntDmgKick;
	float hgruntShotgunPellets;
	float hgruntGrenadeSpeed;

	float houndeyeHealth;
	float houndeyeDmgBlast;

	float slaveHealth;
	float slaveDmgClaw;
	float slaveDmgClawrake;
	float slaveDmgZap;

	float ichthyosaurHealth;
	float ichthyosaurDmgShake;

	float leechHealth;
	float leechDmgBite;

	float controllerHealth;
	float controllerDmgZap;
	float controllerSpeedBall;
	float controllerDmgBall;

	float nihilanthHealth;
	float nihilanthZap;

	float scientistHealth;

	//Construction
	float constructionHealth;
	float constructionHead;
	float constructionChest;
	float constructionStomach;
	float constructionLeg;
	float constructionArm;

	float snarkHealth;
	float snarkDmgBite;
	float snarkDmgPop;

	//Diablo
	float diabloHealth;
	float diabloDmgOneSlash;
	float diabloDmgBothSlash;
	float diabloHead;
	float diabloChest;
	float diabloStomach;
	float diabloLeg;

	//=========================================================
	// NPCs: Zombie,Zombie Soldier,Zombie Barney
	// For Spirit of Half-Life v1.9: Opposing-Force Edition
	//=========================================================

	// Zombie Scientist
	float zombieHealth;
	float zombieDmgOneSlash;
	float zombieDmgBothSlash;
	float zombieHead;
	float zombieChest;
	float zombieStomach;
	float zombieLeg;
	float zombieArm;

	// Zombie Soldier
	float zombieSoldierHealth;
	float zombieSoldierDmgOneSlash;
	float zombieSoldierDmgBothSlash;
	float zombieSoldierHead;
	float zombieSoldierChest;
	float zombieSoldierStomach;
	float zombieSoldierLeg;
	float zombieSoldierArm;

	// Zombie Construction
	float zombieConstructionHealth;
	float zombieConstructionDmgOneSlash;
	float zombieConstructionDmgBothSlash;
	float zombieConstructionHead;
	float zombieConstructionChest;
	float zombieConstructionStomach;
	float zombieConstructionLeg;
	float zombieConstructionArm;

	// Zombie Barney
	float zombieBarneyHealth;
	float zombieBarneyDmgOneSlash;
	float zombieBarneyDmgBothSlash;
	float zombieBarneyHead;
	float zombieBarneyChest;
	float zombieBarneyStomach;
	float zombieBarneyLeg;
	float zombieBarneyArm;

	// Pit Drone
	float pitdroneHealth;
	float pitdroneDmgBite;
	float pitdroneDmgWhip;
	float pitdroneDmgSpit;
	float pitdroneHead;
	float pitdroneChest;
	float pitdroneStomach;
	float pitdroneLeg;
	float pitdroneArm;

	// Voltigore
	float voltigoreHealth;
	float voltigoreDmgPunch;
	float voltigoreDmgBeam;
	float voltigoreHead;
	float voltigoreChest;
	float voltigoreStomach;
	float voltigoreLeg;
	float voltigoreArm;

	// Opposing-Force
	float fgruntHealth;
	float fgruntDmgKick;
	float fgruntShotgunPellets;
	float fgruntGrenadeSpeed;
	float fgruntHead;
	float fgruntChest;
	float fgruntStomach;
	float fgruntArm;
	float fgruntLeg;

	float medicHealth;
	float medicDmgKick;
	float medicHeal;

	float torchHealth;
	float torchDmgKick;

	float pwormHealth;
	float pwormDmgSwipe;
	float pwormDmgBeam;

	float sroachHealth;
	float sroachDmgBite;
	float sroachLifespan;

	float gonomeHealth;
	float gonomeDmgOneSlash;
	float gonomeDmgGuts;
	float gonomeDmgOneBite;

	float strooperHealth;
	float strooperDmgKick;
	float strooperGrenadeSpeed;
	float strooperMaxCharge;
	float strooperRchgSpeed;

	float turretHealth;
	float miniturretHealth;
	float sentryHealth;

	// Player Weapons
	float plrDmgCrowbar;
	float plrDmgWrench;
	float plrDmgKnife;
	float plrDmgKnifeCharge;
	float plrDmg9MM;
	float plrDmg357;
	float plrDmg762;
	float plrDmgMP5;
	float plrDmgM203Grenade;
	float plrDmgBuckshot;
	float plrDmgCrossbowClient;
	float plrDmgCrossbowMonster;
	float plrDmgRPG;
	float plrDmgGauss;
	float plrDmgEgonNarrow;
	float plrDmgEgonWide;
	float plrDmgHornet;
	float plrDmgHandGrenade;
	float plrDmgSatchel;
	float plrDmgTripmine;
	float plrDmgM249;
	float plrDmgShock;
	float plrDmgSpore;

	// weapons shared by monsters
	float monDmg9MM;
	float monDmgMP5;
	float monDmg12MM;
	float monDmgHornet;
	float monDmgM249;
	float monDmgShockroach;

	// health/suit charge
	float suitchargerCapacity;
	float batteryCapacity;
	float healthchargerCapacity;
	float healthkitCapacity;
	float flashlightCharge;
	float scientistHeal;

	// monster damage adj
	float monHead;
	float monChest;
	float monStomach;
	float monLeg;
	float monArm;

	// player damage adj
	float plrHead;
	float plrChest;
	float plrStomach;
	float plrLeg;
	float plrArm;
};

extern	DLL_GLOBAL	skilldata_t	gSkillData;
float GetSkillCvar(char *pName);

extern DLL_GLOBAL int		g_iSkillLevel;

#define SKILL_EASY		1
#define SKILL_MEDIUM	2
#define SKILL_HARD		3
