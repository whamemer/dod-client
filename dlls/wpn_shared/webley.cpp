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
****/
//
// webley.cpp
//
// implementation of CWEBLEY class
//

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "dod_gamerules.h"

#include "dod_shared.h"

LINK_ENTITY_TO_CLASS( weapon_webley, CWEBLEY )

extern struct p_wpninfo_s P_WpnInfo[];

enum WEBLEY_e
{
    WEBLEY_IDLE = 0,
    WEBLEY_SHOOT,
    WEBLEY_RELOAD,
    WEBLEY_DRAW
};

void CWEBLEY::Spawn( void )
{
    Precache();

    m_iId = WEAPON_WEBLEY;
    SET_MODEL( ENT( pev ), P_WpnInfo[WEAPON_WEBLEY].wmodel );

    m_iDefaultAmmo = P_WpnInfo[WEAPON_WEBLEY].ammo_default;

    FallInit();
}

void CWEBLEY::Precache( void )
{
    PRECACHE_MODEL( P_WpnInfo[WEAPON_WEBLEY].vmodel );
    PRECACHE_MODEL( P_WpnInfo[WEAPON_WEBLEY].wmodel );

    PRECACHE_SOUND( "weapons/webley_shoot.wav" );
    PRECACHE_SOUND( "weapons/357_cock1.wav" );
    PRECACHE_SOUND( "weapons/colt_reload_clipin.wav" );
    PRECACHE_SOUND( "weapons/colt_reload_clipout.wav" );
    PRECACHE_SOUND( "weapons/colt_draw_pullslide.wav" );
    
    m_usFireWebley = PRECACHE_EVENT( 1, "events/weapons/webley.sc" );
}

int CWEBLEY::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = "ammo_12mm";
    p->iMaxAmmo1 = P_WpnInfo[WEAPON_WEBLEY].ammo_maxcarry;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = P_WpnInfo[WEAPON_WEBLEY].ammo_maxclip;
    p->iSlot = 1;
    p->iPosition = 0;
    p->iFlags = ITEM_FLAG_PISTOL;
    p->iId = WEAPON_WEBLEY;
    p->iWeight = P_WpnInfo[WEAPON_WEBLEY].misc_weight;
    return 1;
}

void CWEBLEY::PrimaryAttack( void )
{
    if( PlayerIsWaterSniping() )
    {
        PlayEmptySound();
        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15f;
    }
    else if( !m_fInAttack )
    {
        if( m_iClip <= 0 )
        {
            PlayEmptySound();
            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15f;
            m_fInAttack = 1;
        }
        else
        {
            m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

            ItemInfo sz[40];
            GetItemInfo( sz );

            float flSpread;
            entvars_t *pevAttacker = m_pPlayer->pev;
            int shared_rand = m_pPlayer->random_seed;

            Vector vecSrc = m_pPlayer->GetGunPosition();
            vec3_t vecDirShooting = gpGlobals->v_forward;

            CBaseEntity::FireBulletsNC( (Vector *)m_pPlayer, vecSrc, vecDirShooting, flSpread, 8192.0f, BULLET_PLAYER_WEBLEY, 3, 0, pevAttacker, shared_rand );

            HUD_PlaybackEvent( 1, ENT( m_pPlayer->pev ), m_usFireWebley, 0.0f, g_vecZero, g_vecZero, 0, 0, 0, 0, m_iClip == 0, 0 );

            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + P_WpnInfo[WEAPON_WEBLEY].anim_firedelay;
            m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.1f;
            m_fInAttack = 1;
            m_flTimeWeaponIdle = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10.0f, 15.0f ) + UTIL_WeaponTimeBase();
            
            RemoveStamina( 0.0f, m_pPlayer );
        }
    }
}

BOOL CWEBLEY::Deploy( void )
{
    m_pPlayer->m_iFOV = ZoomOut();
    UpdateZoomSpeed();

    m_pPlayer->CheckPlayerSpeed();

    float idleTime = P_WpnInfo[m_iId].anim_drawtime;

    return TimedDeploy( P_WpnInfo[WEAPON_WEBLEY].vmodel, P_WpnInfo[WEAPON_WEBLEY].pmodel, WEBLEY_DRAW, P_WpnInfo[WEAPON_WEBLEY].szAnimExt, P_WpnInfo[WEAPON_WEBLEY].szAnimReloadExt, P_WpnInfo[WEAPON_WEBLEY].anim_drawtime, 1.0f, 0 );
}

void CWEBLEY::Reload( void )
{
    DefaultReload( P_WpnInfo[WEAPON_WEBLEY].ammo_maxclip, WEBLEY_RELOAD, P_WpnInfo[WEAPON_WEBLEY].anim_reloadtime );
}

void CWEBLEY::WeaponIdle( void )
{
    ResetEmptySound();

    if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
    {
        SendWeaponAnim( WEBLEY_IDLE );
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5f;
    }
}

class CWebleyAmmoClip : public CBasePlayerAmmo
{
    void Spawn( void )
    {
        SET_MODEL( ENT( pev ), "models/allied_ammo.mdl" );
        CBasePlayerAmmo::Spawn();
    }

    BOOL AddAmmo( CBaseEntity *pOther )
    {
		int bResult = ( pOther->GiveAmmo( P_WpnInfo[WEAPON_WEBLEY].ammo_maxclip, "ammo_12mm", P_WpnInfo[WEAPON_WEBLEY].ammo_maxcarry ) != -1 );
		if( bResult )
		{
			EMIT_SOUND_DYN( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM, 0, 100 );
		}
		return bResult;
    }
};

LINK_ENTITY_TO_CLASS( ammo_webley, CWebleyAmmoClip )