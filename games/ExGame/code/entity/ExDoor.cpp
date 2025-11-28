// (c) 2015 Beem Media

#pragma once

#include "ExDoor.h"
#include "ExGame.h"
#include "EGParse.h"
#include "ExMapInfos.h"
#include "ExAchievements.h"

EG_CLASS_DECL( ExDoor )
EG_CLASS_DECL( ExDoorComponent )

void ExDoor::OnCreate( const egEntCreateParms& Parms )
{
	Super::OnCreate( Parms );

	if( IsAuthority() )
	{
		if( Parms.Reason == eg_spawn_reason::Spawned || Parms.Reason == eg_spawn_reason::MapEntity )
		{
			assert( !m_bInit );
			m_mode = DOOR_CLOSED;
			m_BasePose = GetPose();
			eg_transform RotIn;
			RotIn = eg_transform::BuildRotationY( EG_Deg( 90.f ) );
			eg_transform RotOut;
			RotOut = eg_transform::BuildRotationY( EG_Deg( -90.f ) );
			m_OpenPoseIn = RotIn * m_BasePose;
			m_OpenPoseOut = RotOut * m_BasePose;
			m_LockLevel = GetGameData() ? GetGameData()->GetCurrentMapInfo().DefaultLockLevel : 1;
			EGParse_ProcessFnCallScript( *Parms.InitString, Parms.InitString.Len(), [this]( const egParseFuncInfo& InfoStr )
			{
				if( EGString_EqualsI( InfoStr.FunctionName, "SetLocked" ) )
				{
					m_bIsLocked = InfoStr.NumParms >= 1 && eg_string( InfoStr.Parms[0] ).ToBool();
				}
				else if( EGString_EqualsI( InfoStr.FunctionName, "SetKeyId" ) )
				{
					m_HasKeyVar = InfoStr.NumParms >= 1 ? eg_string_crc( InfoStr.Parms[0] ) : CT_Clear;
				}
				else if( EGString_EqualsI( InfoStr.FunctionName , "SetLockLevel" ) )
				{
					m_LockLevel = InfoStr.NumParms >= 1 ? EGString_ToInt( InfoStr.Parms[0] ) : m_LockLevel;
				}
			}
			);
			m_bInit = true;
		}

		SetActive( m_bAnimating );

		//A door has an inactive AI until it is activated, it then switches to
		//a fully active ai so that the animation is smooth. Once fully open it
		//switches back to being an inactive AI.
	}
}

void ExDoor::OnEnterWorld()
{
	Super::OnEnterWorld();

	if( IsServer() )
	{
		ReplicateDataToClient( &m_bIsLocked , sizeof(m_bIsLocked) );
	}

	if( IsAuthority() )
	{
		ExDoorComponent* DoorComp = FindComponentByClass<ExDoorComponent>();
		if( DoorComp )
		{
			DoorComp->ApplyMovementType( this );
		}

		CheckForKey();
	}
}

void ExDoor::OnUpdate( eg_real DeltaTime )
{
	assert( m_bAnimating ); // We shouldn't be updating if we're not animating.

	eg_bool bComplete = false;
	m_Progress += DeltaTime*1.f / m_AnimationTime;
	if( m_Progress >= 1.f )
	{
		bComplete = true;
		m_Progress = 1.f;
	}


	//So there are three possibilities. Either the door want to open in one of
	//two directions, or it wants to close. From that we decide what the
	//desired angle of the door is.

	eg_transform FinalPose;
	const eg_real PoseProgress = ExCore_GetModifiedAnimationProgress( m_Progress , ex_move_animation_context::Door );

	switch( m_mode )
	{
	case DOOR_CLOSED:
		FinalPose = eg_transform::Lerp( m_StartPose, m_BasePose, PoseProgress );
		break;
	case DOOR_OPENOUT:
		FinalPose = eg_transform::Lerp( m_StartPose, m_OpenPoseOut, PoseProgress );
		break;
	case DOOR_OPENIN:
		FinalPose = eg_transform::Lerp( m_StartPose, m_OpenPoseIn, PoseProgress );
		break;
	}

	MoveToPose( FinalPose );

	if( bComplete )
	{
		m_bAnimating = false;
		SetActive( false );
	}
}

eg_bool ExDoor::SendMsg( ex_notify_t msg, eg_ent_id SrcEntId )
{
	eg_bool bResult = false;

	if( NOTIFY_ACTIVATE == msg && !m_bAnimating )
	{
		//The door was notified, so make the AI active.
		SetActive( true );
		m_bAnimating = true;
		m_Progress = 0.f;
		m_StartPose = GetPose();

		//If the door is open, shut it.
		if( DOOR_CLOSED != m_mode )
		{
			m_mode = DOOR_CLOSED;
			RunEvent( m_CloseAction );
			bResult = true;
		}
		else
		{
			CheckForKey();

			if( !IsLocked() )
			{
				//Otherwise decide which direction we want to open the door.
				ExEnt* SrcEnt = GetEnt( SrcEntId );
				eg_transform SrcEntPose( CT_Default );
				if( SrcEnt )
				{
					SrcEntPose = SrcEnt->GetPose();
				}
				else
				{
					assert( false ); // No entity notified this door to open?
				}

				eg_vec4 DoorFace( 0.f, 0.f, 1.f, 0.f );
				eg_vec4 SrcEntFace( 0.f, 0.f, 1.f, 0.f );

				DoorFace *= m_BasePose;
				SrcEntFace *= SrcEntPose;

				//Ent->GetServer()->SDK_DebugOutpt( EGString_Format( "Door %f %f %f, Src %f %f %f" , DoorFace.x , DoorFace.y , DoorFace.z , SrcEntFace.x , SrcEntFace.y , SrcEntFace.z ) );

				eg_real DoorFaceDot = DoorFace.Dot( SrcEntFace );

				m_mode = DoorFaceDot > 0.f ? DOOR_OPENOUT : DOOR_OPENIN;
				RunEvent( m_OpenAction );
				if( m_bIsSecretDoor )
				{
					ExAchievements::Get().UnlockAchievement( eg_crc("ACH_SECRET_DOOR") );
				}
				bResult = true;
			}
			else
			{
				RunEvent( m_LockAction );
				bResult = false;
			}
		}
	}

	return bResult;
}

