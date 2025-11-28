/******************************************************************************
ExPlayerController - Main controller for interacting with the maps.

(c) 2015 Beem Media
******************************************************************************/

#include "ExEnemyController.h"
#include "ExMapNavBase.h"
#include "ExGame.h"
#include "EGBoolList.h"
#include "ExServer.h"
#include "ExMonsterSpriteComponent.h"
#include "ExCombatEncounter.h"
#include "ExBeastiary.h"

EG_CLASS_DECL( ExEnemyController )

void ExEnemyController::OnCreate( const egEntCreateParms& Parms )
{
	Super::OnCreate( Parms );

	if( GetRole() == eg_ent_role::Authority )
	{
		ExGame* pGameData = GetGameData();
		if( Parms.Reason == eg_spawn_reason::Spawned || Parms.Reason == eg_spawn_reason::GameLoad )
		{
			pGameData->AddMonster( GetID() );
		}
		m_bCanMove = false;
		Initialize();
		SetActive( false );
	}

	UpdateDebugHitBoxVisibility();
}

void ExEnemyController::OnDestroy()
{
	if( GetRole() == eg_ent_role::Authority )
	{
		if( ExGame* pGameData = GetGameData() )
		{
			pGameData->RemoveMonster( GetID() );
		}
	}

	Super::OnDestroy();
}

void ExEnemyController::OnUpdate( eg_real DeltaTime )
{
	unused( DeltaTime );

	if( IsServer() )
	{
		ExGame* pGameData = GetGameData();

		// We want the turn animation to complete a little before the turn is over.
		eg_real t = EG_Clamp( ( pGameData->GetTurnTime() + .02f ) / EX_TURN_TIME, 0.f, 1.f );

		eg_transform FinalPose = eg_transform::Lerp( m_StartMovePose, m_EndMovePose, t );
		SetPose( FinalPose );
		ReplicateChangedData( &m_RepData, sizeof(m_RepData) );
	}
}

void ExEnemyController::OnEnterWorld()
{
	Super::OnEnterWorld();

	if( IsServer() )
	{
		m_RepDataChecksum = CT_Clear;
		ReplicateChangedData( &m_RepData, sizeof(m_RepData) );
	}
}

void ExEnemyController::OnDataReplicated( const void* Offset , eg_size_t Size )
{
	Super::OnDataReplicated( Offset , Size );

	assert( Offset == &m_RepData && Size == sizeof(m_RepData) ); // What's being replicated?

	if( IsClient() )
	{
		if( m_CachedEncounterId != m_RepData.EncounterId )
		{
			m_CachedEncounterId = m_RepData.EncounterId;

			ExMonsterSpriteComponent* Sprite = GetComponentById<ExMonsterSpriteComponent>( eg_crc("Sprite") );
			if( Sprite )
			{
				eg_d_string TexturePath = "/game/monstersprites/T_Monster_MissingMonster_d";

				const exCombatEncounterInfo* EncounterInfo = ExCombatEncounter::Get().FindCombatEncounter( m_RepData.EncounterId );
				if( EncounterInfo && EncounterInfo->Monsters.IsValidIndex(0) )
				{
					const eg_string_crc MonsterId = EncounterInfo->Monsters[0].MonsterId;
					TexturePath = ExBeastiary::Get().FindInfo( MonsterId ).ImagePath.FullPath;
				}

				Sprite->SetTexture( eg_crc("Mesh") , *TexturePath );
			}
		}
	}
}

