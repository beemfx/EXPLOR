// (c) 2020 Beem Media. All Rights Reserved.

#include "ExCombatAI.h"
#include "ExCombatant.h"
#include "EGRandom.h"
#include "ExBeastiary.h"
#include "ExSpellBook.h"

ExCombatant* ExCombatAI_ChooseCombatantToAttack( EGArray<ExCombatant>& Team , EGRandom& Rng , eg_bool bForceInOrder , ex_target_strategy Strategy , eg_bool bRanged )
{
	// Get all potential targets:
	
	struct exAttackChoiceInfo
	{
		ExCombatant* Combatant = nullptr;
		eg_int Weight = 0;
	};

	EGArray<exAttackChoiceInfo> PotentialTargets;
	eg_int TotalWeight = 0;
	eg_int MaxHP = 0;

	auto AddPotentialTarget = [&PotentialTargets,&TotalWeight,&MaxHP,Strategy]( ExCombatant* InComb ) -> void
	{
		if( InComb )
		{
			exAttackChoiceInfo NewTarget;
			NewTarget.Combatant = InComb;

			eg_int WeightOfThis = 0;

			switch( Strategy )
			{
			case ex_target_strategy::Default:
			case ex_target_strategy::Agression:
			case ex_target_strategy::AgressionThenKill:
				WeightOfThis = EG_Max<eg_int>( 1 , InComb->GetAttrValue( ex_attr_t::AGR ) );
				break;
			case ex_target_strategy::PartyOrder:
				break;
			case ex_target_strategy::Kill:
			case ex_target_strategy::HighestHP:
			case ex_target_strategy::LowestHP:
				WeightOfThis = EG_Max<eg_int>( 1 , InComb->GetHP() );
				MaxHP = EG_Max( WeightOfThis , MaxHP );
				break;
			}

			NewTarget.Weight = WeightOfThis;

			TotalWeight += WeightOfThis;

			PotentialTargets.Append( NewTarget );
		}
	};
	
	for( eg_size_t i = 0; i < Team.Len(); i++ )
	{
		const ex_combat_s MemberState = Team[i].GetCombatState();

		switch( MemberState )
		{
		case ex_combat_s::FIRST_ROW:
			AddPotentialTarget( &Team[i] );
			break;
		case ex_combat_s::BACK_ROW:
			if( bRanged )
			{
				AddPotentialTarget( &Team[i] );
			}
			break;
		case ex_combat_s::DOWN:
			// Special case where if we want to kill and a target is unconscious, attack it.
			if( (Strategy == ex_target_strategy::Kill || Strategy == ex_target_strategy::AgressionThenKill) && !Team[i].IsDead() )
			{
				return &Team[i];
			}
			break;
		default:
			// Not in combat.
			break;
		}
	}

	if( Strategy == ex_target_strategy::LowestHP || Strategy == ex_target_strategy::Kill )
	{
		// The low HP strategy isn't particularly reliable since there's no real relative
		// base for the computation. But this does effective cause the targeting to
		// ignore fighters with orders of magnitude greater HP. It's also an unfair
		// strategy in terms of gameplay, but for the "Kill" the idea is that a Killer
		// is relentless.

		TotalWeight = 0;
		const ex_attr_value UpperBound = EGMath_round( MaxHP*1.1 ); // The upper bound is slightly higher so there is some chance of going for the highest HP target.
		for( exAttackChoiceInfo& Info : PotentialTargets )
		{
			Info.Weight = UpperBound - Info.Weight;
			TotalWeight += Info.Weight;
		}
	}

	ExCombatant* AttackChoice = nullptr;
	assert( PotentialTargets.Len() > 0 ); // No one on the front row?
	if( PotentialTargets.Len() > 0 )
	{
		auto GetByWeight = [&PotentialTargets,&TotalWeight,&Team,&Rng]() -> ExCombatant*
		{
			ExCombatant* Out = nullptr;

			assert( TotalWeight >= 1 ); // Bath algorithm?

			const eg_int RandomValue = Rng.GetRandomRangeI( 1 , TotalWeight );
			eg_int WeightConsidered = 0;
			for( eg_size_t i = 0; nullptr == Out && i < PotentialTargets.Len(); i++ )
			{
				const eg_int ThisWeight = PotentialTargets[i].Weight;
				WeightConsidered += ThisWeight;
				if( RandomValue <= WeightConsidered )
				{
					Out = PotentialTargets[i].Combatant;
				}
			}
			assert( nullptr != Out ); // Bad algorithm?

			return Out;
		};
		
		
		if( bForceInOrder )
		{
			AttackChoice = PotentialTargets[0].Combatant;
		}
		else
		{
			switch( Strategy )
			{
				case ex_target_strategy::Default:
				case ex_target_strategy::Agression:
				case ex_target_strategy::AgressionThenKill:
				case ex_target_strategy::HighestHP:
				case ex_target_strategy::LowestHP:
				case ex_target_strategy::Kill:
				{
					AttackChoice = GetByWeight();
				} break;

				case ex_target_strategy::PartyOrder:
				{
					AttackChoice = PotentialTargets[0].Combatant;
				} break;
			}

			if( nullptr == AttackChoice )
			{
				assert( false ); // Bad selection algorithm?
				AttackChoice = PotentialTargets[0].Combatant;
			}
		}
	}

	return AttackChoice;
}

