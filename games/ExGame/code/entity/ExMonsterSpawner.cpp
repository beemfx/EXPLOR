// (c) 2021 Beem Media. All Rights Reserved.

#include "ExMonsterSpawner.h"
#include "ExEnemyController.h"
#include "ExGame.h"
#include "ExMapInfos.h"

EG_CLASS_DECL( ExMonsterSpawner )
EG_CLASS_DECL( ExMonsterSpawnerComponent )

void ExMonsterSpawner::OnCreate( const egEntCreateParms& Parms )
{
	Super::OnCreate( Parms );

	if( IsServer() && Parms.Reason != eg_spawn_reason::GameLoad )
	{
		if( ExMonsterSpawnerComponent* SpComp = FindComponentByClass<ExMonsterSpawnerComponent>() )
		{
			if( ExGame* Game = GetGameData() )
			{
				eg_string_crc EncounterId = CT_Clear;
				eg_bool bShouldStayDead = SpComp->GetMosnterStaysDead();

				switch( SpComp->GetSpawnType() )
				{
				case ex_monster_spawner_t::None:
					EncounterId = CT_Clear;
					break;
				case ex_monster_spawner_t::Monster:
					EncounterId = Game->GetCurrentMapInfo().EncounterData.Fixed;
					break;
				case ex_monster_spawner_t::Boss:
					EncounterId = Game->GetCurrentMapInfo().EncounterData.Boss;
					break;
				case ex_monster_spawner_t::Custom:
					EncounterId = SpComp->GetCustomSpawnType();
					break;
				}

				if( EncounterId.IsNotNull() )
				{
					ExEnemyController* NewMonster = EGCast<ExEnemyController>(Game->SDK_SpawnEnt( eg_crc("ENT_InWorldEnemy") , GetPose() ) );
					assert( NewMonster != nullptr ); // What was this...
					if( NewMonster )
					{
						NewMonster->SetEncounterId( EncounterId );
						NewMonster->SetIsBoss( bShouldStayDead ); // Treat all placed monsters as bosses so they don't come back (fixes city guard issue).

						if( bShouldStayDead )
						{
							ExGame::exDefeatedBoss DefeatedBossInfo;
							DefeatedBossInfo.MapId = Game->GetCurrentMapId();
							DefeatedBossInfo.NavVertex = NewMonster->GetBossVertex();
							if( Game->IsBossDefeated( DefeatedBossInfo ) )
							{
								NewMonster->DeleteFromWorld();
							}
						}
					}
				}
			}
		}
	}
}

void ExMonsterSpawnerComponent::InitComponentFromDef( const egComponentInitData& InitData )
{
	Super::InitComponentFromDef( InitData );

	const ExMonsterSpawnerComponent* DefAsMonsterSpawner = EGCast<ExMonsterSpawnerComponent>(InitData.Def);

	m_SpawnType = DefAsMonsterSpawner->m_SpawnType;
	m_CustomSpawnType = DefAsMonsterSpawner->m_CustomSpawnType;
	m_bMonsterStaysDead = DefAsMonsterSpawner->m_bMonsterStaysDead;
}
