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

#ifndef BASEMONSTER_H
#define BASEMONSTER_H

//
// generic Monster
//
class CBaseMonster : public CBaseToggle
{
private:
	int					m_afConditions;

public:
	typedef enum
	{
		SCRIPT_PLAYING = 0,		// Playing the sequence
		SCRIPT_WAIT,				// Waiting on everyone in the script to be ready
		SCRIPT_CLEANUP,					// Cancelling the script / cleaning up
		SCRIPT_WALK_TO_MARK,
		SCRIPT_RUN_TO_MARK,
	} SCRIPTSTATE;



	// these fields have been added in the process of reworking the state machine. (sjb)
	EHANDLE				m_hEnemy;		 // the entity that the monster is fighting.
	EHANDLE				m_hTargetEnt;	 // the entity that the monster is trying to reach
	EHANDLE				m_hOldEnemy[MAX_OLD_ENEMIES];
	Vector				m_vecOldEnemy[MAX_OLD_ENEMIES];

	float				m_flFieldOfView;// width of monster's field of view ( dot product )
	float				m_flWaitFinished;// if we're told to wait, this is the time that the wait will be over.
	float				m_flMoveWaitFinished;

	Activity			m_Activity;// what the monster is doing (animation)
	Activity			m_IdealActivity;// monster should switch to this activity

	int					m_LastHitGroup; // the last body region that took damage

	MONSTERSTATE		m_MonsterState;// monster's current state
	MONSTERSTATE		m_IdealMonsterState;// monster should change to this state

	int					m_iTaskStatus;
	Schedule_t			*m_pSchedule;
	int					m_iScheduleIndex;

	WayPoint_t			m_Route[ROUTE_SIZE];	// Positions of movement
	int					m_movementGoal;			// Goal that defines route
	int					m_iRouteIndex;			// index into m_Route[]
	float				m_moveWaitTime;			// How long I should wait for something to move

	Vector				m_vecMoveGoal; // kept around for node graph moves, so we know our ultimate goal
	Activity			m_movementActivity;	// When moving, set this activity

	int					m_iAudibleList; // first index of a linked list of sounds that the monster can hear.
	int					m_afSoundTypes;

	Vector				m_vecLastPosition;// monster sometimes wants to return to where it started after an operation.

	int					m_iHintNode; // this is the hint node that the monster is moving towards or performing active idle on.

	int					m_afMemory;

	int					m_iMaxHealth;// keeps track of monster's maximum health value (for re-healing, etc)

	Vector				m_vecEnemyLKP;// last known position of enemy. (enemy's origin)

	int					m_cAmmoLoaded;		// how much ammo is in the weapon (used to trigger reload anim sequences)

	int					m_afCapability;// tells us what a monster can/can't do.

	float				m_flNextAttack;		// cannot attack again until this time

	int					m_bitsDamageType;	// what types of damage has monster (player) taken
	BYTE				m_rgbTimeBasedDamage[CDMG_TIMEBASED];

	int					m_lastDamageAmount;// how much damage did monster (player) last take
											// time based damage counters, decr. 1 per 2 seconds
	int					m_bloodColor;		// color of blood particless

	int					m_failSchedule;				// Schedule type to choose if current schedule fails

	float				m_flHungryTime;// set this is a future time to stop the monster from eating for a while. 

	float				m_flDistTooFar;	// if enemy farther away than this, bits_COND_ENEMY_TOOFAR set in CheckEnemy
	float				m_flDistLook;	// distance monster sees (Default 2048)

	int					m_iTriggerCondition;// for scripted AI, this is the condition that will cause the activation of the monster's TriggerTarget
	string_t			m_iszTriggerTarget;// name of target that should be fired. 

	Vector				m_HackedGunPos;	// HACK until we can query end of gun

// Scripted sequence Info
	SCRIPTSTATE			m_scriptState;		// internal cinematic state
	CCineMonster		*m_pCine;

	virtual int		Save(CSave &save);
	virtual int		Restore(CRestore &restore);

	virtual STATE GetState() { return (pev->deadflag == DEAD_DEAD) ? STATE_OFF : STATE_ON; };