void ExEnemyController::OnTurnStart( ExGame* pGameData )
{
	ExMapNavBase* PlayerEnt = EGCast<ExMapNavBase>( GetEnt( pGameData->GetPrimaryPlayer() ) );

	if( m_bCanMove )
	{
		if( !m_bHasSeenPlayer )
		{
			eg_transform PlayerPose = PlayerEnt->GetPose();
			eg_vec4 DirToPlayer = PlayerPose.GetPosition() - GetPose().GetPosition();
			eg_real DistSqToPlayer = DirToPlayer.LenSqAsVec3();
			DirToPlayer.NormalizeThisAsVec3();
			if( DirToPlayer.LenAsVec3() < EG_SMALL_NUMBER )
			{
				EGLogf( eg_log_t::GameLib, "Entity %u has seen the player!", GetID().Id );
				m_bHasSeenPlayer = true;
			}
			else
			{
				egRaycastInfo RaycastInfo = SDK_GetGame()->SDK_RayCastFromEnt( GetID(), DirToPlayer );
				if( RaycastInfo.nEntID == PlayerEnt->GetID() )
				{
					EGLogf( eg_log_t::GameLib, "Entity %u has seen the player!", GetID().Id );
					m_bHasSeenPlayer = true;
				}
			}
		}

		if( m_bHasSeenPlayer )
		{
			SetActive( true );
			m_NextNavToPlayer = m_CurentNavVertex;
			m_StartMovePose = eg_transform::BuildTranslation( m_CurentNavVertex.GetPosition().ToVec3() );
			m_EndMovePose = m_StartMovePose;

			// We basically want to find out where the player is, and we'll move
			// towards it.
			if( PlayerEnt )
			{
				eg_transform PlayerTargetPose;
				PlayerTargetPose = eg_transform::BuildTranslation( PlayerEnt->GetMapVertex().GetPosition().ToVec3() );
				eg_uint Path[256];
				eg_uint NavCount = m_NavGraph.FindShortestPathBetween( GetGameData(), m_StartMovePose.GetPosition(), PlayerTargetPose.GetPosition(), Path, countof( Path ) );

				// This is a little convoluted, but the idea is that it should be that the 2nd vertex in the path
				// found should be one of the edges of the current nav node. If that's not the case something
				// weird happened.
				assert( NavCount == 0 || Path[0] == m_CurentNavVertex.GetIndex() );
				if( NavCount > 1 && Path[0] == m_CurentNavVertex.GetIndex() )
				{
					// Compare Path[1] against all edges of the current nave node, if one
					// of them is the next item, that's where we want to go.
					for( eg_uint e = 0; e < m_CurentNavVertex.GetNumEdges(); e++ )
					{
						EGNavGraphVertex Edge = m_CurentNavVertex.GetEdge( GetGameData(), e );
						if( Path[1] == Edge.GetIndex() )
						{
							m_NextNavToPlayer = m_CurentNavVertex.GetEdge( GetGameData(), e );
						}
					}
				}
			}
		}

		eg_bool bBlocked = false;

		if( m_CurentNavVertex.GetIndex() != m_NextNavToPlayer.GetIndex() )
		{
			eg_vec4 From = m_CurentNavVertex.GetPosition();
			eg_vec4 To = m_NextNavToPlayer.GetPosition();

			eg_vec4 MoveDir = To - From;
			eg_real DistToNavSq = MoveDir.LenSqAsVec3();
			MoveDir.NormalizeThisAsVec3();
			egRaycastInfo RayCastInfo = SDK_GetGame()->SDK_RayCastFromEnt( GetID(), MoveDir );
			if( INVALID_ID != RayCastInfo.nEntID )
			{
				eg_vec4 ToHitEnt = RayCastInfo.vHit - From;
				eg_real DistToHitEntSq = ToHitEnt.LenSqAsVec3();
				if( DistToHitEntSq <= DistToNavSq )
				{
					ExEnt* HitEntController = GetEnt( RayCastInfo.nEntID );
					eg_bool bIsDoor = HitEntController && HitEntController->IsA( EGClass::FindClass( "ExDoor" ) );
					if( bIsDoor )
					{
						//GetServer()->SDK_DebugOutpt( "Hit door" );
						bBlocked = true;
					}
				}
			}
		}

		if( !bBlocked )
		{
			m_EndMovePose = eg_transform::BuildTranslation( m_NextNavToPlayer.GetPosition().ToVec3() );
		}
		else
		{
			m_NextNavToPlayer = m_CurentNavVertex;
			CalmHostility();
		}
	}
}

eg_bool ExEnemyController::SendMsg( ex_notify_t Msg, eg_ent_id SrcEntId )
{
	unused( SrcEntId );
	ExGame* pGameData = GetGameData();

	switch( Msg )
	{
	case NOTIFY_ACTIVATE:
	{

	} break;
	case NOTIFY_TURN_START:
	{
		OnTurnStart( pGameData );
	} break;
	case NOTIFY_TURN_END:
	{
		if( m_bHasSeenPlayer )
		{
			TeleporTo( m_EndMovePose );
			SetActive( false );
		}
	} break;
	case NOTIFY_UNK:
	{
	} break;
	}

	return true;
}

void ExEnemyController::HandleCombatStarted()
{
	RunEvent( eg_crc("HandleCombatStarted") );
}

void ExEnemyController::HandleCombatEnded( ex_combat_result CombatResult )
{
	// Keep in mind the result is the player's result, so what happened to this is the opposite.
	
	switch( CombatResult )
	{
		case ex_combat_result::Defeat:
		case ex_combat_result::Fled:
			RunEvent( eg_crc("HandleCombatEnded") );
			break;
		case ex_combat_result::Victory:
			DeleteFromWorld();
			break;
	}
}

void ExEnemyController::CalmHostility()
{
	m_bHasSeenPlayer = false;
}

void ExEnemyController::SetEncounterId( const eg_string_crc& NewEncounterId )
{
	assert( IsServer() ); // Should only set on server.
	
	m_RepData.EncounterId = NewEncounterId;

	if( CanReplicateData() )
	{
		ReplicateChangedData( &m_RepData, sizeof(m_RepData) );
	}
}

eg_string_crc ExEnemyController::GetEncounterId() const
{
	return m_RepData.EncounterId;
}
