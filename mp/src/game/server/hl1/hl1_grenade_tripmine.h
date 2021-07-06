//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Tripmine
//
// $NoKeywords: $
//=============================================================================//

#ifndef	GRENADETRIP_H
#define	GRENADETRIP_H

#include "cbase.h"
#include "hl1_basecombatweapon_shared.h"
#include "basecombatcharacter.h"
#include "soundent.h"
#include "game.h"
#include "hl1_player.h"
#include "hl1_basegrenade.h"
#include "beam_shared.h"

#define TRIPMINE_MODEL "models/w_tripmine.mdl"
#define TRIPMINE_BEAM_SPRITE "sprites/laserbeam.vmt"

extern ConVar sk_plr_dmg_tripmine;



class CTripmineGrenade : public CHL1BaseGrenade
{
	DECLARE_CLASS(CTripmineGrenade, CHL1BaseGrenade);
public:
	CTripmineGrenade();
	void		Spawn(void);
	void		Precache(void);

	int			OnTakeDamage_Alive(const CTakeDamageInfo &info);

	void		WarningThink(void);
	void		PowerupThink(void);
	void		BeamBreakThink(void);
	void		DelayDeathThink(void);
	void		Event_Killed(const CTakeDamageInfo &info);

	CHandle<CBaseEntity>	m_hRealOwner;

	DECLARE_DATADESC();

private:
	void			MakeBeam(void);
	void			KillBeam(void);

private:
	float					m_flPowerUp;
	Vector					m_vecDir;
	Vector					m_vecEnd;
	float					m_flBeamLength;

	CHandle<CBeam>			m_hBeam;

	CHandle<CBaseEntity>	m_hStuckOn;
	Vector					m_posStuckOn;
	QAngle					m_angStuckOn;

	int						m_iLaserModel;
};

LINK_ENTITY_TO_CLASS(monster_tripmine, CTripmineGrenade);

#endif // GRENADETRIP_H