	static	TYPEDESCRIPTION m_SaveData[];

	void KeyValue(KeyValueData *pkvd);

	// monster use function
	void EXPORT			MonsterUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void EXPORT			CorpseUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

	// overrideable Monster member functions

		// LRC- to allow level-designers to change monster allegiances
	int					m_iClass;
	int					m_iPlayerReact;
	virtual int			Classify() { return m_iClass ? m_iClass : CLASS_NONE; }

	virtual int	 BloodColor() { return m_bloodColor; }

	virtual CBaseMonster *MyMonsterPointer() { return this; }
	virtual void Look(int iDistance);// basic sight function for monsters
	virtual void RunAI();// core ai function!	
	void Listen();

	virtual BOOL	IsAlive() { return (pev->deadflag != DEAD_DEAD); }
	virtual BOOL	ShouldFadeOnDeath();

	// Basic Monster AI functions
	virtual float ChangeYaw(int speed);
	float VecToYaw(Vector vecDir);
	float FlYawDiff();

	float DamageForce(float damage);

	// stuff written for new state machine
	virtual void MonsterThink();
	void EXPORT	CallMonsterThink() { this->MonsterThink(); }
	virtual int IRelationship(CBaseEntity *pTarget);
	virtual void MonsterInit();
	virtual void MonsterInitDead();	// Call after animation/pose is set up
	virtual void BecomeDead();
	void EXPORT CorpseFallThink();

	void EXPORT MonsterInitThink();
	virtual void StartMonster();
	virtual CBaseEntity* BestVisibleEnemy();// finds best visible enemy for attack
	virtual BOOL FInViewCone(CBaseEntity *pEntity);// see if pEntity is in monster's view cone
	virtual BOOL FInViewCone(Vector *pOrigin);// see if given location is in monster's view cone
	virtual void HandleAnimEvent(MonsterEvent_t *pEvent);

	virtual int CheckLocalMove(const Vector &vecStart, const Vector &vecEnd, CBaseEntity *pTarget, float *pflDist);// check validity of a straight move through space
	virtual void Move(float flInterval = 0.1);
	virtual void MoveExecute(CBaseEntity *pTargetEnt, const Vector &vecDir, float flInterval);
	virtual BOOL ShouldAdvanceRoute(float flWaypointDist);

	virtual Activity GetStoppedActivity() { return ACT_IDLE; }
	virtual void Stop() { m_IdealActivity = GetStoppedActivity(); }

	// This will stop animation until you call ResetSequenceInfo() at some point in the future
	inline void StopAnimation() { pev->framerate = 0; }

	// these functions will survey conditions and set appropriate conditions bits for attack types.
	virtual bool CheckRangeAttack1(float flDot, float flDist);
	virtual bool CheckRangeAttack2(float flDot, float flDist);
	virtual bool CheckMeleeAttack1(float flDot, float flDist);
	virtual bool CheckMeleeAttack2(float flDot, float flDist);

	BOOL FHaveSchedule();
	BOOL FScheduleValid();
	void ClearSchedule();
	BOOL FScheduleDone();
	void ChangeSchedule(Schedule_t *pNewSchedule);
	void NextScheduledTask();
	Schedule_t *ScheduleInList(const char *pName, Schedule_t **pList, int listCount);

	virtual Schedule_t *ScheduleFromName(const char *pName);
	static Schedule_t *m_scheduleList[];

	void MaintainSchedule();
	virtual void StartTask(Task_t *pTask);
	virtual void RunTask(Task_t *pTask);
	virtual Schedule_t *GetScheduleOfType(int Type);
	virtual Schedule_t *GetSchedule();
	virtual void ScheduleChange() {}
	//		virtual int CanPlaySequence( void ) { return ((m_pCine == NULL) && (m_MonsterState == MONSTERSTATE_NONE || m_MonsterState == MONSTERSTATE_IDLE || m_IdealMonsterState == MONSTERSTATE_IDLE)); }
	virtual int CanPlaySequence(int interruptFlags);
	//		virtual int CanPlaySequence( BOOL fDisregardState, int interruptLevel );
	virtual int CanPlaySentence(BOOL fDisregardState) { return IsAlive(); }
	virtual void PlaySentence(const char *pszSentence, float duration, float volume, float attenuation);
	virtual void PlayScriptedSentence(const char *pszSentence, float duration, float volume, float attenuation, BOOL bConcurrent, CBaseEntity *pListener);

