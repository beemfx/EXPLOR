// (c) 2015 Beem Media

#include "ExPlayerController.h"
#include "ExGame.h"
#include "EGBoolList.h"
#include "ExServer.h"
#include "EGDebugText.h"
#include "EGEntWorld.h"
#include "ExDoor.h"
#include "ExCosmeticEnt.h"
#include "EGSettings2Types.h"
#include "ExGameSettings.h"

EG_CLASS_DECL( ExPlayerController )

static inline eg_bool ExInput_ForwardActive( const egLockstepCmds& Cmds )
{
	return Cmds.IsActive( CMDA_FORWARD1 ) || Cmds.IsActive( CMDA_FORWARD2 );
}

static inline eg_bool ExInput_BackActive( const egLockstepCmds& Cmds )
{
	return Cmds.IsActive( CMDA_BACK1 ) || Cmds.IsActive( CMDA_BACK2 );
}

static inline eg_bool ExInput_TurnRightActive( const egLockstepCmds& Cmds )
{
	return Cmds.IsActive( CMDA_TURNRIGHT1 ) || Cmds.IsActive( CMDA_TURNRIGHT2 );
}

static inline eg_bool ExInput_TurnLeftActive( const egLockstepCmds& Cmds )
{
	return Cmds.IsActive( CMDA_TURNLEFT1 ) || Cmds.IsActive( CMDA_TURNLEFT2 );
}

static inline eg_bool ExInput_StrafeRightActive( const egLockstepCmds& Cmds )
{
	return Cmds.IsActive( CMDA_STRAFERIGHT1 ) || Cmds.IsActive( CMDA_STRAFERIGHT2 );
}

static inline eg_bool ExInput_StrafeLeftActive( const egLockstepCmds& Cmds )
{
	return Cmds.IsActive( CMDA_STRAFELEFT1 ) || Cmds.IsActive( CMDA_STRAFELEFT2 );
}

static inline eg_bool ExInput_ForwardPressed( const egLockstepCmds& Cmds )
{
	return Cmds.WasPressed( CMDA_FORWARD1 ) || Cmds.WasPressed( CMDA_FORWARD2 );
}

static inline eg_bool ExInput_BackPressed( const egLockstepCmds& Cmds )
{
	return Cmds.WasPressed( CMDA_BACK1 ) || Cmds.WasPressed( CMDA_BACK2 );
}

static inline eg_bool ExInput_TurnRightPressed( const egLockstepCmds& Cmds )
{
	return Cmds.WasPressed( CMDA_TURNRIGHT1 ) || Cmds.WasPressed( CMDA_TURNRIGHT2 );
}

static inline eg_bool ExInput_TurnLeftPressed( const egLockstepCmds& Cmds )
{
	return Cmds.WasPressed( CMDA_TURNLEFT1 ) || Cmds.WasPressed( CMDA_TURNLEFT2 );
}

static inline eg_bool ExInput_StrafeRightPressed( const egLockstepCmds& Cmds )
{
	return Cmds.WasPressed( CMDA_STRAFERIGHT1 ) || Cmds.WasPressed( CMDA_STRAFERIGHT2 );
}

static inline eg_bool ExInput_StrafeLeftPressed( const egLockstepCmds& Cmds )
{
	return Cmds.WasPressed( CMDA_STRAFELEFT1 ) || Cmds.WasPressed( CMDA_STRAFELEFT2 );
}

static const eg_real ExPlayerController_WalkTime = EX_TURN_TIME;
static const eg_real ExPlayerController_CameraHeightAboveGraph = .6f;
static const eg_real ExPLayerController_CameraDistanceBehind = -1.2f;
static const eg_real ExPlayerController_TurnTime = .5f;
static const eg_real ExPlayerController_HeadBobIntensityAccel = 4.f;

static EGSettingsBool ExPlayer_DebugShowTarget( "ExPlayer.DebugShowTarget" , eg_loc("SettingsDebugShowTarget","DEBUG: Show Player Target") , false , EGS_F_USER_SAVED|EGS_F_DEBUG_EDITABLE );
static EGSettingsBool ExPlayer_EnableHeadBob( "ExPlayer.EnableHeadBob" , eg_loc("ExSettingsEnableHeadBob","Enable Head Bob") , true , EGS_F_USER_SAVED|EGS_F_EDITABLE );

