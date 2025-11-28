// (c) 2017 Beem Media

#include "ExCombatant.h"
#include "ExRoster.h"
#include "ExGame.h"
#include "ExCombat.h"
#include "ExSkills.h"
#include "ExGlobalData.h"
#include "EGRandom.h"
#include "ExBeastiary.h"

ExCombatant::ExCombatant() 
: m_Fighter( CT_Clear )
, m_TeamIndex( 0 )
, m_ArrayIndex( 0 )
, m_CombatRoundTime( 0 )
, m_SpeculationCombatRoundTime( 0 )
, m_ServerPartyIndex( 0 )
, m_CombatState( ex_combat_s::OUT_OF_COMBAT )
, m_BeastiaryId( CT_Clear )
, m_bIsBlocking( false )
, m_Boosts( CT_Clear )
{

}

void ExCombatant::InitCombatant( ExCombat* CombatOwner , const ExFighter& FighterIn, eg_uint ArrayIndexIn, eg_string_crc BeastiaryIdIn, eg_uint ServerPartyIndexIn )
{
	m_Fighter = FighterIn;
	m_CombatOwner = CombatOwner;
	m_Fighter.ResolveReplicatedData();
	m_ArrayIndex = ArrayIndexIn;
	m_BeastiaryId = BeastiaryIdIn;
	m_ServerPartyIndex = ServerPartyIndexIn;
	m_CombatState = ex_combat_s::OUT_OF_COMBAT;

	ExSkills* SkillsModule = CombatOwner ? CombatOwner->GetSkillsModule() : nullptr;
	if( SkillsModule )
	{
		SkillsModule->GetSkills( CombatOwner->GetGame() , &m_Fighter , m_Spells , ex_skills_get_type::AvailableForCombat );
	}
}

void ExCombatant::InitCombatInfo( eg_uint TeamIndexIn, eg_uint ArrayIndexIn )
{
	m_TeamIndex = TeamIndexIn;
	m_ArrayIndex = ArrayIndexIn;
	m_Fighter.SetCombatBoosts( &m_Boosts ); // Must set after the array is finalized so it's not pointing at garbage.

	// Some fighters can't fight at the beginning of combat since they were
	// downed in another fight, we'll make sure they start out down.
	if( !CanFight() )
	{
		m_CombatState = ex_combat_s::DOWN;
	}
}

eg_bool ExCombatant::IsInActiveCombat() const
{
	return ( m_CombatState == ex_combat_s::FIRST_ROW || m_CombatState == ex_combat_s::BACK_ROW );
}

eg_bool ExCombatant::CanCast( const exSpellInfo* Spell ) const
{
	return Spell && Spell->GetManaCost( m_Fighter.GetMeleeAttrValue( ex_attr_t::MAG ), m_Fighter.GetAttrValue( ex_attr_t::LVL ) ) <= m_Fighter.GetMP();
}

eg_bool ExCombatant::CanFight() const
{
	return m_Fighter.CanFight();
}

void ExCombatant::GetAvailableActions( ExCombatActionsArray& OutActions )
{
	OutActions.Clear();
	OutActions.Append( ex_combat_actions::FIGHT );
	if( m_Spells.Len() > 0 )
	{
		OutActions.Append( ex_combat_actions::CAST );
	}
	if( m_CombatState == ex_combat_s::FIRST_ROW )
	{
		OutActions.Append( ex_combat_actions::MELEE );
	}
	if( !IsInFrontLines() && IsInShootingRange() && HasRangedAttack() )
	{
		OutActions.Append( ex_combat_actions::SHOOT );
	}
	OutActions.Append( ex_combat_actions::BLOCK );
	OutActions.Append( ex_combat_actions::RUN );
	if( m_CombatState != ex_combat_s::FIRST_ROW )
	{
		OutActions.Append( ex_combat_actions::ADVANCE );
	}
	else if( m_CombatOwner && m_CombatOwner->GetTeamByIndex( m_TeamIndex ).IsFrontRowOverFull() )
	{
		OutActions.Append( ex_combat_actions::BACKAWAY ); // TODO: Only if the front row is over full.
	}

	if( m_TeamIndex == 0 )
	{
		OutActions.Append( ex_combat_actions::VIEW );
		OutActions.Append( ex_combat_actions::ABANDON );
		if( EX_CHEATS_ENABLED )
		{
			OutActions.Append( ex_combat_actions::DEBUG_WIN );
		}
	}
}