	virtual void SentenceStop();

	Task_t *GetTask();
	virtual MONSTERSTATE GetIdealState();
	virtual void SetActivity(Activity NewActivity);
	void SetSequenceByName(char *szSequence);
	void SetState(MONSTERSTATE State);
	virtual void ReportAIState();

	void CheckAttacks(CBaseEntity *pTarget, float flDist);
	virtual int CheckEnemy(CBaseEntity *pEnemy);
	void PushEnemy(CBaseEntity *pEnemy, Vector &vecLastKnownPos);
	BOOL PopEnemy();

	BOOL FGetNodeRoute(Vector vecDest);

	inline void TaskComplete() { if (!HasConditions(bits_COND_TASK_FAILED)) m_iTaskStatus = TASKSTATUS_COMPLETE; }
	void MovementComplete();
	inline void TaskFail() { SetConditions(bits_COND_TASK_FAILED); }
	inline void TaskBegin() { m_iTaskStatus = TASKSTATUS_RUNNING; }
	int TaskIsRunning();
	inline int TaskIsComplete() { return (m_iTaskStatus == TASKSTATUS_COMPLETE); }
	inline int MovementIsComplete() { return (m_movementGoal == MOVEGOAL_NONE); }

	int IScheduleFlags();
	BOOL FRefreshRoute();
	BOOL FRouteClear();
	void RouteSimplify(CBaseEntity *pTargetEnt);
	void AdvanceRoute(float distance);
	virtual BOOL FTriangulate(const Vector &vecStart, const Vector &vecEnd, float flDist, CBaseEntity *pTargetEnt, Vector *pApex);
	void MakeIdealYaw(Vector vecTarget);
	virtual void SetYawSpeed() { return; };// allows different yaw_speeds for each activity
	BOOL BuildRoute(const Vector &vecGoal, int iMoveFlag, CBaseEntity *pTarget);
	virtual BOOL BuildNearestRoute(Vector vecThreat, Vector vecViewOffset, float flMinDist, float flMaxDist);
	int RouteClassify(int iMoveFlag);
	void InsertWaypoint(Vector vecLocation, int afMoveFlags);

	BOOL FindLateralCover(const Vector &vecThreat, const Vector &vecViewOffset);
	virtual BOOL FindCover(Vector vecThreat, Vector vecViewOffset, float flMinDist, float flMaxDist);
	virtual BOOL FValidateCover(const Vector &vecCoverLocation) { return TRUE; };
	virtual float CoverRadius() { return 784; } // Default cover radius

	virtual BOOL FCanCheckAttacks();
	virtual void CheckAmmo() { return; };
	virtual int IgnoreConditions();

	inline void	SetConditions(int iConditions) { m_afConditions |= iConditions; }
	inline void	ClearConditions(int iConditions) { m_afConditions &= ~iConditions; }
	inline BOOL HasConditions(int iConditions) { if (m_afConditions & iConditions) return TRUE; return FALSE; }
	inline BOOL HasAllConditions(int iConditions) { if ((m_afConditions & iConditions) == iConditions) return TRUE; return FALSE; }

	virtual BOOL FValidateHintType(short sHint);
	int FindHintNode();
	virtual BOOL FCanActiveIdle();
	void SetTurnActivity();
	float FLSoundVolume(CSound *pSound);

	BOOL MoveToNode(Activity movementAct, float waitTime, const Vector &goal);
	BOOL MoveToTarget(Activity movementAct, float waitTime);
	BOOL MoveToLocation(Activity movementAct, float waitTime, const Vector &goal);
	BOOL MoveToEnemy(Activity movementAct, float waitTime);

	// Returns the time when the door will be open
	float	OpenDoorAndWait(entvars_t *pevDoor);

	virtual int ISoundMask();
	virtual CSound* PBestSound();
	virtual CSound* PBestScent();
	virtual float HearingSensitivity() { return 1.0; };