struct exPcWalkCycle
{
	eg_real       Time;
	eg_string_crc Event;
};

static const exPcWalkCycle ExPlayerController_WalkCycle[] =
{
	{ 0.f*ExPlayerController_WalkTime , eg_crc("Walk1") },
	{ 1.f*ExPlayerController_WalkTime , eg_crc("Walk2") },
	{ 2.f*ExPlayerController_WalkTime , eg_crc("") },
};

static const exPcWalkCycle ExPlayerController_TurnCycle[] =
{
	{ 0.f*ExPlayerController_TurnTime , eg_crc("Turn2") },
	{ 1.f*ExPlayerController_TurnTime , eg_crc("Turn1") },
	{ 2.f*ExPlayerController_TurnTime , eg_crc("") },
};


static_assert( countof(ExPlayerController_WalkCycle) <= ExPlayerController::MAX_CYCLES && countof(ExPlayerController_TurnCycle) <= ExPlayerController::MAX_CYCLES , "Increase max cycles" );

ExPlayerController::ExPlayerController() : ExMapNavBase()
, m_CyclesPassed( CT_Clear )
{

}

void ExPlayerController::OnEncounterSetCamera( const eg_transform& CameraPose )
{
	m_FocusCamera.SetFocus( CameraPose , .5f );
}

eg_transform ExPlayerController::GetCameraPose() const
{
	eg_transform Camera = eg_transform(m_LookTransform) * GetPose();
	if( m_FocusCamera.bUsingFocusCamera )
	{
		Camera = eg_transform::Lerp( Camera , m_FocusCamera.CameraPose , m_FocusCamera.GetTransitionPct() );
	}
	return Camera;
}

eg_transform ExPlayerController::GetBaseCameraPose() const
{
	eg_transform CameraTrans = CT_Default;
	CameraTrans.TranslateThis( eg_vec3( 0.f, ExPlayerController_CameraHeightAboveGraph, ExPLayerController_CameraDistanceBehind) );
	return CameraTrans * GetPose();
}

void ExPlayerController::OnCreate( const egEntCreateParms& Parms )
{
	Super::OnCreate( Parms );

	m_LookTransform = eg_transform::BuildIdentity();

	if( GetRole() == eg_ent_role::Authority )
	{
		Initialize();
		m_RepDataChecksum = eg_crc( "" );
		SetActive( true );
	}

	if( IsClient() )
	{
		SetActive( true );
	}

	UpdateDebugHitBoxVisibility();
}

void ExPlayerController::OnEnterWorld()
{
	Super::OnEnterWorld();

	EGEntWorld* World = GetWorld();
	if( World )
	{
		World->PostLoadWorldComplete.AddUnique( this , &ThisClass::OnWorldLoadComplete );
	}
}

void ExPlayerController::OnLeaveWorld()
{
	EGEntWorld* World = GetWorld();
	if( World )
	{
		World->PostLoadWorldComplete.RemoveAll( this );
	}
	
	Super::OnLeaveWorld();
}

void ExPlayerController::OnWorldLoadComplete()
{
	RefreshTargetedEnt();
}

void ExPlayerController::AssignToPlayer( const eg_lockstep_id& InLockstepId )
{
	Super::AssignToPlayer( InLockstepId );
	assert( GetGameData()->GetPrimaryPlayer() == INVALID_ID ); // Should only be one player controller.
	GetGameData()->SetPrimaryPlayer( GetID() );
}

void ExPlayerController::OnEncounterStarted()
{
	OnStoppedMoving();
}

void ExPlayerController::OnStoppedMoving()
{
	if( m_bWentIdleFromWalk )
	{
		m_bWentIdleFromWalk = false;
		RunEvent( eg_crc( "Halt" ) );

	}
	if( m_bWentIdleFromTurn )
	{
		m_bWentIdleFromTurn = false;
	}
}

