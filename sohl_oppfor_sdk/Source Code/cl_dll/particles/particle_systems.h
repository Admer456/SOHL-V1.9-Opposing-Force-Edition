/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
*	This product contains software technology licensed from:
*	The Battle Grounds Team and Contributors.
*
****/
#ifndef PARTICLE_SYSTEMS_H
#define PARTICLE_SYSTEMS_H

// base particle system
class CParticleSystem  
{
protected:
	// time of the last particle system draw
	float m_flLastDraw;

	// the id of the particle
	int	m_iID;
	// how long the system has been alive for
	float m_flSystemsAge;
	// the age when the grim reaper will come for this particle system
	float m_flSystemMaxAge;
	// number of particles to be created in the beginning
	unsigned int m_iStartingParticles;
	// the number of new particles we want to create after the system has begin
	float m_flNewParticles;
	// the delay in creating single particles after creation
	// they should all be created at equal intervals, but due to varying frame rates this doesn't
	// happen.  this variables allows us to try how much more time we have to
	// add new particles that should have been created between frames
	float m_flParticleCreationTime;

	// centre of the particle system
	vec3_t m_vPosition;
	// the particle system is moving in which direction?
	vec3_t m_vDirection;

	// pointer to the current texture
	particle_texture_s *pParticleTexture;

	// returns the length of time between this draw and the last one
	inline float TimeSinceLastDraw( void ) const { return (gEngfuncs.GetClientTime() - m_flLastDraw); }

	// returns the number of particles to be created on creation of this system
	unsigned int StartingParticles( void );
	// returns the percentages of particles to be created now
	float NewParticlesCreationDelay( void );
	// tests whether this system should be deleted or not
	virtual inline bool TestSystem( void );
	// updates the counters in the system
	virtual void UpdateSystem( void );

	// adds a particle to this system
	virtual inline void AddParticle( CParticle *pParticle );
public:
	// draw all associated particles
	virtual bool DrawSystem();
	// returns the id of a ps
	virtual unsigned int SystemID( void ) const { return m_iID; }

	// creates a particle system
	CParticleSystem( );
	// destroys a particle system
	virtual ~CParticleSystem( );
};

// flintlock smoke system
class CFlintlockSmokeParticleSystem : public CParticleSystem
{
	friend class CParticleSystemManager;
	CFlintlockSmokeParticleSystem(vec3_t vPosition);
};

// barrel smoke system
class CBarrelSmokeParticleSystem : public CParticleSystem
{
	particle_texture_s *pParticleTextures[NUM_DIFFERENT_BARREL_PARTICLES];
	friend class CParticleSystemManager;
	CBarrelSmokeParticleSystem(vec3_t vPosition, vec3_t vDirection);
	virtual void UpdateSystem( void );
};

// spark particle system
class CSparkParticleSystem : public CParticleSystem
{
	friend class CParticleSystemManager;
	CSparkParticleSystem(vec3_t vPosition, vec3_t vDirection);
	virtual void UpdateSystem( void );
};

// white smoke system
class CWhiteSmokeParticleSystem : public CParticleSystem
{
	friend class CParticleSystemManager;
	CWhiteSmokeParticleSystem(vec3_t vPosition, vec3_t vDirection);
};

// white smoke system
class CBrownSmokeParticleSystem : public CParticleSystem
{
	friend class CParticleSystemManager;
	CBrownSmokeParticleSystem(vec3_t vPosition, vec3_t vDirection);
};
class CMuzzleFlashParticleSystem : public CParticleSystem
{
	friend class CParticleSystemManager;
	CMuzzleFlashParticleSystem(vec3_t vPosition, vec3_t vDirection, int iType);
};

// creates a particle system based on what the mapper wants
class CMappedParticleSystem : public CParticleSystem
{
	friend class CParticleSystemManager;

	// all the details about this system
	mapped_particle_system *m_pSystem;
	// the file from which all our settings are loaded
	char *m_sParticleFile;

	// give all values in the system defaults before loading the file
	void CreateDefaultParticleSystem( void );
	// parse the defintion file
	bool LoadParticleDefinition( void );
protected:
	virtual inline bool TestSystem( void );
	virtual void UpdateSystem( void );
public:
	CMappedParticleSystem( char *sParticleDefinition, particle_system_management *pSysDetails );
	~CMappedParticleSystem();
};

// creates a block of grass for the mapper
class CGrassParticleSystem : public CParticleSystem
{
	friend class CParticleSystemManager;

	// the file from which all our settings are loaded
	char *m_sParticleFile;
	// container for all particle types
	// all particles of that type are contained within
	vector<grass_particle_types*> m_cGrassTypes;

	// give all values in the system defaults before loading the file
	grass_particle_system *CreateDefaultParticleSystem( void );
	// parse the defintion file
	bool LoadParticleDefinition( particle_system_management *pSysDetails );
protected:
	// tests whether this system is ready to die
	virtual inline bool TestSystem( void ) { return !(m_flSystemMaxAge == 0.01); }
	// adds a new particle to this system
	inline void AddParticle( CParticle *pParticle, grass_particle_types *pGrassType );
public:
	CGrassParticleSystem( char *sParticleDefinition, particle_system_management *pSysDetails);
	~CGrassParticleSystem();
};
#endif
