// (c) 2020 Beem Media. All Rights Reserved.

#include "ExVarControllerComponent.h"
#include "ExEnt.h"
#include "ExGame.h"
#include "ExGenericIoEnt.h"

EG_CLASS_DECL( ExVarControllerComponent )

void ExVarControllerComponent::InitComponentFromDef( const egComponentInitData& InitData )
{
	Super::InitComponentFromDef( InitData );

	const ExVarControllerComponent* DefAsThisType = EGCast<ExVarControllerComponent>(InitData.Def);

	m_ControlData = DefAsThisType->m_ControlData;
}

void ExVarControllerComponent::OnEnterWorld()
{
	Super::OnEnterWorld();

	if( IsServer() )
	{
		if( ExEnt* Owner = GetOwner<ExEnt>() )
		{
			if( ExGenericIoEnt* AsGenericIoEnt = EGCast<ExGenericIoEnt>(Owner) )
			{
				m_ControlData.ControlVarId = AsGenericIoEnt->GetControlVar();
			}

			if( ExGame* Game = Owner->GetGameData() )
			{
				Game->OnServerGameVarChanged.AddUnique( this , &ThisClass::OnGameVarChanged );
				HandleVarValue( Game->SmGetVar( m_ControlData.ControlVarId ).as_int() , false );
			}
		}
	}
}

void ExVarControllerComponent::OnLeaveWorld()
{
	if( IsServer() )
	{
		if( ExEnt* Owner = GetOwner<ExEnt>() )
		{
			if( ExGame* Game = Owner->GetGameData() )
			{
				Game->OnServerGameVarChanged.RemoveAll( this );
			}
		}
	}

	Super::OnLeaveWorld();
}

void ExVarControllerComponent::OnGameVarChanged( eg_string_crc VarId , const egsm_var& VarValue )
{
	if( VarId == m_ControlData.ControlVarId )
	{
		HandleVarValue( VarValue.as_int() , true );
	}
}

void ExVarControllerComponent::HandleVarValue( eg_int VarValue , eg_bool bChangedTo )
{
	if( ExEnt* Owner = GetOwner<ExEnt>() )
	{
		for( const exVarControllerState& State : m_ControlData.States )
		{
			if( State.Value == VarValue )
			{
				Owner->RunEvent( bChangedTo ? State.ChangedToValueEvent : State.SpawnedWithValueEvent );
			}
		}
	}
}