void ExPlayerController::OnUpdate( eg_real DeltaTime )
{
	if( GetRole() == eg_ent_role::Authority )
	{
		ExGame* pGameData = GetGameData();

		if( false )
		{
			eg_region_id MyRegion = GetWorldRegion();
			EGLogToScreen::Get().Log( this , 0 , 1.f , EGString_Format( "Region %X" , MyRegion.GetListId() ) );
		}

		//If we are currently handling a map event the player isn't allowed to do anything.
		if( !pGameData->CanPlayerMove() )
		{
			assert( m_MoveState == ex_move_s::NONE || m_MoveState == ex_move_s::WAITING_ON_DOOR || m_MoveState == ex_move_s::WAITING_ON_LOCKED_DOOR );
		}

		if( ex_move_s::WAITING_ON_DOOR == m_MoveState || ex_move_s::WAITING_ON_LOCKED_DOOR == m_MoveState )
		{
			m_StateProgress += DeltaTime*m_StateRate;
			if( m_StateProgress >= 1.f )
			{
				m_MoveState = ex_move_s::NONE;
				RefreshTargetedEnt();
			}
		}

		//We always request a ray-trace for the player character.
		//Since it is first person we just trace what the character is looking at.
		if( ex_move_s::NONE == m_MoveState && pGameData->CanPlayerMove() )
		{
			egLockstepCmds Cmds;
			SDK_GetGame()->SDK_GetCommandsForPlayer( m_LockstepId, &Cmds );
			OnUpdate_HandleInput( Cmds, DeltaTime );
			// If we are still idle, end the walk cycle.
			if( m_MoveState == ex_move_s::NONE )
			{
				m_CyclesPassed.UnsetAll();
				m_WalkCycle = 0.f;

				OnStoppedMoving();
			}
		}

		if( ex_move_s::WAIT_TURN == m_MoveState )
		{
			m_StateProgress += DeltaTime*m_StateRate;
			if( m_StateProgress < 1.f )
			{
			}
			else
			{
				m_MoveState = ex_move_s::NONE;
				RefreshTargetedEnt();
				SDK_GetGame()->SDK_RunServerEvent( eg_crc( "OnTurnEnd" ) );
				CloseLastDoorIfOpen();
			}
		}

		if( ex_move_s::WALKING == m_MoveState || ex_move_s::TURNING == m_MoveState )
		{
			m_StateProgress += DeltaTime*m_StateRate;
			if( m_StateProgress < 1.f )
			{
				eg_transform FinalPose = eg_transform::Lerp( m_StateStartPose , m_StateEndPose, ExCore_GetModifiedAnimationProgress( m_StateProgress , m_MoveState == ex_move_s::TURNING ? ex_move_animation_context::Turning : ex_move_animation_context::Walking ) );
				SetPose( FinalPose );
			}
			else
			{
				m_bWentIdleFromWalk = m_MoveState == ex_move_s::WALKING;
				m_bWentIdleFromTurn = m_MoveState == ex_move_s::TURNING;
				SetPose( m_StateEndPose );
				m_MoveState = ex_move_s::NONE;
				RefreshTargetedEnt();
				if( m_bWentIdleFromWalk )
				{
					SDK_GetGame()->SDK_RunServerEvent( eg_crc( "OnTurnEnd" ) );
				}
				if( m_bWentIdleFromTurn )
				{
					SDK_GetGame()->SDK_RunServerEvent( eg_crc( "OnPlayerRotated" ) );
				}
				CloseLastDoorIfOpen();
			}

			auto HandleCycle = [this, &DeltaTime]( const exPcWalkCycle* Cycles, eg_uint CyclesCount ) -> void
			{
				m_WalkCycle += DeltaTime;

				for( eg_uint i = 0; i < CyclesCount; i++ )
				{
					const exPcWalkCycle& Cycle = Cycles[i];
					if( !m_CyclesPassed.IsSet( i ) && m_WalkCycle >= Cycle.Time )
					{
						m_CyclesPassed.Set( i );
						if( Cycle.Event != eg_crc( "" ) )
						{
							RunEvent( Cycle.Event );
						}
						else
						{
							m_WalkCycle = 0.f;
							m_CyclesPassed.UnsetAll();
						}
					}
				}
			};

			if( m_MoveState == ex_move_s::WALKING )
			{
				HandleCycle( ExPlayerController_WalkCycle, countof( ExPlayerController_WalkCycle ) );
			}
			else if( m_MoveState == ex_move_s::TURNING )
			{
				HandleCycle( ExPlayerController_TurnCycle, countof( ExPlayerController_TurnCycle ) );
			}
		}

		eg_bool bIsWalking = m_MoveState == ex_move_s::WALKING;
		eg_real CameraSway = UpdateHeadSway( DeltaTime, bIsWalking );
		eg_vec4 HeadHump = UpdateHeadBob( DeltaTime, bIsWalking );

		m_FocusCamera.Update( DeltaTime );
		ReplicateDataToClient( &m_FocusCamera , sizeof(m_FocusCamera) );

		eg_transform CameraTrans;
		CameraTrans = eg_transform::BuildIdentity();
		CameraTrans.TranslateThis( HeadHump.ToVec3() );
		CameraTrans.TranslateThis( eg_vec3( 0.f, ExPlayerController_CameraHeightAboveGraph, ExPLayerController_CameraDistanceBehind) );
		CameraTrans.RotateZThis( EG_Rad(CameraSway) );
		m_LookTransform = CameraTrans;
		m_LookTransform.RecPose.TimeMs = GetWorld() ? GetWorld()->GetGameTimeMs() : 0;
		ReplicateDataToClient( &m_LookTransform.RecPose , sizeof(m_LookTransform.RecPose) );
		
		// TODO: No need to do this every frame, just on turn end or start
		// RefreshTargetedEnt();

		ReplicateChangedData( &m_RepData, sizeof( m_RepData ) );
	}

	if( IsClient() )
	{
		m_LookTransform.Smooth( DeltaTime , GetWorld() ? GetWorld()->GetEntUpdateRate() : 0.f );
	}
}