eg_bool ExCombatant::SpellCanReach( const exSpellInfo* Spell ) const
{
	return Spell && ( Spell->CanTargetOutOfCombat() || IsInActiveCombat() );
}

void ExCombatant::OnBeginTurn()
{
	m_bIsBlocking = false;
}

void ExCombatant::SetAttackType( ex_attack_t AttackType )
{
	m_Fighter.SetAttackType( AttackType );
}

exDamage ExCombatant::GetAttackDamage() const
{
	return m_Fighter.GetDamage();
}

exDamage ExCombatant::GetFullDefense() const
{
	exDamage Defense = m_Fighter.GetDefense();

	if( m_bIsBlocking )
	{
		Defense.Physical = static_cast<ex_attr_value>(Defense.Physical*EX_BLOCKING_MULTIPLIER);
	}

	return Defense;
}

exDamage ExCombatant::GetRolledDefense( EGRandom& Rng ) const
{
	exDamage RawDefense = GetFullDefense();

	auto RollDefensiveValue = [&Rng]( ex_attr_value& Value ) -> void
	{
		if( Value == 0 )
		{
			return;
		}
		
		enum class ex_defense_tier
		{
			Low,
			Mid,
			High,
		};

		ex_defense_tier DefenseTier = ex_defense_tier::Mid;
		{
			const eg_int TierPct = Rng.GetRandomRangeI( 0 , 100 );
			if( TierPct < 20 )
			{
				DefenseTier = ex_defense_tier::Low;
			}
			else if( TierPct < 80 )
			{
				DefenseTier = ex_defense_tier::Mid;
			}
			else
			{
				DefenseTier = ex_defense_tier::High;
			}
		}

		eg_real DefensePct(1.f);
		switch( DefenseTier )
		{
			case ex_defense_tier::Low:
				DefensePct = Rng.GetRandomRangeF(0.f,.3f);
				break;
			case ex_defense_tier::Mid:
				DefensePct = Rng.GetRandomRangeF(.3f,.75f);
				break;
			case ex_defense_tier::High:
				DefensePct = Rng.GetRandomRangeF(.75f,1.f);
				break;
		}
		ex_attr_value AdjValue = EGMath_ceil(EGMath_GetMappedRangeValue( DefensePct , eg_vec2(0.f,1.f) , eg_vec2( 0.f , EG_To<eg_real>(Value) ) ));
		AdjValue = EG_Clamp( AdjValue , 0 , Value );
		Value = AdjValue;
	};

	if( m_bIsBlocking )
	{
		// Full defense...
	}
	else
	{
		RollDefensiveValue( RawDefense.Physical );
		RollDefensiveValue( RawDefense.Fire );
		RollDefensiveValue( RawDefense.Water );
		RollDefensiveValue( RawDefense.Earth );
		RollDefensiveValue( RawDefense.Air );
	}

	return RawDefense;
}

ex_attr_value ExCombatant::GetAttrValue( ex_attr_t Attr ) const
{
	return EG_Max<ex_attr_value>( m_Fighter.GetAttrValue( Attr ) , 0 );
}

ex_attr_value ExCombatant::GetMeleeAttrValue( ex_attr_t Attr ) const
{
	return m_Fighter.GetMeleeAttrValue( Attr );
}

void ExCombatant::AdjustAggression( ex_attr_value DeltaValue )
{
	AdjustBoostAttr( ex_attr_t::AGR , EGMath_round(DeltaValue*ExGlobalData::Get().GetCombatAggroFactor( m_Fighter.GetClass() )) );
}

void ExCombatant::ResetAgression()
{
	SetBoostAttr( ex_attr_t::AGR , 0 );
}

void ExCombatant::AdjustBoostAttr( ex_attr_t Attr, ex_attr_value DeltaValue )
{
	SetBoostAttr( Attr , GetBoostAttrValue( Attr ) + DeltaValue );
}

