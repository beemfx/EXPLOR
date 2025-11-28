// (c) 2020 Beem Media. All Rights Reserved.

#include "ExMapTransitionEnt.h"
#include "ExGame.h"
#include "ExPlayerController.h"
#include "EGComponent.h"
#include "EGVisualComponent.h"

EG_CLASS_DECL( ExMapTransitionEnt )

void ExMapTransitionEnt::OnCreate(const egEntCreateParms& Parms)
{
	Super::OnCreate(Parms);

	if( GetRole() == eg_ent_role::Authority )
	{
		if( Parms.Reason == eg_spawn_reason::Spawned )
		{
			ParseInitString( Parms.InitString );
		}
	}

	SetActive(false);
}

void ExMapTransitionEnt::OnEnterWorld()
{
	Super::OnEnterWorld();

	if( IsServer() )
	{
		if( ExGame* Game = GetGameData() )
		{
			Game->SetTrackedMapTransitionEnt( this , true );
		}
	}
}

void ExMapTransitionEnt::OnLeaveWorld()
{
	if( IsServer() )
	{
		if( ExGame* Game = GetGameData() )
		{
			Game->SetTrackedMapTransitionEnt( this , false );
		}
	}

	Super::OnLeaveWorld();
}

void ExMapTransitionEnt::ParseInitString( const eg_d_string& InitString )
{
	EGParse_ProcessFnCallScript( *InitString , InitString.Len() , [this]( const egParseFuncInfo& InfoStr )->void
	{
		if( EGString_EqualsI(InfoStr.FunctionName , "SetId") && InfoStr.NumParms >= 1 )
		{
			m_TransitionId = eg_string_crc( InfoStr.Parms[0] );
		}
		else if( EGString_EqualsI( InfoStr.FunctionName , "SetScript" ) && InfoStr.NumParms >= 1 )
		{
			EGArray<eg_d_string> Script = EGString_Split<eg_d_string>( InfoStr.Parms[0] , '.' , 2 );

			m_ScriptId = Script.IsValidIndex(0) ? eg_string_crc(*Script[0]) : CT_Clear;
			m_ScriptEntry = Script.IsValidIndex(1) ? eg_string_crc(*Script[1]) : CT_Clear;
		}
	} );
}