void ExPlayerController::OnUpdate_HandleInput( const egLockstepCmds& Cmds , eg_real DeltaTime )
{
	unused( DeltaTime );

	assert( m_MoveState == ex_move_s::NONE );

	m_StateStartPose = GetPose();
	eg_transform PoseMatrix( m_StateStartPose );

	// Figure out which way we are facing.
	eg_vec4 FaceDir( 0.f, 0.f, 1.f, 0.f );
	FaceDir *= PoseMatrix;
	eg_vec4 RightDir( 1.f , 0.f , 0.f , 0.f );
	RightDir *= PoseMatrix;

	m_StateProgress = 0.f;
	m_StateRate = 0.f;

	//GetServer()->SDK_FramePrint( EGString_Format( "Pos %f %f %f" , Pose.Pos.x , Pose.Pos.y , Pose.Pos.z ) );
	//GetServer()->SDK_FramePrint( EGString_Format( "Face %f %f %f" , FaceDir.x , FaceDir.y , FaceDir.z ) );

	auto DoMove = [this,&FaceDir,&RightDir]( eg_real Direction , eg_bool bStrafe ) -> void
	{
		bool bFoundNav = false;
		eg_real DistToNavSq = 0.f;
		EGNavGraphVertex NavToVertex( CT_Clear );
		for( eg_uint i = 0; !bFoundNav && i < m_CurentNavVertex.GetNumEdges(); i++ )
		{
			// If the angle between the face direction and the edge in question
			// is zero then we found the vertex we want to go to.
			EGNavGraphVertex Edge = m_CurentNavVertex.GetEdge( GetGameData(), i );
			eg_vec4 ToEdge = Edge.GetPosition() - m_StateStartPose.GetPosition();
			ToEdge.y = 0.f;
			eg_real DistSq = ToEdge.LenSqAsVec3();
			ToEdge.NormalizeThisAsVec3();
			eg_real Dotp = ToEdge.Dot( bStrafe ? RightDir : FaceDir );
			//GetServer()->SDK_FramePrint( EGString_Format( "Edge %u: %f" , i , Dotp ) );
			if( EG_IsEqualEps( Dotp, Direction, EG_SMALL_NUMBER) )
			{
				DistToNavSq = DistSq;
				NavToVertex = m_CurentNavVertex.GetEdge( GetGameData(), i );
				bFoundNav = true;
			}
		}

		if( bFoundNav )
		{
			// Handle doors and stuff...
			eg_bool bBlocked = false;
			egRaycastInfo RayCastInfo = SDK_GetGame()->SDK_RayCastFromEnt( GetID(), (bStrafe ? RightDir : FaceDir)*Direction );
			if( INVALID_ID != RayCastInfo.nEntID )
			{
				ExEnt* HitEntController = GetEnt( RayCastInfo.nEntID );
				eg_bool bIsDoor = HitEntController && HitEntController->IsA( &ExDoor::GetStaticClass() );
				eg_bool bIsCosmetic = HitEntController && HitEntController->IsA( &ExCosmeticEnt::GetStaticClass() );
				if( bIsDoor || bIsCosmetic )
				{
					eg_vec4 ToHitEnt = RayCastInfo.vHit - m_StateStartPose.GetPosition();
					eg_real DistToHitEntSq = ToHitEnt.LenSqAsVec3();
					if( DistToHitEntSq <= DistToNavSq )
					{
						if( bIsDoor )
						{
							eg_bool bDoDoorOpen = !bStrafe && Direction > 0.f;
							if( bDoDoorOpen )
							{
								eg_bool bDoorWasOpened = false;

								// We probably have a door in the way.
								assert( GetGameData()->GetLastDoorOpenedByPlayer() == INVALID_ID ); // We will have a door stuck open.
								ExDoor* DoorEnt = EGCast<ExDoor>(HitEntController);
								const eg_real AnimationTime = (DoorEnt ? DoorEnt->GetAnimationTime() : .5f) + EG_SMALL_NUMBER;
								if( DoorEnt )
								{
									bDoorWasOpened = DoorEnt->SendMsg( NOTIFY_ACTIVATE, GetID() );
								}
								if( bDoorWasOpened )
								{
									GetGameData()->ResetStuffForPlayerMoving();
									GetGameData()->SetLastDoorOpenedByPlayer( RayCastInfo.nEntID );
									m_StateRate = 1.f / AnimationTime;
									m_MoveState = ex_move_s::WAITING_ON_DOOR;
								}
								else
								{
									static const eg_real WAITING_ON_LOCKED_DOOR_TIME = .5f;
									m_StateRate = 1.f / WAITING_ON_LOCKED_DOOR_TIME;
									m_MoveState = ex_move_s::WAITING_ON_LOCKED_DOOR;
									GetGameData()->SetActiveTranslationText( eg_loc("DoorLockedText","The door is locked...") );
								}
							}
							bBlocked = true;
						}
						else if( bIsCosmetic )
						{
							if( ExCosmeticEnt* AsCosmetic = EGCast<ExCosmeticEnt>( HitEntController ) )
							{
								bBlocked = true;
							}
						}
					}
				}
			}

			if( !bBlocked )
			{
				m_StateEndPose = eg_transform::BuildIdentity();
				m_StateEndPose.SetRotation( m_StateStartPose.GetRotation() );
				m_StateEndPose.SetTranslation( NavToVertex.GetPosition().ToVec3() );
				m_StateEndPose.SetScale( 1.f );
				m_CurentNavVertex = NavToVertex;
				m_StateRate = 1.f / ExPlayerController_WalkTime;
				m_MoveState = ex_move_s::WALKING;
				GetGameData()->RevealAutomap( m_StateEndPose );
				GetGameData()->ResetStuffForPlayerMoving();
				SDK_GetGame()->SDK_RunServerEvent( eg_crc( "OnTurnStart" ) );
			}
		}
		else
		{
			// TODO: Play sound indicating that we can't walk through here.
		}
	};

	// We use else if, because we will only do one thing per turn.
	if( Cmds.WasReleased( CMDA_WAITTURN ) )
	{
		m_FocusCamera.RestoreFocus();

		if( GetGameData()->GetLastDoorOpenedByPlayer() != INVALID_ID ) // For waiting if we had a door open, close it and call it good.
		{
			CloseLastDoorIfOpen();
		}
		else // Otherwise wait for a turn (allow enemies and npc's to move)
		{
			m_StateEndPose = m_StateStartPose;
			m_StateRate = 1.f / ExPlayerController_TurnTime;
			m_MoveState = ex_move_s::WAIT_TURN;
			GetGameData()->SDK_RunClientEvent( eg_crc("ClientNotifyWait") );
			GetGameData()->ResetStuffForPlayerMoving();
			SDK_GetGame()->SDK_RunServerEvent( eg_crc( "OnTurnStart" ) );
		}
	}
	else if( ExInput_TurnRightActive( Cmds ) )
	{
		m_FocusCamera.RestoreFocus();

		eg_transform RotTrans;
		RotTrans = eg_transform::BuildRotationY( EG_Deg( 90.f ) );
		m_StateEndPose = RotTrans * m_StateStartPose;
		m_StateRate = 1.f / ExPlayerController_TurnTime;
		m_MoveState = ex_move_s::TURNING;
		GetGameData()->ResetStuffForPlayerMoving();
	}
	else if( ExInput_TurnLeftActive( Cmds ) )
	{
		m_FocusCamera.RestoreFocus();

		eg_transform RotTrans;
		RotTrans = eg_transform::BuildRotationY( EG_Deg( -90.f ) );
		m_StateEndPose = RotTrans * m_StateStartPose;
		m_StateRate = 1.f / ExPlayerController_TurnTime;
		m_MoveState = ex_move_s::TURNING;
		GetGameData()->ResetStuffForPlayerMoving();
	}
	else if( ExInput_ForwardActive( Cmds ) )
	{
		m_FocusCamera.RestoreFocus();

		DoMove( 1.f , false );
	}
	else if( ExInput_BackActive( Cmds ) )
	{
		m_FocusCamera.RestoreFocus();

		DoMove( -1.f , false );
	}
	else if( ExInput_StrafeLeftActive( Cmds ) )
	{
		m_FocusCamera.RestoreFocus();

		DoMove( -1.f , true );
	}
	else if( ExInput_StrafeRightActive( Cmds ) )
	{
		m_FocusCamera.RestoreFocus();

		DoMove( 1.f , true );
	}
	else
	{
		m_StateEndPose = m_StateStartPose;
	}
}

