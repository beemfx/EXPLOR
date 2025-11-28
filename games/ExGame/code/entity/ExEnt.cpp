// (c) 2020 Beem Media. All Rights Reserved.

#include "ExEnt.h"
#include "ExGame.h"
#include "ExLookAtComponent.h"
#include "EGAmbientSoundComponent.h"

EG_CLASS_DECL( ExEnt )

const eg_string_crc ExEnt::PlayerControllerId = eg_crc( "PlayerController" );

void ExEnt::OnCreate( const egEntCreateParms& Parms )
{
	Super::OnCreate( Parms );

	if( GetRole() == eg_ent_role::Authority && Parms.Reason != eg_spawn_reason::GameLoad )
	{
		m_InitId = *Parms.InitString;
	}
}

eg_bool ExEnt::SendMsg( ex_notify_t Note , eg_ent_id SrcEnt )
{
	unused( Note , SrcEnt );
	return true;
}

void ExEnt::AssignToPlayer( const eg_lockstep_id& InLockstepId )
{
	m_LockstepId = InLockstepId;
}

void ExEnt::HandlePlayerLookAt( ExPlayerController* PlayerEnt ) const
{
	const ExLookAtComponent* LookAtComp = FindComponentByClass<ExLookAtComponent>();
	if( LookAtComp && PlayerEnt )
	{
		LookAtComp->ApplyLookAtToPlayer( PlayerEnt );
	}
}

void ExEnt::SetAmbientVolume( eg_real NewValue )
{
	if( EGAmbientSoundComponent* AmbCmp = FindComponentByClass<EGAmbientSoundComponent>() )
	{
		AmbCmp->SetVolume( NewValue );
	}
}

const ExGame* ExEnt::GetGameData() const
{
	return EGCast<ExGame>( SDK_GetGame() );
}

ExGame* ExEnt::GetGameData()
{
	return EGCast<ExGame>( SDK_GetGame() );
}

ExEnt* ExEnt::GetEnt( const eg_ent_id& Id )
{
	return EGCast<ExEnt>( SDK_GetGame()->SDK_GetEnt( Id ) );
}