void ExDoor::SetLocked( eg_bool bNewValue )
{
	if( m_bIsLocked != bNewValue )
	{
		m_bIsLocked = bNewValue;
		ReplicateDataToClient( &m_bIsLocked , sizeof(m_bIsLocked) );
	}
}

void ExDoor::CheckForKey()
{
	if( IsLocked() )
	{
		if( m_HasKeyVar.IsNotNull() )
		{
			if( GetGameData()->HasGameVar( m_HasKeyVar ) )
			{
				eg_bool bHasKey = GetGameData()->GetGameVar( m_HasKeyVar ).as_bool();
				if( bHasKey )
				{
					SetLocked( false );
				}
			}
			else
			{
				assert( false ); // This door has an assigned key, but no key to it exists.
			}
		}
	}
}

ex_door_unlock_result ExDoor::AttemptToUnlock( const class ExFighter* GuyToUnlock )
{
	ex_door_unlock_result Result = ex_door_unlock_result::Unlocked;

	assert( IsServer() );

	if( IsLocked() )
	{
		CheckForKey();

		if( IsLocked() )
		{
			Result = ex_door_unlock_result::LockedNeedsThief;

			if( GuyToUnlock )
			{
				// Door can't have a key associated with it, class must be thief, and thief level must be above door level.
				if( m_HasKeyVar.IsNotNull() )
				{
					Result = ex_door_unlock_result::LockedNeedsKey;
				}
				else if( GuyToUnlock->GetClass() != ex_class_t::Thief )
				{
					Result = ex_door_unlock_result::LockedNeedsThief;
				}
				else if( GuyToUnlock->GetAttrValue( ex_attr_t::LVL ) < m_LockLevel )
				{
					Result = ex_door_unlock_result::LockedNeedsLevel;
				}
				else
				{
					Result = ex_door_unlock_result::Unlocked;
					ExAchievements::Get().UnlockAchievement( eg_crc("ACH_LOCKSMITH") );
					SetLocked( false );	
				}
			}
		}
		else
		{
			Result = ex_door_unlock_result::Unlocked;
		}
	}

	RunEvent( IsLocked() ? m_LockAction : m_UnlockAction );

	return Result;
}

void ExDoorComponent::InitComponentFromDef( const egComponentInitData& InitData )
{
	Super::InitComponentFromDef( InitData );

	const ExDoorComponent* DoorDef = EGCast<ExDoorComponent>(InitData.Def);

	m_DoorMovementType = DoorDef->m_DoorMovementType;
	m_DoorMovementAmount = DoorDef->m_DoorMovementAmount;
	m_DoorAnimationTime = DoorDef->m_DoorAnimationTime;
	m_OpenAction = DoorDef->m_OpenAction;
	m_CloseAction = DoorDef->m_CloseAction;
	m_LockAction = DoorDef->m_LockAction;
	m_UnlockAction = DoorDef->m_UnlockAction;
	m_bStaysOpen = DoorDef->m_bStaysOpen;
	m_bIsSecretDoor = DoorDef->m_bIsSecretDoor;
}

void ExDoorComponent::OnEnterWorld()
{
	Super::OnEnterWorld();

	ExDoor* DoorOwner = EGCast<ExDoor>(m_InitData.Owner);
	if( DoorOwner && DoorOwner->IsAuthority() )
	{
		DoorOwner->m_AnimationTime = m_DoorAnimationTime;
		DoorOwner->m_OpenAction = m_OpenAction;
		DoorOwner->m_CloseAction = m_CloseAction;
		DoorOwner->m_LockAction = m_LockAction;
		DoorOwner->m_UnlockAction = m_UnlockAction;
		DoorOwner->m_bStaysOpen = m_bStaysOpen;
		DoorOwner->m_bIsSecretDoor = m_bIsSecretDoor;
	}
}

void ExDoorComponent::ApplyMovementType( ExDoor* Door ) const
{
	if( Door )
	{
		switch( m_DoorMovementType )
		{
			case ex_door_movement_t::Swing:
			{
				const eg_transform RotIn = eg_transform::BuildRotationY( EG_Deg( m_DoorMovementAmount ) );
				const eg_transform RotOut = eg_transform::BuildRotationY( EG_Deg( -m_DoorMovementAmount ) );
				Door->m_OpenPoseIn = RotIn * Door->m_BasePose;
				Door->m_OpenPoseOut = RotOut * Door->m_BasePose;
			} break;
			case ex_door_movement_t::SlideVertical:
			{
				const eg_transform Offset = eg_transform::BuildTranslation( 0.f , m_DoorMovementAmount , 0.f );
				Door->m_OpenPoseIn = Offset * Door->m_BasePose;
				Door->m_OpenPoseOut = Door->m_OpenPoseIn;
			} break;
			case ex_door_movement_t::SlideHorizontal:
			{
				const eg_transform Offset = eg_transform::BuildTranslation( m_DoorMovementAmount , 0.f , 0.f );
				Door->m_OpenPoseIn = Offset * Door->m_BasePose;
				Door->m_OpenPoseOut = Door->m_OpenPoseIn;
			} break;
		}
	}
}