eg_real ExPlayerController::UpdateHeadSway( eg_real DeltaTime, eg_bool bWalking )
{
	static const eg_real HEAD_SWAY_RATE = 1.8f;

	const eg_bool bEnableHeadBob = ExPlayer_EnableHeadBob.GetValue() && ExGameSettings_WalkAnimation.GetValue() == ex_move_animation::Smooth;

	if( bWalking )
	{
		m_HeadSwayTime += DeltaTime*( 1.f / HEAD_SWAY_RATE );
	}

	if( m_HeadSwayTime >= 1.f )
	{
		m_HeadSwayTime = 0.f;
	}
	else
	{
		//m_HeadSwayTime += DeltaTime*(1.f/(10.f*HEAD_SWAY_RATE));
	}

	eg_real CameraSway = m_HeadBobIntensity*.01f*EGMath_sin( EG_Rad(EGMath_GetMappedRangeValue( m_HeadSwayTime, eg_vec2( 0.f, 1.f ), eg_vec2( 0.f, 2.f*EG_PI ) )) );
	if( !bEnableHeadBob )
	{
		CameraSway = 0.f;
	}
	return CameraSway;
}

eg_vec4 ExPlayerController::UpdateHeadBob( eg_real DeltaTime, eg_bool bWalking )
{
	static const eg_real HEAD_HUMP_RATE = .5f;

	const eg_bool bEnableHeadBob = ExPlayer_EnableHeadBob.GetValue() && ExGameSettings_WalkAnimation.GetValue() == ex_move_animation::Smooth;

	eg_vec4 Out( 0.f, 0.f, 0.1f, 0.f );

	if( bWalking )
	{
		m_HeadBobTime += DeltaTime*( 1.f / HEAD_HUMP_RATE );
		m_HeadBobIntensity += ExPlayerController_HeadBobIntensityAccel*DeltaTime;
	}
	else
	{
		m_HeadBobIntensity -= ExPlayerController_HeadBobIntensityAccel*DeltaTime;
	}

	m_HeadBobIntensity = EG_Clamp( m_HeadBobIntensity, 0.f, 1.f );

	if( m_HeadBobTime >= 1.f )
	{
		m_HeadBobTime = 0.f;
	}

	if( bEnableHeadBob )
	{
		Out *= m_HeadBobIntensity;
	}

	eg_real CameraSway = EGMath_GetMappedRangeValue( m_HeadBobTime, eg_vec2( 0.f, 1.f ), eg_vec2( 0.f, 2.f*EG_PI ) );
	eg_transform Trans = eg_transform::BuildRotationX( EG_Rad(CameraSway) );
	if( bEnableHeadBob )
	{
		Out *= Trans;
		// Raise the head up just a little bit, this will give the illusion that
		// when standing still we are always going down to rest.
		Out += eg_vec4( 0.f, m_HeadBobIntensity*.1f, 0.f, 0.f );
	}

	return Out;
}

