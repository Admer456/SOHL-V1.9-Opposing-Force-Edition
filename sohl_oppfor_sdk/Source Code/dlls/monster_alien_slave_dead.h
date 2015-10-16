/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/
#ifndef MONSTER_ALIEN_SLAVE_DEAD_H
#define MONSTER_ALIEN_SLAVE_DEAD_H

class CDeadISlave : public CBaseMonster
{
public:
	void Spawn(void);
	int	Classify(void) { return	CLASS_ALIEN_MONSTER; }

	void KeyValue(KeyValueData *pkvd);

	int	m_iPose;// which sequence to display
	static char *m_szPoses[1];
};

#endif // MONSTER_ALIEN_SLAVE_DEAD_H