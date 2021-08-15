//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Tripmine
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "hl1_basecombatweapon_shared.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "soundent.h"
#include "game.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "hl1_player.h"
#include "hl1_basegrenade.h"
#include "beam_shared.h"
#include "hl1_grenade_tripmine.h"


//-----------------------------------------------------------------------------
// CWeaponTripMine
//-----------------------------------------------------------------------------

class CWeaponTripMine : public CBaseHL1CombatWeapon
{
	DECLARE_CLASS( CWeaponTripMine, CBaseHL1CombatWeapon );
public:

	CWeaponTripMine( void );

	void	Spawn( void );
	void	Precache( void );
	void	Equip( CBaseCombatCharacter *pOwner );
	void	PrimaryAttack( void );
	void	WeaponIdle( void );
	bool	Holster( CBaseCombatWeapon *pSwitchingTo = NULL );

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

private:
	int		m_iGroundIndex;
	int		m_iPickedUpIndex;
};

LINK_ENTITY_TO_CLASS( weapon_tripmine, CWeaponTripMine );

PRECACHE_WEAPON_REGISTER( weapon_tripmine );

IMPLEMENT_SERVERCLASS_ST( CWeaponTripMine, DT_WeaponTripMine )
END_SEND_TABLE()

BEGIN_DATADESC( CWeaponTripMine )
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponTripMine::CWeaponTripMine( void )
{
	m_bReloadsSingly	= false;
	m_bFiresUnderwater	= true;
}

void CWeaponTripMine::Spawn( void )
{
	BaseClass::Spawn();

	m_iWorldModelIndex = m_iGroundIndex;
	SetModel( TRIPMINE_MODEL );

	SetActivity( ACT_TRIPMINE_GROUND );
	ResetSequenceInfo( );
	m_flPlaybackRate = 0;

	if ( !g_pGameRules->IsDeathmatch() )
	{
		UTIL_SetSize( this, Vector(-16, -16, 0), Vector(16, 16, 28) ); 
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponTripMine::Precache( void )
{
	BaseClass::Precache();

	m_iGroundIndex		= PrecacheModel( TRIPMINE_MODEL );
	m_iPickedUpIndex	= PrecacheModel( GetWorldModel() );

	UTIL_PrecacheOther( "monster_tripmine" );
}

void CWeaponTripMine::Equip( CBaseCombatCharacter *pOwner )
{
	m_iWorldModelIndex = m_iPickedUpIndex;

	BaseClass::Equip( pOwner );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponTripMine::PrimaryAttack( void )
{
	CHL1_Player *pPlayer = ToHL1Player( GetOwner() );
	if ( !pPlayer )
	{
		return;
	}

	if ( pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
		return;

	Vector vecAiming	= pPlayer->GetAutoaimVector( 0 );
	Vector vecSrc		= pPlayer->Weapon_ShootPosition( );

	trace_t tr;

	UTIL_TraceLine( vecSrc, vecSrc + vecAiming * 64, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr );

	if ( tr.fraction < 1.0 )
	{
		CBaseEntity *pEntity = tr.m_pEnt;
		if ( pEntity && !( pEntity->GetFlags() & FL_CONVEYOR ) )
		{
			QAngle angles;
			VectorAngles( tr.plane.normal, angles );

			Create("monster_tripmine", tr.endpos + tr.plane.normal * 2, angles, pPlayer);

			pPlayer->RemoveAmmo( 1, m_iPrimaryAmmoType );

			pPlayer->SetAnimation( PLAYER_ATTACK1 );
			
			if ( pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
			{
				if ( !pPlayer->SwitchToNextBestWeapon( pPlayer->GetActiveWeapon() ) )
					Holster();
			}
			else
			{
				SendWeaponAnim( ACT_VM_DRAW );
				SetWeaponIdleTime( gpGlobals->curtime + random->RandomFloat( 10, 15 ) );
			}

			m_flNextPrimaryAttack = gpGlobals->curtime + 0.5;
			
			SetWeaponIdleTime( gpGlobals->curtime ); // MO curtime correct ?
		}
	}
	else
	{
		SetWeaponIdleTime( m_flTimeWeaponIdle = gpGlobals->curtime + random->RandomFloat( 10, 15 ) );
	}
}

void CWeaponTripMine::WeaponIdle( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if ( !pPlayer )
	{
		return;
	}

	if ( !HasWeaponIdleTimeElapsed() )
		return;

	int iAnim;

	if ( random->RandomFloat( 0, 1 ) <= 0.75 )
	{
		iAnim = ACT_VM_IDLE;
	}
	else
	{
		iAnim = ACT_VM_FIDGET;
	}

	SendWeaponAnim( iAnim );
}

bool CWeaponTripMine::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( !pPlayer )
	{
		return false;
	}

	if ( !BaseClass::Holster( pSwitchingTo ) )
	{
		return false;
	}

	if ( pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
	{
		SetThink( &CWeaponTripMine::DestroyItem );
		SetNextThink( gpGlobals->curtime + 0.1 );
	}

	pPlayer->SetNextAttack( gpGlobals->curtime + 0.5 );

	return true;
}