void ExPlayerController::CloseLastDoorIfOpen()
{
	assert( m_MoveState == ex_move_s::NONE ); // Shouldn't close a door while doing something else
	if( INVALID_ID != GetGameData()->GetLastDoorOpenedByPlayer() )
	{
		ExDoor* DoorEnt = EGCast<ExDoor>(GetEnt( GetGameData()->GetLastDoorOpenedByPlayer() ));
		const eg_real AnimationTime = (DoorEnt ? DoorEnt->GetAnimationTime() : .5f) + EG_SMALL_NUMBER;
		const eg_bool bShouldClose = DoorEnt && !DoorEnt->ShouldStayOpen();
		if( bShouldClose && DoorEnt )
		{
			DoorEnt->SendMsg( NOTIFY_ACTIVATE, GetID() );
			m_MoveState = ex_move_s::WAITING_ON_DOOR;
			m_StateProgress = 0.f;
			m_StateRate = 1.f / AnimationTime;
		}

		GetGameData()->SetLastDoorOpenedByPlayer( INVALID_ID );
	}
}

void ExPlayerController::RefreshTargetedEnt()
{
	eg_real  DistToEntSq = 0.f;
	eg_ent_id TargetedEnt = GetTargetedEnt( nullptr , &DistToEntSq );
	eg_real DistToVertexSq = 0.f;
	EGNavGraphVertex TargetedVertex = GetTargetedVertex( &DistToVertexSq );

	if( TargetedEnt != INVALID_ID && TargetedVertex.IsValid() && DistToEntSq < DistToVertexSq )
	{
		if( EX_CHEATS_ENABLED && ExPlayer_DebugShowTarget.GetValueThreadSafe() )
		{
			EGLogToScreen::Get().Log( this , 0 , 1.f , EGString_Format( "Targeting %d" , TargetedEnt.Id ) );
		}
		
		m_TargetedEnt = TargetedEnt;
	}
	else
	{
		m_TargetedEnt = INVALID_ID;
	}

	GetGameData()->SetPlayerTargetedEnt( m_TargetedEnt );
}

