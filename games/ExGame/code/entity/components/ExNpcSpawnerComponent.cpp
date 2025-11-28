// (c) 2020 Beem Media. All Rights Reserved.

#include "ExNpcSpawnerComponent.h"
#include "ExGame.h"
#include "ExMapNavBase.h"
#include "ExIoEnt.h"

EG_CLASS_DECL( ExNpcSpawnerComponent )

void ExNpcSpawnerComponent::InitComponentFromDef( const egComponentInitData& InitData )
{
	Super::InitComponentFromDef( InitData );

	const ExNpcSpawnerComponent* DefAsNpcSpawner = EGCast<ExNpcSpawnerComponent>(InitData.Def);

	m_SpawnerData = DefAsNpcSpawner->m_SpawnerData;
}

void ExNpcSpawnerComponent::OnEnterWorld()
{
	Super::OnEnterWorld();

	if( IsServer() )
	{
		ExEnt* MapNavOwner = GetOwner<ExEnt>();

		m_OwnerInitId = MapNavOwner ? eg_string_crc(*MapNavOwner->GetInitId()) : CT_Clear;

		if( m_OwnerGame )
		{
			m_OwnerGame->OnServerGameVarChanged.AddUnique( this , &ThisClass::OnGameStateVarChanged );
		}

		RefreshFromGameState( false );
	}
}

void ExNpcSpawnerComponent::OnLeaveWorld()
{
	if( IsServer() )
	{
		if( m_OwnerGame )
		{
			m_OwnerGame->OnServerGameVarChanged.RemoveAll( this );
		}

		for( const exNpcSpawnerData& SpawnerData : m_SpawnerData )
		{
			if( SpawnerData.SpawnedEntity )
			{
				SpawnerData.SpawnedEntity->DeleteFromWorld();
				SpawnerData.SpawnedEntity = nullptr;
			}
		}
	}

	Super::OnLeaveWorld();
}

void ExNpcSpawnerComponent::RefreshFromGameState( eg_bool bBecauseVarChanged )
{
	assert( IsServer() );

	if( m_OwnerGame )
	{
		for( const exNpcSpawnerData& SpawnerData : m_SpawnerData )
		{
			if( m_OwnerInitId == SpawnerData.SpawnerEntityInitId && m_OwnerGame->HasGameVar( SpawnerData.ControlVariable ) )
			{
				const eg_int GameVar = m_OwnerGame->GetGameVar( SpawnerData.ControlVariable ).as_int();
				HandleGameState( SpawnerData , EG_IsBetween<eg_int>( GameVar , SpawnerData.ExistenceRange.x , SpawnerData.ExistenceRange.y ) , bBecauseVarChanged );
			}
		}
	}
}

void ExNpcSpawnerComponent::HandleGameState( const exNpcSpawnerData& SpawnerData , eg_bool bShouldExist , eg_bool bBecauseVarChanged )
{
	assert( IsServer() );
	
	// EGLogf( eg_log_t::General , "Spawner 0x%X: Entity (%s) (%s)" , m_OwnerInitId.ToUint32() , *SpawnerData.EntId.Path , bShouldExist ? "Exists" : "Does not exist" );
	if( bShouldExist && !SpawnerData.SpawnedEntity.IsValid() )
	{
		ExEnt* MapNavOwner = GetOwner<ExEnt>();
		if( m_OwnerGame && MapNavOwner )
		{
			SpawnerData.SpawnedEntity = m_OwnerGame->SDK_SpawnEnt( eg_string_crc(*SpawnerData.EntId.Path) , MapNavOwner->GetPose() , *SpawnerData.EntInitString ); 
			if( SpawnerData.SpawnedEntity )
			{
				// Entities spawned from a spawner should not be serialized as they are controlled by the spawner.
				SpawnerData.SpawnedEntity->SetIsSerialized( false );
				if( bBecauseVarChanged && SpawnerData.SpawnEvent.IsNotNull() )
				{
					SpawnerData.SpawnedEntity->RunEvent( SpawnerData.SpawnEvent );
				}
			}
		}
	}
	else if( !bShouldExist && SpawnerData.SpawnedEntity.IsValid() )
	{
		ExIoEnt* AsNpc = EGCast<ExIoEnt>(SpawnerData.SpawnedEntity.GetObject());
		if( AsNpc )
		{
			AsNpc->HandleNpcDespawn( SpawnerData.DespawnDuration , bBecauseVarChanged ? SpawnerData.DespawnEvent : CT_Clear );
		}
		else
		{
			SpawnerData.SpawnedEntity->DeleteFromWorld();
		}
		SpawnerData.SpawnedEntity = nullptr;
	}
}

void ExNpcSpawnerComponent::OnGameStateVarChanged( eg_string_crc VarId , const egsm_var& NewValue )
{
	assert( IsServer() );

	for( const exNpcSpawnerData& SpawnerData : m_SpawnerData )
	{
		if( m_OwnerInitId == SpawnerData.SpawnerEntityInitId && VarId == SpawnerData.ControlVariable )
		{
			const eg_int GameVar = NewValue.as_int();
			HandleGameState( SpawnerData , EG_IsBetween<eg_int>( GameVar , SpawnerData.ExistenceRange.x , SpawnerData.ExistenceRange.y ) , true );
		}
	}
}
