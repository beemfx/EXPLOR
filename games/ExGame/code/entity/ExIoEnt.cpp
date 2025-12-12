// (c) 2020 Beem Media. All Rights Reserved.

#include "ExIoEnt.h"
#include "ExNpcComponent.h"
#include "ExScriptExecComponent.h"
#include "ExGame.h"
#include "ExPlayerController.h"

EG_CLASS_DECL( ExIoEnt )

void ExIoEnt::OnCreate( const egEntCreateParms& Parms )
{
	Super::OnCreate( Parms );

	if( GetRole() == eg_ent_role::Authority )
	{
		if( Parms.Reason == eg_spawn_reason::Spawned )
		{
			ParseInitString( Parms.InitString );
		}
		Initialize();

		if( !(ExGame::PoseToFace(GetPose()) != ex_face::UNK && ExGame::PoseToFace(GetPose())  != ex_face::NONE) )
		{
			EGLogf( eg_log_t::Warning , "An ExIoEnt (%s) was spawned that wasn't facing a good direction." , *Parms.InitString );
		}
	}
	SetActive( false );
}

void ExIoEnt::OnEnterWorld()
{
	Super::OnEnterWorld();

	if( IsServer() )
	{
		if( ExNpcComponent* NpcComp = FindComponentByClass<ExNpcComponent>() )
		{
			EGArray<eg_d_string> InitData = EGString_Split<eg_d_string>( *NpcComp->GetScriptIdStr() , '.' , 1 );
			m_ScriptId = InitData.IsValidIndex(0) ? eg_string_crc(*InitData[0]) : CT_Clear;
			m_ScriptEntry = InitData.IsValidIndex(1) ? eg_string_crc(*InitData[1]) : CT_Clear;
			m_EncounterPriority = EG_Max(NpcComp->GetEncounterPriority(),m_EncounterPriority);
		}

		ex_face FaceToInteractAgainst = ExGame::PoseToFace( GetPose() );

		if( ExScriptExecComponent* ScriptComp = FindComponentByClass<ExScriptExecComponent>() )
		{
			if( ScriptComp->IsInteractionInAnyDirection() )
			{
				FaceToInteractAgainst = ex_face::ALL;
			}
			m_EncounterPriority = EG_Max(ScriptComp->GetEncounterPriority(),m_EncounterPriority);
		}

		GetGameData()->AddTrackedIoEnt( this , m_CurentNavVertex , FaceToInteractAgainst );
	}
}

void ExIoEnt::OnLeaveWorld()
{
	if( IsServer() )
	{
		GetGameData()->RemoveTrackedIoEnt( this );
	}
	
	Super::OnLeaveWorld();
}

void ExIoEnt::OnUpdate( eg_real DeltaTime )
{
	Super::OnUpdate( DeltaTime );

	if( IsServer() && m_bDespawnQueued )
	{
		m_DespawnCountdown -= DeltaTime;
		if( m_DespawnCountdown <= 0.f )
		{
			m_bDespawnQueued = false;
			DeleteFromWorld();
		}
	}
}

void ExIoEnt::OnEncounter()
{
	ExGame* GameData = GetGameData();
	GameData->RunScript( m_ScriptId , m_ScriptEntry );

	HandlePlayerLookAt( GameData->GetPlayerEnt() );
}

void ExIoEnt::HandleNpcDespawn( eg_real Duration , eg_string_crc Event )
{
	assert( IsServer() );
	m_bEncountersEnabled = false;
	m_DespawnCountdown = Duration;
	if( m_DespawnCountdown > EG_SMALL_NUMBER )
	{
		SetActive( true );
		m_bDespawnQueued = true;
		if( Event.IsNotNull() )
		{
			RunEvent( Event );
		}
	}
	else
	{
		DeleteFromWorld();
	}
}

void ExIoEnt::ParseInitString( const eg_d_string& InitString )
{
	if( InitString.Len() > 0 )
	{
		m_InitId = eg_string_crc(*InitString);
		EGArray<eg_d_string> InitData = EGString_Split<eg_d_string>( *InitString , '.' , 1 );
		if( InitData.Len() == 1 )
		{
			m_ScriptId = eg_string_crc(*InitData[0]);
			m_ScriptEntry = CT_Clear;
		}
		else if( InitData.Len() == 2 )
		{
			m_ScriptId = eg_string_crc(*InitData[0]);
			m_ScriptEntry = eg_string_crc(*InitData[1]);
		}
		else
		{
			assert( false );
			m_ScriptId = eg_string_crc(*InitString);
			m_ScriptEntry = CT_Clear;
		}
	}
	else
	{
		m_InitId = CT_Clear;
		m_ScriptId = CT_Clear;
		m_ScriptEntry = CT_Clear;
	}
}