	BOOL FBecomeProne();
	virtual void BarnacleVictimBitten(entvars_t *pevBarnacle);
	virtual void BarnacleVictimReleased();

	void SetEyePosition();

	BOOL FShouldEat();// see if a monster is 'hungry'
	void Eat(float flFullDuration);// make the monster 'full' for a while.

	CBaseEntity *CheckTraceHullAttack(float flDist, int iDamage, int iDmgType);
	BOOL FacingIdeal();

	BOOL FCheckAITrigger();// checks and, if necessary, fires the monster's trigger target. 
	BOOL NoFriendlyFire();

	BOOL BBoxFlat();

	// PrescheduleThink 
	virtual void PrescheduleThink() { return; };

	BOOL GetEnemy();
	void MakeDamageBloodDecal(int cCount, float flNoise, TraceResult *ptr, const Vector &vecDir);
	void TraceAttack(entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);

	// combat functions
	float UpdateTarget(entvars_t *pevTarget);
	virtual Activity GetDeathActivity();
	Activity GetSmallFlinchActivity();
	virtual void Killed(entvars_t *pevAttacker, int iGib);
	virtual void GibMonster();
	BOOL		 ShouldGibMonster(int iGib);
	void		 CallGibMonster();
	virtual int		HasCustomGibs() { return FALSE; } //LRC
	virtual BOOL	HasHumanGibs();
	virtual BOOL	HasAlienGibs();
	virtual void	FadeMonster();	// Called instead of GibMonster() when gibs are disabled

	Vector ShootAtEnemy(const Vector &shootOrigin);
	virtual Vector BodyTarget(const Vector &posSrc) { return Center() * 0.75 + EyePosition() * 0.25; };		// position to shoot at

	virtual	Vector  GetGunPosition();

	virtual int TakeHealth(float flHealth, int bitsDamageType);
	virtual int TakeArmor(float flArmor);
	virtual int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);
	int			DeadTakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType);

	void RadiusDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType);
	void RadiusDamage(Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType);
	virtual int		IsMoving() { return m_movementGoal != MOVEGOAL_NONE; }

	void RouteClear();
	void RouteNew();

	virtual void DeathSound() { return; };
	virtual void AlertSound() { return; };
	virtual void IdleSound() { return; };
	virtual void PainSound() { return; };
	virtual void StepSound();
	virtual void AttackSound() { return; };

	virtual void StopFollowing(bool clearSchedule) {}

	inline void	Remember(int iMemory) { m_afMemory |= iMemory; }
	inline void	Forget(int iMemory) { m_afMemory &= ~iMemory; }
	inline BOOL HasMemory(int iMemory) { if (m_afMemory & iMemory) return TRUE; return FALSE; }
	inline BOOL HasAllMemories(int iMemory) { if ((m_afMemory & iMemory) == iMemory) return TRUE; return FALSE; }

	BOOL ExitScriptedSequence();
	BOOL CineCleanup();

	void StartPatrol(CBaseEntity *path);

	CBaseEntity* DropItem(char *pszItemName, const Vector &vecPos, const Vector &vecAng);// drop an item.

	//LRC
	virtual float	CalcRatio(CBaseEntity *pLocus, int mode)//AJH added 'mode' = ratio to return
	{
		//ALERT(at_console, "monster CR: %f/%f = %f\n", pev->health, pev->max_health, pev->health / pev->max_health);
		switch (mode) {	//AJH pretty trivial switch statement! Add more cases later.
		case 1: {
			return pev->velocity.Length();
		}break;
		}
		return pev->health / pev->max_health;
	}

	//
	// Glowshell effects
	//
	void	GlowShellOn(Vector color, float flDuration);

	void	GlowShellOff();
	void	GlowShellUpdate();

	float	m_glowShellStartTime;
	float	m_glowShellDuration;
	Vector  m_glowShellColor;
	BOOL	m_glowShellUpdate;

	Vector	m_prevRenderColor;
	int		m_prevRenderFx;
	int		m_prevRenderMode;
	int		m_prevRenderAmt;
};



#endif // BASEMONSTER_H