void ExCombatant::SetBoostAttr( ex_attr_t Attr, ex_attr_value NewValue )
{
	switch( Attr )
	{
		#define ATTR_ITEM( _var_ , _aname_ , _lname_ , _desc_ ) case ex_attr_t::_var_: m_Boosts._var_ = NewValue; break;
		#include "ExAttrs.items"
	}
}

void ExCombatant::OnAbandonedCombat()
{
	m_Fighter.ApplyRawHit( m_Fighter.GetHP() );
	m_Fighter.SetUnconcious( true );
}

void ExCombatant::ApplyPostSpellBoosts()
{
	m_Fighter.ResolveEquipmentBoosts();
}

void ExCombatant::UpdateFighterOnServer( class ExGame* Game ) const
{
	if( EG_IsBetween<eg_size_t>( m_ServerPartyIndex , 0 , ExRoster::PARTY_SIZE ) )
	{
		ExFighter FighterToClone = m_Fighter;
		if( FighterToClone.IsCreated() )
		{
			FighterToClone.SetCombatBoosts( nullptr );
			FighterToClone.ResolveEquipmentBoosts();
			// If spell boosts put stuff over the maximum, reduce it
			if( FighterToClone.GetMP() >= FighterToClone.GetAttrValue( ex_attr_t::MP ) )
			{
				FighterToClone.RestoreAllMP();
			}
			if( FighterToClone.GetHP() >= FighterToClone.GetAttrValue( ex_attr_t::HP ) )
			{
				FighterToClone.RestoreAllHP();
			}
			Game->ClientUpdateRosterCharacterByPartyIndex( m_ServerPartyIndex , FighterToClone );
		}
	}
}

ex_target_strategy ExCombatant::GetTargetStrategy() const
{
	return m_Fighter.GetTargetStrategy();
}

eg_bool ExCombatant::HasRangedAttack() const
{
	return m_Fighter.HasRangedAttack();
}

void ExCombatant::FormatText( eg_cpstr Flags, class EGTextParmFormatter* Formatter ) const
{
	const eg_cpstr FullFlags = Flags;
	eg_string_crc BaseFlag = Formatter->GetNextFlag( &Flags );

	switch_crc( BaseFlag )
	{
		case_crc("NAME"):
		{
			if( m_BeastiaryId.IsNotNull() && ExBeastiary::Get().Contains( m_BeastiaryId ) )
			{
				const exBeastInfo& BeastInfo = ExBeastiary::Get().FindInfo( m_BeastiaryId );
				Formatter->SetText( eg_loc_text(BeastInfo.Name).GetString() );
			}
			else
			{
				Formatter->SetText( m_Fighter.LocName );
			}
		} break;
		case_crc("ROW"):
		{
			eg_string_crc CombatStateStr( CT_Clear );

			switch( m_CombatState )
			{
			case ex_combat_s::FIRST_ROW:
				CombatStateStr = eg_loc("FrontRowText","Front Row");
				break;
			case ex_combat_s::BACK_ROW:
				CombatStateStr = eg_loc("BackRowText","Back Row");
				break;
			case ex_combat_s::OUT_OF_COMBAT:
				CombatStateStr = eg_loc("OutOfCombatText","Out of Combat");
				break;
			case ex_combat_s::DOWN:
				CombatStateStr = eg_loc("CombatDownText","Down");
				break;
			case ex_combat_s::ESCAPED:
				CombatStateStr = eg_loc("CombatEscapedText","Escaped");
				break;
			}

			Formatter->SetText( eg_loc_text(CombatStateStr).GetString() );
		} break;
		case_crc("SPEED"):
			Formatter->SetNumber( static_cast<eg_int>(m_Fighter.GetAttrValue( ex_attr_t::SPD)) );
			break;
		case_crc("ROUND_TIME"):
			Formatter->SetNumber( static_cast<eg_int>(m_CombatRoundTime) );
			break;
		default:
			m_Fighter.FormatTextInternal( FullFlags , Formatter );
			break;
	}
}

ex_attr_value ExCombatant::GetBoostAttrValue( ex_attr_t Attr ) const
{
	switch( Attr )
	{
		#define ATTR_ITEM( _var_ , _aname_ , _lname_ , _desc_ ) case ex_attr_t::_var_: return m_Boosts._var_;
		#include "ExAttrs.items"
	}

	assert( false );
	return 0;
}
