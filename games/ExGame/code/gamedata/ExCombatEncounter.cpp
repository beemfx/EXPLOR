// (c) 2018 Beem Media

#include "ExCombatEncounter.h"
#include "ExBeastiary.h"
#include "EGRandom.h"

EG_CLASS_DECL( ExCombatEncounter )

ExCombatEncounter* ExCombatEncounter::s_Inst = nullptr;

void ExCombatEncounter::Init( eg_cpstr Filename )
{
	assert( nullptr == s_Inst );
	s_Inst = EGDataAsset::LoadDataAsset<ExCombatEncounter>( EGString_ToWide(Filename) );
	assert( s_Inst != nullptr ); // Will crash
	Get();
}

void ExCombatEncounter::Deinit()
{
	assert( s_Inst != nullptr );
	if( s_Inst )
	{
		EGDeleteObject( s_Inst );
	}
	s_Inst = nullptr;
}

const exCombatEncounterInfo* ExCombatEncounter::FindCombatEncounter( const eg_string_crc& EncounterId ) const
{
	for( const exCombatEncounterInfo& Info : m_CombatEncounters )
	{
		if( Info.Id == EncounterId )
		{
			return &Info;
		}
	}
	
	return nullptr;
}

void ExCombatEncounter::OnPropChanged(const egRflEditor & ChangedProperty, egRflEditor & RootEditor, eg_bool & bNeedsRebuildOut)
{
	Super::OnPropChanged( ChangedProperty , RootEditor , bNeedsRebuildOut );

	if( EGString_Equals( ChangedProperty.GetVarName() , "SpawnCountType" ) || EGString_Equals( ChangedProperty.GetVarName() , "Monsters" ) )
	{
		bNeedsRebuildOut = true;
	}
}

void ExCombatEncounter::PostLoad(eg_cpstr16 Filename, eg_bool bForEditor, egRflEditor & RflEditor)
{
	Super::PostLoad( Filename , bForEditor , RflEditor );

	if( !bForEditor )
	{
		eg_size_t TotalNumItems = 0;
		for( const exCombatEncounterCategory& Category : m_Categories )
		{
			TotalNumItems += Category.Items.Len();
		}

		m_CombatEncounters.Reserve( TotalNumItems );

		for( const exCombatEncounterCategory& Category : m_Categories )
		{
			for( const exCombatEncounterInfo& Item : Category.Items )
			{
				m_CombatEncounters.Append( Item );
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

void exCombatEncounterInfo::CreateMonsterList( EGRandom& Rng, EGArray<exCombatEncounterMonster>& Out ) const
{
	auto GetRandomMonsterFromWeight = [this,&Rng,&Out]( eg_int TotalWeight ) -> void
	{
		const eg_int RandomValue = Rng.GetRandomRangeI( 1 , TotalWeight );
		eg_int WeightConsidered = 0;
		for( eg_uint MonsterIdx = 0; MonsterIdx < Monsters.Len(); MonsterIdx++ )
		{
			const exCeMonsterInfo& Info = Monsters[MonsterIdx];
			if( Info.bIsUniqueBoss )
			{
				// Skip boss creatures they are added differently.
			}
			else
			{
				WeightConsidered += Info.Weight;

				if( RandomValue <= WeightConsidered )
				{
					exCombatEncounterMonster NewMonster;
					NewMonster.BeastInfo = &ExBeastiary::Get().FindInfo( Info.MonsterId );
					NewMonster.Level = Rng.GetRandomRangeI( Info.MinLevel , Info.MaxLevel );
					NewMonster.EncounterListIndex = MonsterIdx;
					NewMonster.bIsBoss = Info.bIsUniqueBoss;
					Out.Append( NewMonster );
					return;
				}
			}
		}
	};

	auto SpawnByWeight = [this,&Rng,&Out,&GetRandomMonsterFromWeight]() -> void
	{
		const eg_int NumMonstersToSpawn = EG_Max<eg_int>(1,Rng.GetRandomRangeI( MinMonsters , MaxMonsters ));

		eg_int NumWeights = 0;
		for( eg_uint MonsterIdx=0; MonsterIdx < Monsters.Len(); MonsterIdx++ )
		{
			const exCeMonsterInfo& Info = Monsters[MonsterIdx];

			if( Info.bIsUniqueBoss )
			{
				// If this is a boss creature just add it.
				exCombatEncounterMonster NewMonster;
				NewMonster.BeastInfo = &ExBeastiary::Get().FindInfo( Info.MonsterId );
				NewMonster.Level = Rng.GetRandomRangeI( Info.MinLevel , Info.MaxLevel );
				NewMonster.EncounterListIndex = MonsterIdx;
				NewMonster.bIsBoss = Info.bIsUniqueBoss;
				Out.Append( NewMonster );
			}
			else
			{
				NumWeights += Info.Weight;
			}
		}

		for( eg_int i=0; i<NumMonstersToSpawn; i++ )
		{
			GetRandomMonsterFromWeight( NumWeights );
		}
	};

	SpawnByWeight();

	auto MonsterSortEncounterOrderLevelFn = []( const exCombatEncounterMonster& Left , const exCombatEncounterMonster& Right ) -> eg_bool
	{
		if( Left.bIsBoss != Right.bIsBoss )
		{
			return Left.bIsBoss > Right.bIsBoss;
		}
		if( Left.EncounterListIndex != Right.EncounterListIndex )
		{
			return Left.EncounterListIndex < Right.EncounterListIndex;
		}

		return Left.Level > Right.Level;
	};

	Out.Sort( MonsterSortEncounterOrderLevelFn );
}