void ExPlayerController::TeleporTo( const eg_transform& NewPose )
{
	ExMapNavBase::TeleporTo( NewPose );
	CloseLastDoorIfOpen();

	GetGameData()->RevealAutomap( GetPose() );

	RefreshTargetedEnt();
}

void ExPlayerController::exFocusCamera::SetFocus( const eg_transform& InCameraPose , eg_real InTransitionTime )
{
	CameraPose = InCameraPose;
	TimeSinceLook = 0.f;
	TransitionTime = InTransitionTime;
	bIsRestoring = false;
	bUsingFocusCamera = true;
}

void ExPlayerController::exFocusCamera::RestoreFocus()
{
	bIsRestoring = true;
	TimeSinceLook = EG_Min( TimeSinceLook , TransitionTime );
}

void ExPlayerController::exFocusCamera::Update( eg_real DeltaTime )
{
	eg_transform CameraOut;
	CameraOut = eg_transform::BuildIdentity();

	if( !bUsingFocusCamera )
	{
		TimeSinceLook = 0.f;
		return;
	}

	if( bIsRestoring )
	{
		TimeSinceLook -= DeltaTime;
		if( TimeSinceLook < 0 )
		{
			TimeSinceLook = 0;
			bUsingFocusCamera = false;
		}
	}
	else
	{
		TimeSinceLook += DeltaTime;
	}
}

eg_real ExPlayerController::exFocusCamera::GetTransitionPct() const
{
	eg_real TransitionPct = ( TimeSinceLook < TransitionTime ) ? EGMath_GetMappedCubicRangeValueNormalized( TimeSinceLook, eg_vec2( 0.f, TransitionTime ), eg_vec2( 0.f, 1.f ) ) : 1.f;
	TransitionPct = ExCore_GetModifiedAnimationProgress( TransitionPct , ex_move_animation_context::Looking );
	return TransitionPct;
}