void ExCombatAI_TestChoosCombatantToAttack( const ExGame* InGame )
{
	unused( InGame );
	
	EGLogf( eg_log_t::General , "Testing choose target..." );

	EGArray<ExCombatant> TestTeam;

	ex_attr_value TotalAgr = 0;
	ex_attr_value TotalHp = 0;

	auto AddFighter = [&TestTeam,&TotalAgr,&TotalHp]( ex_attr_value Agr , ex_attr_value Hp ) -> void
	{
		ExFighter Fighter = CT_Default;
		Fighter.SetCreated( true );
		Fighter.SetAttrValue( ex_attr_t::AGR , Agr );
		Fighter.SetAttrValue( ex_attr_t::BHP , Hp );
		Fighter.RestoreAllHP();
		Fighter.RestoreAllMP();
		EGString_Copy( Fighter.LocName , *EGSFormat16( L"Fighter {0}" ,  TestTeam.LenAs<eg_int>() ) , countof(Fighter.LocName) );
		Fighter.ResolveReplicatedData();

		ExCombatant NewCombatant;
		NewCombatant.InitCombatant( nullptr , Fighter , TestTeam.LenAs<eg_uint>() , CT_Clear , TestTeam.LenAs<eg_uint>() );
		NewCombatant.SetCombatState( ex_combat_s::FIRST_ROW );
		TestTeam.Append( NewCombatant );

		TotalAgr += Agr;
		TotalHp += Hp;
	};

	AddFighter( 90 , 1000 );
	AddFighter( 90 , 1000 );
	AddFighter( 20 , 1200 );
	AddFighter( 10 , 60 );
	AddFighter( 30 , 1000 );
	AddFighter( 30 , 100 );

	EGRandom Rng = CT_Default;
	EGItemMap<ExCombatant*,eg_int> Results( 0 );
	for( eg_int i=0; i<10000; i++ )
	{
		ExCombatant* Target = ExCombatAI_ChooseCombatantToAttack( TestTeam , Rng , false , ex_target_strategy::Default , true );
		if( Target )
		{
			Results[Target]++;
		}
		else
		{
			assert( false ); // No target?
		}
	}

	for( eg_int i=0; i<Results.Len(); i++ )
	{
		ExCombatant* Key = Results.GetKeyByIndex( i );
		eg_int Count = Results.GetByIndex( i );

		if( Key )
		{
			EGLogf( eg_log_t::General , "%s (%d,%d): %d" , *eg_d_string8(Key->GetFighter().LocName) , Key->GetAttrValue( ex_attr_t::AGR ) , Key->GetHP() , Count );
		}
		else
		{
			assert( false );
		}
	}
}

const exSpellInfo* ExCombatAI_ChooseSpellToUse( const ExCombatant* Combatant , EGRandom& Rng )
{
	const exSpellInfo* SpellOut = nullptr;

	if( Combatant && ExBeastiary::Get().Contains( Combatant->GetBeastiaryId() ) )
	{
		const exBeastInfo& BeastInfo = ExBeastiary::Get().FindInfo( Combatant->GetBeastiaryId() );	
		const eg_real RandValue = Rng.GetRandomRangeF( 0.f , 100.f );

		eg_real Threshold = 0.f;
		for( const exBeastSkillInfo& Skill : BeastInfo.BehaviorData.Skills )
		{
			Threshold += Skill.ChanceToUse;
			assert( Threshold <= 100.f ); // Should only be up to 100 percentages of casting chance.
			if( RandValue < Threshold )
			{
				SpellOut = &ExSpellBook::Get().FindInfo( Skill.SkillId );
				if( SpellOut->Id.IsNull() )
				{
					assert( false ); // Bad spell Id
					SpellOut = nullptr;
				}
				break;
			}
		}
	}

	return SpellOut;
}
