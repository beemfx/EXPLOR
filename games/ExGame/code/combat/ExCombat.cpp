//
// ExCombat - The Combat System for EXPLOR
// (c) 2016 Beem Media
//
#include "ExCombat.h"
#include "ExGame.h"
#include "ExBeastiary.h"
#include "ExSkills.h"
#include "ExCombatEncounter.h"
#include "ExGlobalData.h"
#include "ExCombatAI.h"
#include "ExAchievements.h"

EG_CLASS_DECL( ExCombat )

eg_int ExCombat::s_SampleCombatPlayerTeamLevel = 0;
ex_class_t ExCombat::s_SampleCombatTeamClass = ex_class_t::Unknown;
EGArray<ExCombat::exSampleCombatMonsterInfo> ExCombat::s_SampleCombatOpponentList;

ExCombat::ExCombat()
: m_CurrentRoundCombatants()
, m_NextCombatantForRound( 0 )
, m_CombatRound( 0 )
, m_Rng( CT_Default )
{
	ClearCombatLog();
}

void ExCombat::AddAnimHandler( IExCombatAnimHandler* Handler )
{
	if( Handler )
	{
		m_AnimHandlers.Append( Handler );
	}
}

void ExCombat::BeginCombat( const ExGame* Game )
{
	ClearCombatLog();
	AppendCombatLog( eg_loc_text( eg_loc("CombatBeginString","|SC(EX_HURT)|Combat Ensues!|RC()|") ) );

	m_Skills = Game ? Game->GetSkillsModule() : nullptr;

	for( eg_uint i = 0; i < countof( m_Teams ); i++ )
	{
		m_Teams[i].PreInit( this );
	}

	if( Game && Game->IsInCombat() )
	{
		CreatePlayerTeam( Game );
		CreateOpposingTeam( Game->GetNextCombatEncounter() );
	}
	else
	{
		CreateSamplePlayerTeam( Game );
		CreateSampleOpposingTeam();
	}

	// Put all active fighters into the combat queue, team A is queued
	// 1st that way if a combatant has the same speed as one of the
	// player's party members, the player will go first.

	for( eg_uint i = 0; i < countof( m_Teams ); i++ )
	{
		if( Game )
		{
			m_Teams[i].Init( i , i==0 ? Game->GetMinFrontLineSize() : Game->GetEnemyFrontLineSize() , i==0 ? Game->GetFrontLineSize() : Game->GetEnemyFrontLineSize() );
		}
		else
		{
			m_Teams[i].Init( i , i==0 ? 3 : 5 , i==0 ? 3 : 5 );
		}
	}

	AdvanceResting();

	if( GetNextCombatant() )
	{
		AppendCombatLog( EGFormat( eg_loc("CombatNextUp","{0:NAME} fights next.") , GetNextCombatant() ) );
	}
}

void ExCombat::ApplyAmbush()
{
	m_bIsAmbush = true;

	ExCombatTeam& PlayerTeam = m_Teams[0];
	const eg_int FightersOnFrontRow = m_Rng.GetRandomRangeI( PlayerTeam.m_FrontRowWidth , ExRoster::PARTY_SIZE );

	// Build index array of all fighters then remove them at random to decide which are on front row
	EGArray<eg_int> FrontRowFighterIndexes;
	for( eg_int i=0; i<PlayerTeam.GetNumCombatants(); i++ )
	{
		if( ExCombatant* Combatant = PlayerTeam.GetCombatantByIndex( i ) )
		{
			if( Combatant->CanFight() )
			{
				FrontRowFighterIndexes.Append( i );
			}
		}
	}

	while( FrontRowFighterIndexes.HasItems() && FrontRowFighterIndexes.Len() > FightersOnFrontRow )
	{
		FrontRowFighterIndexes.DeleteByIndex( m_Rng.GetRandomRangeI( 0 , FrontRowFighterIndexes.LenAs<eg_int>()-1 ) );
	}

	for( eg_int i=0; i < PlayerTeam.GetNumCombatants(); i++ )
	{
		if( ExCombatant* Combatant = PlayerTeam.GetCombatantByIndex( i ) )
		{
			if( Combatant->GetCombatState() == ex_combat_s::FIRST_ROW || Combatant->GetCombatState() == ex_combat_s::BACK_ROW )
			{
				Combatant->SetCombatState( FrontRowFighterIndexes.Contains( i ) ? ex_combat_s::FIRST_ROW : ex_combat_s::BACK_ROW );
			}
		}
	}

	PlayerTeam.AdvanceRanks();
}

ExCombatant* ExCombat::GetCombatant( eg_uint TeamIndex , eg_uint ArrayIndex )
{
	return m_Teams[TeamIndex].GetCombatantByIndex( ArrayIndex );
}

ExCombatant* ExCombat::GetNextCombatant()
{
	return m_CurrentRoundCombatants.IsValidIndex( m_NextCombatantForRound ) ? m_CurrentRoundCombatants[m_NextCombatantForRound] : nullptr;
}

ExCombatant* ExCombat::GetFutureCombatant()
{
	if( m_CurrentRoundCombatants.IsValidIndex( m_NextCombatantForRound + 1 ) )
	{
		return m_CurrentRoundCombatants[m_NextCombatantForRound+1];
	}
	
	return m_SpeculationFirstCombatantNextRound;
}

void ExCombat::ProcessTurn( const exTurnInput& TurnInput )
{
	BeginTurn( TurnInput );
}

void ExCombat::ProcessTurnFor( ExCombatant* Combatant, const exTurnInput& TurnInput )
{
	ExCombatTeam& CurTeam = Combatant->GetTeamIndex() == 0 ? m_Teams[0] : m_Teams[1];
	ExCombatTeam& OppTeam = Combatant->GetTeamIndex() == 0 ? m_Teams[1] : m_Teams[0];

	Combatant->OnBeginTurn();

	if( TurnInput.TurnOpt == ex_combat_turn_opt::AUTO || TurnInput.Action == ex_combat_actions::AUTO )
	{
		const exSpellInfo* SpellToCast = ExCombatAI_ChooseSpellToUse( Combatant , m_Rng );
		ExCombatant* Defender = OppTeam.ChooseCombatantToAttack( false , Combatant->GetTargetStrategy() , Combatant->HasRangedAttack() );

		if( SpellToCast )
		{
			DoCastInternal( Combatant , Defender , SpellToCast );
		}
		else if( Combatant->IsInFrontLines() && Defender && Defender->IsInFrontLinesOrCanBeKilled() )
		{
			DoMeleeCombat( Combatant, Defender );
		}
		else if( Combatant->IsInShootingRange() && Combatant->HasRangedAttack() && Defender && Defender->IsInShootingRangeOrCanBeKilled() )
		{
			DoRangedCombat( Combatant , Defender );
		}
		else
		{
			DoBlock( Combatant );
		}
	}
	else
	{
		switch( TurnInput.Action )
		{
		case ex_combat_actions::UNKNOWN:
		case ex_combat_actions::AUTO:
		{
			assert( false ); // Should have been handled above.
		} break;
		case ex_combat_actions::FIGHT:
		{
			ExCombatant* Defender = OppTeam.ChooseCombatantToAttack( true , Combatant->GetTargetStrategy() , Combatant->HasRangedAttack() );

			if( Combatant->IsInFrontLines() && Defender && Defender->IsInFrontLines() )
			{
				DoMeleeCombat( Combatant, Defender );
			}
			else if( Combatant->IsInShootingRange() && Combatant->HasRangedAttack() && Defender && Defender->IsInShootingRange() )
			{
				DoRangedCombat( Combatant , Defender );
			}
			else
			{
				DoBlock( Combatant );
			}
		} break;
		case ex_combat_actions::BLOCK:
		{
			DoBlock( Combatant );
		} break;
		case ex_combat_actions::MELEE:
		{
			DoMeleeCombat( Combatant , TurnInput.Target ); // This function will assert if this wasn't allow, in which case the menu code has a bug in it.
		} break;
		case ex_combat_actions::SHOOT:
		{
			DoRangedCombat( Combatant , TurnInput.Target );
		} break;
		case ex_combat_actions::CAST:
		{
			DoCast( Combatant , TurnInput );
		} break;
		case ex_combat_actions::RUN:
		{
			DoRun( Combatant );
		} break;
		case ex_combat_actions::ADVANCE:
		{
			DoAdvance( Combatant );
		} break;
		case ex_combat_actions::BACKAWAY:
		{
			DoBackAway( Combatant );
		} break;
		case ex_combat_actions::ABANDON:
		{
			for( ExCombatant& TeamMember : CurTeam.m_TeamMembers )
			{
				if( TeamMember.GetCombatState() != ex_combat_s::ESCAPED )
				{
					TeamMember.OnAbandonedCombat();
				}
			}
		} break;
		case ex_combat_actions::VIEW:
		{
			assert( false ); // This should be handled by the combat menu, not here
		} break;
		case ex_combat_actions::DEBUG_WIN:
		{
			for( ExCombatant& TeamMember : OppTeam.m_TeamMembers )
			{
				TeamMember.SetDead( true );
				ChangeFighterState( &TeamMember , ex_combat_s::DOWN , false );
				HandleXPForDownedCombatant( &TeamMember );
			}
		} break;
		}
	}

	CullDeadFighters();
}

void ExCombat::AdvanceRanks()
{
	for( eg_size_t i = 0; i < countof( m_Teams ); i++ )
	{
		m_Teams[i].AdvanceRanks();
	}
}

void ExCombat::CullDeadFighters()
{
	// We'll Cull downed fighters...
	EGFixedArray<ExCombatant*, MAX_ACTIVE_COMBAT_FIGHTERS> CullList( CT_Clear );

	for( ExCombatTeam& Team : m_Teams )
	{
		for( ExCombatant& Combatant : Team.m_TeamMembers )
		{
			if( Combatant.IsInActiveCombat() && !Combatant.CanFight() )
			{
				CullList.Append( &Combatant );
			}
		}
	}

	for( ExCombatant* Combatant : CullList )
	{
		ChangeFighterState( Combatant , ex_combat_s::DOWN , true );
	}

	// Restore raised fighters (we'll move them to out of combat, and AdvanceRanks will put them where they should be)
	CullList.Clear();

	for( ExCombatant& Combatant : m_Teams[0].m_TeamMembers )
	{
		if( Combatant.GetCombatState() == ex_combat_s::DOWN && Combatant.CanFight() )
		{
			CullList.Append( &Combatant );
		}
	}

	for( ExCombatant* Combatant : CullList )
	{
		ChangeFighterState( Combatant , ex_combat_s::OUT_OF_COMBAT , false );
	}

	AdvanceRanks();
}

void ExCombat::DoCast( ExCombatant* Attacker, const exTurnInput& TurnInput )
{	
	if( Attacker && Attacker->GetSpells().IsValidIndex( TurnInput.ActionIndex) )
	{
		Attacker->SetAttackType( ex_attack_t::MELEE );
		const ex_attr_value AttackerMag = Attacker->GetAttrValue( ex_attr_t::MAG );
		const ex_attr_value AttackerLvl = Attacker->GetAttrValue( ex_attr_t::LVL );
		Attacker->SetAttackType( ex_attack_t::NONE );

		const exSpellInfo* Spell = Attacker->GetSpells()[TurnInput.ActionIndex];
		const ex_attr_value ManaCost = Spell ? Spell->GetManaCost( AttackerMag , AttackerLvl) : 0;

		assert( ManaCost <= Attacker->GetMP() ); // Menu should not have allowed this attack to happen.
		Attacker->UseMP( ManaCost );

		DoCastInternal( Attacker , TurnInput.Target , Spell );
	}
	else
	{
		assert( false ); // Menu gave us a bad spell cast.
	}
}

void ExCombat::DoCastInternal( ExCombatant* Attacker , ExCombatant* ChosenTarget , const exSpellInfo* Spell )
{
	if( nullptr == Attacker )
	{
		assert( false );
		return;
	}

	auto GetFightersInCombat = []( EGArray<ExCombatant*>& Out , ExCombatTeam& Team , eg_uint Count , eg_bool bActiveOnly ) -> void
	{
		eg_uint NumAdded = 0;
		for( eg_uint i=0; i<Team.GetNumCombatants() && NumAdded < Count; i++ )
		{
			ExCombatant* Target = Team.GetCombatantByIndex( i );
			if( Target && (!bActiveOnly || Target->IsInActiveCombat()) )
			{
				Out.Append( Target );
				NumAdded++;
			}
		}
	};

	AppendCombatLog( EGFormat( eg_loc("CombatCastString","{0:NAME} casts {1:NAME} and") , Attacker , Spell ) );

	Attacker->SetAttackType( ex_attack_t::MELEE ); // This way boosts from equipment will apply
	const ex_attr_value AttackerMag = Attacker->GetAttrValue( ex_attr_t::MAG );
	const ex_attr_value AttackerLvl = Attacker->GetAttrValue( ex_attr_t::LVL );
	Attacker->SetAttackType( ex_attack_t::NONE );

	ExCombatTeam& CurTeam = Attacker->GetTeamIndex() == 0 ? m_Teams[0] : m_Teams[1];
	ExCombatTeam& OppTeam = Attacker->GetTeamIndex() == 0 ? m_Teams[1] : m_Teams[0];

	exDamage Damage( CT_Clear );
	if( Spell->Type == ex_spell_t::CombatDamage || Spell->Type == ex_spell_t::Resurrect )
	{
		Damage = Spell->GetDamage( AttackerMag , AttackerLvl );
	}
	eg_bool bIsHeal = Spell->Type == ex_spell_t::CombatDamage && Damage.Physical < 0;

	EGArray<ExCombatant*> Targets;
	EGArray<ExCombatant*> DownedTargets;

	if( Spell->TargetCount == 1 && Spell->Target != ex_spell_target::AllActive && Spell->Target != ex_spell_target::Self )
	{
		ExCombatant* SingleTarget = ChosenTarget;
		if( SingleTarget == nullptr )
		{
			assert( false ); // If 1 thing is targeted for this spell it should be passed in, we'll just choose the first target.
			SingleTarget = Spell->CanTargetEnemy() ? OppTeam.ChooseCombatantToAttack( true , Attacker->GetTargetStrategy() , true ) : CurTeam.ChooseCombatantToAttack( true , Attacker->GetTargetStrategy() , true );
		}
		Targets.Append( SingleTarget );
	}
	else
	{
		switch( Spell->Target )
		{
		case ex_spell_target::None: 
		break;
		case ex_spell_target::EnemyActive:
		GetFightersInCombat( Targets , OppTeam , Spell->TargetCount , true );
		break;
		case ex_spell_target::EnemyAny:
		GetFightersInCombat( Targets , OppTeam , Spell->TargetCount , false );
		break;
		case ex_spell_target::AllyActive: 
		GetFightersInCombat( Targets , CurTeam , Spell->TargetCount , true );
		break;
		case ex_spell_target::AllyAny:
		GetFightersInCombat( Targets , CurTeam , Spell->TargetCount , false );
		break;
		case ex_spell_target::AllActive:
		GetFightersInCombat( Targets , CurTeam , 1000000 , true );
		GetFightersInCombat( Targets , OppTeam , 1000000 , true );
		break;
		case ex_spell_target::AllAny:
		GetFightersInCombat( Targets , CurTeam , 1000000 , false );
		GetFightersInCombat( Targets , OppTeam , 1000000 , false );
		break;
		case ex_spell_target::Self:
		Targets.Append( Attacker );
		break;
		}
	}

	switch( Spell->Type )
	{
	case ex_spell_t::Unknown:
	{
	} break;
	case ex_spell_t::AttributeBoost:
	{
		// Attribute boost does not allow wild stacking.

		{
			eg_bool bHadOne = false;
			auto AppendBoostLog = [&bHadOne,this]( ex_attr_t AttrType , ex_attr_value Value ) -> void
			{
				if( bHadOne )
				{
					AppendCombatLog( EGFormat(eg_loc("CombatAdditionalBoostAmountText","and |SC(EX_ATTR)|{0} {1}|RC()|") , Value , ExCore_GetAttributeName( AttrType ) ) );
				}
				else
				{
					bHadOne = true;
					AppendCombatLog( EGFormat(eg_loc("CombatBoostAmountText","boosts |SC(EX_ATTR)|{0} {1}|RC()|") , Value , ExCore_GetAttributeName( AttrType ) ) );
				}
			};
			#define ATTR_ITEM( _var_ , _aname_ , _lname_ , _desc_ ) if( Spell->AttributesEffected._var_ != 0 ){ AppendBoostLog( ex_attr_t::_var_ , Spell->GetAttrBoost( ex_attr_t::_var_ , AttackerMag , AttackerLvl  ) ); }
			#include "ExAttrs.items"

			if( Targets.Len() == 1 )
			{
				AppendCombatLog( EGFormat( eg_loc("CombatBoostsSingleText","on {0:NAME}.") , Targets[0] ) );
			}
			else
			{
				AppendCombatLog( EGFormat( eg_loc("CombatBoostsMultipleText","on {0} fighters.") , Targets.Len() ) );
			}
		}

		for( ExCombatant* Target : Targets )
		{
			ex_attr_value PrevMaxHp = Target->GetAttrValue( ex_attr_t::HP );
			ex_attr_value PrevMaxMp = Target->GetAttrValue( ex_attr_t::MP );
			eg_real HpPct = PrevMaxHp != 0 ? static_cast<eg_real>(Target->GetHP())/PrevMaxHp : 0.f;
			eg_real MpPct = PrevMaxMp != 0 ? static_cast<eg_real>(Target->GetMP())/PrevMaxMp : 0.f;

			#define ATTR_ITEM( _var_ , _aname_ , _lname_ , _desc_ ) if( Spell->AttributesEffected._var_ != 0 ){ Target->SetBoostAttr( ex_attr_t::_var_ , Spell->GetAttrBoost( ex_attr_t::_var_ , AttackerMag , AttackerLvl  ) ); }
			#include "ExAttrs.items"

			// In case he spell boost HP etc, we don't want it to look like
			// the fighter took damage, so we actually award HP or MP so that
			// it's about the percentage is the same.

			ex_attr_value NewMaxHp = Target->GetAttrValue( ex_attr_t::HP );
			ex_attr_value NewMaxMp = Target->GetAttrValue( ex_attr_t::MP );

			if( NewMaxHp != PrevMaxHp )
			{
				ex_attr_value Adjusted = EGMath_ceil(HpPct * NewMaxHp) - Target->GetHP();
				if( Adjusted > 0 )
				{
					Target->ApplyRawHeal( Adjusted );
				}
			}

			if( NewMaxMp != PrevMaxMp )
			{
				ex_attr_value Adjusted = EGMath_ceil(MpPct * NewMaxMp) - Target->GetMP();
				if( Adjusted > 0 )
				{
					Target->ApplyManaRestoration( Adjusted );
				}
			}

			Target->ApplyPostSpellBoosts();

			exCombatAnimInfo AnimInfo;
			AnimInfo.Type = ex_combat_anim::SpellBonus;
			AnimInfo.Damage = 0;
			AnimInfo.Attacker = Attacker;
			AnimInfo.Defender = Target;
			HandleAnim( AnimInfo );
		}



	} break;
	case ex_spell_t::CombatDamage:
	{
		ex_attr_value TotalHit = 0;
		ex_attr_t ResistType = ex_attr_t::DEF_;

		for( ExCombatant* Target : Targets )
		{
			const exDamage RolledDef = bIsHeal ? exDamage() : Target->GetRolledDefense( m_Rng );
			ResistType = !bIsHeal ? Target->GetFullDefense().GetResistType() : ex_attr_t::DEF_; // We use the full defense to decide which resistance to show.
			const ex_attr_value Hit = bIsHeal ? Damage.Physical : (Damage - RolledDef);
			// Show resistance if damage was taken on the resistance type and the target wasn't killed.
			const eg_bool bShowResisted = !bIsHeal && ResistType != ex_attr_t::DEF_ && Damage.GetAttrValue( ResistType ) > 0 && Hit < Target->GetHP();

			if( !bShowResisted )
			{
				ResistType = ex_attr_t::DEF_;
			}

			if( !bIsHeal || !Target->IsDead() )
			{
				TotalHit += Hit;
			}
			if( !bIsHeal )
			{
				Attacker->AdjustAggression( Hit );
				if( !bIsHeal && (Target->GetHP() <= 0) ) // When a fighter with 0 hit points is hit, they are killed.
				{
					Target->SetDead( true );
					DownedTargets.Append( Target );
				}
				else
				{
					Target->ApplyRawHit( Hit );
					if( Target->GetHP() <= 0 )
					{
						Target->SetUnconcious( true );
						DownedTargets.Append( Target );
					}
				}

				exCombatAnimInfo AnimInfo;
				AnimInfo.Type = ex_combat_anim::SpellAttack;
				AnimInfo.Damage = Hit;
				AnimInfo.Attacker = Attacker;
				AnimInfo.Defender = Target;
				HandleAnim( AnimInfo );

				if( bShowResisted )
				{
					exCombatAnimInfo AnimInfo;
					AnimInfo.Type = ex_combat_anim::Resist;
					AnimInfo.Damage = static_cast<ex_attr_value>(ResistType);
					AnimInfo.Attacker = Attacker;
					AnimInfo.Defender = Target;
					HandleAnim( AnimInfo );
				}
			}
			else
			{
				if( !Target->IsDead() )
				{
					const eg_bool bWasAtZero = Target->GetHP() <= 0;
					Target->ApplyRawHeal( -Hit );
					if( Target->IsUnconcious() )
					{
						ExAchievements::Get().UnlockAchievement( eg_crc("ACH_AWAKE") );
					}
					Target->SetUnconcious( false );
					if( bWasAtZero )
					{
						if( Target->GetArrayIndex() < CurTeam.GetFrontRowWidth() )
						{
							ChangeFighterState( Target , ex_combat_s::FIRST_ROW , true );
						}
						else
						{
							ChangeFighterState( Target , ex_combat_s::BACK_ROW , true );
						}
					}

					exCombatAnimInfo AnimInfo;
					AnimInfo.Type = ex_combat_anim::SpellBonus;
					AnimInfo.Damage = -Hit;
					AnimInfo.Attacker = Attacker;
					AnimInfo.Defender = Target;
					HandleAnim( AnimInfo );
				}
				else
				{
					if( Targets.Len() == 1 )
					{
						exCombatAnimInfo AnimInfo;
						AnimInfo.Type = ex_combat_anim::SpellFail;
						AnimInfo.Damage = 0;
						AnimInfo.Attacker = Attacker;
						AnimInfo.Defender = Target;
						HandleAnim( AnimInfo );
					}
				}
			}
		}

		if( bIsHeal )
		{
			if( Targets.Len() == 1 )
			{
				AppendCombatLog( EGFormat( eg_loc("CombatHealSingle","restored |SC(EX_HEAL)|{1} HP|RC()| to {0:NAME}.") , Targets[0] , -TotalHit ) );
			}
			else
			{
				AppendCombatLog( EGFormat( eg_loc("CombatHealMultiple","restored |SC(EX_HEAL)|{1} HP|RC()| to {0} fighters.") , Targets.Len() , -TotalHit ) );
			}
		}
		else
		{
			if( Targets.Len() == 1 )
			{
				if( ResistType != ex_attr_t::DEF_ )
				{
					AppendCombatLog( EGFormat( eg_loc("CombatSpellDamageSingleWithResist","hit {0:NAME} damaging |SC(RED)|{1} HP|RC()|, |SC({3})|{2}|RC()| was resisted.") , Targets[0] , TotalHit , ExCore_GetAttributeName( exDamage::GetComplimentaryDmgOrDef( ResistType ) ) , ExCore_GetAttributeNameFormatColor( exDamage::GetComplimentaryDmgOrDef( ResistType ) ) ) );
				}
				else
				{
					AppendCombatLog( EGFormat( eg_loc("CombatSpellDamageSingle","hit {0:NAME} damaging |SC(RED)|{1} HP|RC()|.") , Targets[0] , TotalHit ) );
				}
			}
			else
			{
				AppendCombatLog( EGFormat( eg_loc("CombatSpellDamageMultiple","hit {0} fighters damaging |SC(RED)|{1} HP|RC()|.") , Targets.Len() , TotalHit ) );
			}
		}

		if( DownedTargets.Len() == 1 )
		{
			if( DownedTargets[0]->IsDead() )
			{
				AppendCombatLog( EGFormat( eg_loc("CombatWasKilled","{0:NAME} was killed.") , DownedTargets[0] ) );
			}
			else
			{
				AppendCombatLog( EGFormat( eg_loc("GoesDownString","{0:NAME} goes down.") , DownedTargets[0] ) );
			}
		}
		else if( DownedTargets.Len() > 1 )
		{
			if( DownedTargets.Len() >= 5 )
			{
				ExAchievements::Get().UnlockAchievement( eg_crc("ACH_POWERSPELL") );
			}
			AppendCombatLog( EGFormat( eg_loc("CombatMultipleDownString","{0} fighters go down.") , DownedTargets.Len() ) );
		}
	} break;

	case ex_spell_t::Resurrect:
	{
		EGArray<ExCombatant*> RaisedFighters;

		for( ExCombatant* Target : Targets )
		{
			if( Target && Target->IsDead() )
			{
				const eg_bool bWasAtZero = Target->GetHP() <= 0;
				Target->SetDead( false );
				Target->SetUnconcious( false );
				Target->ApplyRawHeal( EG_Abs( Damage.Physical ) );
				if( bWasAtZero )
				{
					if( Target->GetArrayIndex() < CurTeam.GetFrontRowWidth() )
					{
						ChangeFighterState( Target , ex_combat_s::FIRST_ROW , true );
					}
					else
					{
						ChangeFighterState( Target , ex_combat_s::BACK_ROW , true );
					}
				}
				RaisedFighters.Append( Target );

				exCombatAnimInfo AnimInfo;
				AnimInfo.Type = ex_combat_anim::SpellBonus;
				AnimInfo.Damage = EG_Abs( Damage.Physical );
				AnimInfo.Attacker = Attacker;
				AnimInfo.Defender = Target;
				HandleAnim( AnimInfo );

				ExAchievements::Get().UnlockAchievement( eg_crc("ACH_RESURRECT") );
			}
			else
			{
				if( Targets.Len() == 1 )
				{
					exCombatAnimInfo AnimInfo;
					AnimInfo.Type = ex_combat_anim::SpellFail;
					AnimInfo.Damage = 0;
					AnimInfo.Attacker = Attacker;
					AnimInfo.Defender = Target;
					HandleAnim( AnimInfo );
				}
			}
		}

		if( RaisedFighters.Len() > 1 )
		{
			AppendCombatLog( EGFormat( eg_loc("CombatSpellResurrectMultiple","resurrected {0} fighters.") , RaisedFighters.LenAs<eg_int>() ) );
		}
		else if( RaisedFighters.Len() == 1 )
		{
			AppendCombatLog( EGFormat( eg_loc("CombatSpellResurrectSingleTarget","resurrected {0:NAME}.") , RaisedFighters[0] ) );
		}
		else
		{
			AppendCombatLog( eg_loc_text(eg_loc("CombatSpellResurrectNoOne","there was no one to resurrect.")) );
		}

	} break;

	case ex_spell_t::TownPortalMarkAndGo:
	case ex_spell_t::TownPortalRecall:
	assert( false ); // Should not be available in combat.
	break;

	case ex_spell_t::FortifyRanks:
	{
		ExCombatTeam* TargetTeam = nullptr;
		if( Spell->Target == ex_spell_target::AllyActive )
		{
			TargetTeam = &CurTeam;
		}
		else if( Spell->Target == ex_spell_target::EnemyActive )
		{
			TargetTeam = &OppTeam;
		}

		if( TargetTeam )
		{
			for( eg_int i=0; i < TargetTeam->GetNumCombatants(); i++ )
			{
				exCombatAnimInfo AnimInfo;
				AnimInfo.Type = ex_combat_anim::SpellBonus;
				AnimInfo.Damage = 0;
				AnimInfo.Attacker = Attacker;
				AnimInfo.Defender = TargetTeam->GetCombatantByIndex( i );
				HandleAnim( AnimInfo );
			}

			TargetTeam->ApplySpellFortifyRanks();
		}
		else
		{
			assert( false ); // Only works for AllyActive or EnemyActive
		}
	} break;
	case ex_spell_t::ExpandRanks:
	{
		ExCombatTeam* TargetTeam = nullptr;
		if( Spell->Target == ex_spell_target::AllyActive )
		{
			TargetTeam = &CurTeam;
		}
		else if( Spell->Target == ex_spell_target::EnemyActive )
		{
			TargetTeam = &OppTeam;
		}

		if( TargetTeam )
		{
			for( eg_int i=0; i < TargetTeam->GetNumCombatants(); i++ )
			{
				exCombatAnimInfo AnimInfo;
				AnimInfo.Type = ex_combat_anim::SpellAttack;
				AnimInfo.Damage = 0;
				AnimInfo.Attacker = Attacker;
				AnimInfo.Defender = TargetTeam->GetCombatantByIndex( i );
				HandleAnim( AnimInfo );
			}

			TargetTeam->ApplySpellExpandRanks();
		}
		else
		{
			assert( false ); // Only works for AllyActive or EnemyActive
		}
	} break;
	}

	for( ExCombatant* Combatant : DownedTargets )
	{
		exCombatAnimInfo AnimInfo;
		AnimInfo.Type = ex_combat_anim::AnyDeath;
		AnimInfo.Attacker = Attacker;
		AnimInfo.Defender = Combatant;
		AnimInfo.Damage = 0;
		HandleAnim( AnimInfo );
		HandleXPForDownedCombatant( Combatant );
	}
}

void ExCombat::DoMeleeCombat( ExCombatant* Attacker, ExCombatant* Defender )
{
	if( Attacker && Defender && Attacker->GetTeamIndex() != Defender->GetTeamIndex() && Attacker->IsInFrontLines() && Defender->IsInFrontLinesOrCanBeKilled() )
	{
		Attacker->SetAttackType( ex_attack_t::MELEE );
		Defender->SetAttackType( ex_attack_t::MELEE );

		exDamage Damage = Attacker->GetAttackDamage();
		exDamage Defense = Defender->GetRolledDefense( m_Rng );

		ex_attr_value Hit =  Damage - Defense;
		eg_int CriticalPercent = ExGlobalData::Get().GetCriticalPercent( Attacker->GetFighter().GetClass() );
		ex_attr_value DefenderHP = Defender->GetHP();
		eg_bool bCriticalBoost = false;
		if( CriticalPercent > 0 && (DefenderHP - Hit) > 0 )
		{
			ex_attr_value CriticalHit = EGMath_ceil( ((100+CriticalPercent)/100.f) * Hit );
			if( (DefenderHP - CriticalHit) <= 0 )
			{
				Hit = CriticalHit;
				bCriticalBoost = true;
			}
		}

		exCombatAnimInfo AnimInfo;
		AnimInfo.Type = ex_combat_anim::MeleeAttack;
		AnimInfo.Attacker = Attacker;
		AnimInfo.Defender = Defender;
		AnimInfo.Damage = Hit;
		HandleAnim( AnimInfo );
		if( bCriticalBoost )
		{
			AnimInfo.Type = ex_combat_anim::CriticalHit;
			HandleAnim( AnimInfo );
		}

		eg_string_crc AttackString = eg_loc("MeleeAttackStringFull","{0:NAME} attacked {1:NAME} with {2} DAM, was blocked by {3} DEF, and damaged |SC(EX_HURT)|{4} HP|RC()|.");
		if( m_Verbosity == ex_combat_log_verbosity::Minimal )
		{
			if( Hit == 0 )
			{
				AttackString = eg_loc("MeleeAttackStringMinimalMissed","{0:NAME} attacked {1:NAME} and missed.");
			}
			else
			{
				AttackString = eg_loc("MeleeAttackStringMinimal","{0:NAME} attacked {1:NAME} damaging |SC(EX_HURT)|{4} HP|RC()|.");
			}
		}
		AppendCombatLog( EGFormat( AttackString , Attacker , Defender , Damage.GetRawTotal() , Defense.GetRawTotal() , Hit ) );

		if( Attacker->GetFighter().GetClass() == ex_class_t::Thief )
		{
			const eg_int GoldBurgled = HandleBurgleOfCombatant( Defender );
			if( GoldBurgled > 0 )
			{
				static const eg_string_crc BurgleString = eg_loc("BurgleString","|SC(EX_GOLD)|{0} gold|RC()| was burgled.");
				AppendCombatLog( EGFormat( BurgleString , GoldBurgled ) );
				exCombatAnimInfo BurgleAnimInfo = AnimInfo;
				BurgleAnimInfo.Type = ex_combat_anim::Burgled;
				BurgleAnimInfo.Damage = GoldBurgled;
				HandleAnim( BurgleAnimInfo );
			}
		}

		Attacker->AdjustAggression( Hit );

		if( Defender->GetHP() == 0 ) // When a fighter with 0 hit points is hit, they are killed.
		{
			Defender->SetDead( true );
			AppendCombatLog( EGFormat( eg_loc("CombatWasKilled","{0:NAME} was killed.") , Defender ) );
		}
		else
		{
			Defender->ApplyRawHit( Hit );
			if( Defender->GetHP() <= 0 )
			{
				Defender->SetUnconcious( true );
				AppendCombatLog( EGFormat( eg_loc("GoesDownString","{0:NAME} goes down.") , Defender ) );
				AnimInfo.Type = ex_combat_anim::AnyDeath;
				AnimInfo.Damage = 0;
				HandleAnim( AnimInfo );
				HandleXPForDownedCombatant( Defender );
			}
		}

		Attacker->SetAttackType( ex_attack_t::NONE );
		Defender->SetAttackType( ex_attack_t::NONE );
	}
	else
	{
		assert( false ); // The conditions were not correct for a melee attack, both fighters must be on the front row and on opposite teams, and must exist.
	}
}

void ExCombat::DoRangedCombat( ExCombatant* Attacker , ExCombatant* Defender )
{
	if( Attacker && Defender && Attacker->GetTeamIndex() != Defender->GetTeamIndex() && Attacker->IsInShootingRange() && Defender->IsInShootingRangeOrCanBeKilled() )
	{
		Attacker->SetAttackType( ex_attack_t::RANGED );
		Defender->SetAttackType( ex_attack_t::RANGED );

		exDamage Damage = Attacker->GetAttackDamage();
		exDamage Defense = Defender->GetRolledDefense( m_Rng );

		ex_attr_value Hit =  Damage - Defense;
		eg_int CriticalPercent = ExGlobalData::Get().GetCriticalPercent( Attacker->GetFighter().GetClass() );
		ex_attr_value DefenderHP = Defender->GetHP();
		eg_bool bCriticalBoost = false;
		if( CriticalPercent > 0 && (DefenderHP - Hit) > 0 )
		{
			ex_attr_value CriticalHit = EGMath_ceil( ((100+CriticalPercent)/100.f) * Hit );
			if( (DefenderHP - CriticalHit) <= 0 )
			{
				Hit = CriticalHit;
				bCriticalBoost = true;
			}
		}

		exCombatAnimInfo AnimInfo;
		AnimInfo.Type = ex_combat_anim::MeleeAttack;
		AnimInfo.Attacker = Attacker;
		AnimInfo.Defender = Defender;
		AnimInfo.Damage = Hit;
		HandleAnim( AnimInfo );
		if( bCriticalBoost )
		{
			AnimInfo.Type = ex_combat_anim::CriticalHit;
			HandleAnim( AnimInfo );
		}

		eg_string_crc AttackString = eg_loc("RangedAttackStringFull","{0:NAME} shot {1:NAME} with {2} DAM, was blocked by {3} DEF, and damaged |SC(EX_HURT)|{4} HP|RC()|.");
		if( m_Verbosity == ex_combat_log_verbosity::Minimal )
		{
			if( Hit == 0 )
			{
				AttackString = eg_loc("RangedAttackStringMinimalMissed","{0:NAME} shot {1:NAME} and missed.");
			}
			else
			{
				AttackString = eg_loc("RangedAttackStringMinimal","{0:NAME} shot {1:NAME} damaging |SC(EX_HURT)|{4} HP|RC()|.");
			}
		}
		AppendCombatLog( EGFormat( AttackString , Attacker , Defender , Damage.GetRawTotal() , Defense.GetRawTotal() , Hit ) );

		Attacker->AdjustAggression( Hit );

		if( Defender->GetHP() == 0 ) // When a fighter with 0 hit points is hit, they are killed.
		{
			Defender->SetDead( true );
			AppendCombatLog( EGFormat( eg_loc("CombatWasKilled","{0:NAME} was killed.") , Defender ) );
		}
		else
		{
			Defender->ApplyRawHit( Hit );
			if( Defender->GetHP() <= 0 )
			{
				Defender->SetUnconcious( true );
				AppendCombatLog( EGFormat( eg_loc("GoesDownString","{0:NAME} goes down.") , Defender ) );
				AnimInfo.Type = ex_combat_anim::AnyDeath;
				HandleAnim( AnimInfo );
				HandleXPForDownedCombatant( Defender );
			}
		}

		Attacker->SetAttackType( ex_attack_t::NONE );
		Defender->SetAttackType( ex_attack_t::NONE );
	}
	else
	{
		assert( false ); // The conditions were not correct for a ranged attack, both fighters must be on the front row and on opposite teams, and must exist.
	}
}

void ExCombat::DoBlock( ExCombatant* Attacker )
{
	if( Attacker )
	{
		Attacker->SetIsBlocking( true );
		AppendCombatLog( EGFormat( eg_loc("BlockString","{0:NAME} blocks.") , Attacker ) );
	}
}

void ExCombat::DoRun( ExCombatant* Attacker )
{
	if( Attacker )
	{
		const eg_bool bRunSucceded = m_Rng.GetRandomRange(0.f,100.f) <= ExGlobalData::Get().GetCombatFleeChance( Attacker->GetCombatState() == ex_combat_s::FIRST_ROW );

		if( bRunSucceded )
		{
			ChangeFighterState( Attacker , ex_combat_s::ESCAPED , true );
			AppendCombatLog( EGFormat( eg_loc("RunStringSuccess","{0:NAME} made a run for it and escaped.") , Attacker ) );
		}
		else
		{
			AppendCombatLog( EGFormat( eg_loc("RunStringFailure","{0:NAME} made a run for it but could not find an escape route.") , Attacker ) );
		}
	}
}

void ExCombat::DoAdvance( ExCombatant* Attacker )
{
	if( Attacker )
	{
		const eg_bool bAdvanceSucceeded = m_Rng.GetRandomRange(0.f,100.f) <= ExGlobalData::Get().GetCombatAdvanceChance();

		if( bAdvanceSucceeded )
		{
			ChangeFighterState( Attacker , ex_combat_s::FIRST_ROW , true );
			AppendCombatLog( EGFormat( eg_loc("AdvanceStringSuccess","{0:NAME} advanced ranks.") , Attacker ) );
		}
		else
		{
			AppendCombatLog( EGFormat( eg_loc("AdvanceStringFailure","{0:NAME} couldn't find an opening to advance ranks.") , Attacker ) );
		}
	}
}

void ExCombat::DoBackAway( ExCombatant* Attacker )
{
	if( Attacker )
	{
		const eg_bool bBackAwaySucceeded = m_Rng.GetRandomRange(0.f,100.f) <= ExGlobalData::Get().GetCombatBackAwayChance();

		if( bBackAwaySucceeded )
		{
			ChangeFighterState( Attacker , ex_combat_s::BACK_ROW , true );
			AppendCombatLog( EGFormat( eg_loc("CombatBackAwayStringSuccess","{0:NAME} backs away.") , Attacker ) );
		}
		else
		{
			AppendCombatLog( EGFormat( eg_loc("CombatBackAwayStringFailure","{0:NAME} couldn't find an opening to back away.") , Attacker ) );
		}
	}
}

void ExCombat::BeginTurn( const exTurnInput& TurnInput )
{
	ClearCombatLog();

	// Do turn for combatant.
	{
		ExCombatant* CombatantForTurn = GetNextCombatant();

		if( nullptr == CombatantForTurn )
		{
			assert( false ); // No fighters were in active combat?
			return;
		}

		if( CombatantForTurn->IsReadyForCombatTurn() )
		{
			ProcessTurnFor( CombatantForTurn, TurnInput );
			CombatantForTurn->ApplyCombatTurnCompleted();
			m_NextCombatantForRound++;
		}
		else
		{
			assert( false ); // Resting should have been advanced before doing the turn.
		}
	}

	if( nullptr == GetNextCombatant() )
	{
		AdvanceResting();
	}

	if( IsCombatComplete() )
	{
		ProcessCombatResults();
		AppendCombatLog( eg_loc_text(eg_loc("CombatComplete","The combat was completed.")) );
	}
	else if( GetNextCombatant() )
	{
		AppendCombatLog( EGFormat( eg_loc("CombatNextUp","{0:NAME} fights next."), GetNextCombatant() ) );
	}
}

void ExCombat::AdvanceResting()
{
	assert( m_NextCombatantForRound >= m_CurrentRoundCombatants.Len() || IsCombatComplete() ); // The round should be completed before advancing resting.
	if( IsCombatComplete() )
	{
		return;
	}

	// Advance the speed of every fighter until at least one fighter is ready to go.
	eg_bool bAnyFighterReady = false;
	while( !bAnyFighterReady )
	{
		for( ExCombatTeam& Team : m_Teams )
		{
			for( ExCombatant& Combatant : Team.m_TeamMembers )
			{
				if( Combatant.IsInActiveCombat() )
				{
					Combatant.SetAttackType( ex_attack_t::MELEE ); // SPD should be calculated based on carrying melee weapons.
					Combatant.AdvanceCombatRoundTime();
					Combatant.SetAttackType( ex_attack_t::NONE );
					bAnyFighterReady = bAnyFighterReady || Combatant.IsReadyForCombatTurn();
				}
			}
		}
	}

	// Put any fighters that are ready in the current round:
	m_CurrentRoundCombatants.Clear( false );
	m_NextCombatantForRound = 0;
	for( ExCombatTeam& Team : m_Teams )
	{
		for( ExCombatant& Combatant : Team.m_TeamMembers )
		{
			if( Combatant.IsInActiveCombat() && Combatant.IsReadyForCombatTurn() )
			{
				m_CurrentRoundCombatants.Append( &Combatant );
			}
		}
	}

	m_CurrentRoundCombatants.Sort( [this]( const ExCombatant* Left , const ExCombatant* Right ) -> eg_bool
	{
		return CombatRoundSortCompare( Left , Right );
	} );

	m_CombatRound++;

	assert( m_CurrentRoundCombatants.HasItems() ); // Something wrong with algorithm.

	SpeculateNextCombatant();
}

void ExCombat::HandleAnim( const exCombatAnimInfo& AnimInfo )
{
	for( IExCombatAnimHandler* Handler : m_AnimHandlers )
	{
		if( Handler )
		{
			Handler->PlayAnimation( AnimInfo );
		}
	}
}

void ExCombat::HandleXPForDownedCombatant( const ExCombatant* Combatant )
{
	if( Combatant && ExBeastiary::Get().Contains( Combatant->GetBeastiaryId() ) )
	{
		ExCombatTeam& Team = m_Teams[Combatant->GetTeamIndex()];
		const exBeastInfo& BeastInfo = ExBeastiary::Get().FindInfo( Combatant->GetBeastiaryId() );
		ex_xp_value XPReward = BeastInfo.GetXPReward( Combatant->GetAttrValue( ex_attr_t::LVL ) ); 
		eg_int GoldDropped = BeastInfo.GetGoldDropAmount( Combatant->GetAttrValue( ex_attr_t::LVL ) , m_Rng );
		Team.m_XPDropped += XPReward;
		Team.m_GoldDropped += GoldDropped;
		if( BeastInfo.bIsBossCreature )
		{
			Team.m_bKilledBossCreature = true;
		}
	}
}

eg_int ExCombat::HandleBurgleOfCombatant( ExCombatant* Combatant )
{
	if( Combatant && ExBeastiary::Get().Contains( Combatant->GetBeastiaryId() ) )
	{
		ExCombatTeam& Team = m_Teams[Combatant->GetTeamIndex()];
		const exBeastInfo& BeastInfo = ExBeastiary::Get().FindInfo( Combatant->GetBeastiaryId() );
		eg_int GoldDropped = EGMath_ceil( BeastInfo.GetGoldDropAmount( Combatant->GetAttrValue( ex_attr_t::LVL ) , m_Rng ) * ExGlobalData::Get().GetCombatThiefBurglePercent()*.01f );
		const eg_int TimesBurgled = Combatant->ApplyBurgling();
		GoldDropped = EGMath_ceil(GoldDropped/EGMath_pow(2.f,EG_To<eg_real>(TimesBurgled)-1.f));
		Team.m_GoldDropped += GoldDropped;
		return GoldDropped;
	}
	return 0;
}

void ExCombat::ProcessCombatResults()
{
	auto DidCombatantEarnXP = []( ExCombatant* Combatant ) -> eg_bool
	{
		return Combatant && !Combatant->IsDead() && Combatant->GetCombatState() != ex_combat_s::ESCAPED;
	};
		
	for( ExCombatTeam& Team : m_Teams )
	{
		eg_uint TeamIndex = &Team == &m_Teams[0] ? 0 : 1;
		eg_uint OtherTeamIndex = &Team == &m_Teams[0] ? 1 : 0;

		// For the player team only move them out of combat so
		// the portraits move to the back row. We don't adjust
		// monsters so that they stay on screen when the party loses.
		if( TeamIndex == 0 )
		{
			Team.SetCompletedCombatRanks();
		}

		Team.m_Results.Result = ex_combat_result::Defeat;
		if( DidTeamWin( TeamIndex ) )
		{
			Team.m_Results.Result = ex_combat_result::Victory;
		}

		Team.m_Results.XPPerSurvivor = 0;
		Team.m_Results.GoldDropped = m_Teams[OtherTeamIndex].m_GoldDropped;

		if( m_Teams[OtherTeamIndex].m_bKilledBossCreature )
		{
			Team.m_Results.bKilledBossCreature = true;
		}

		eg_size_t SurvivingFighters = 0;
		for( eg_uint i = 0; i < Team.GetNumCombatants(); i++ )
		{
			ExCombatant* PartyMember = Team.GetCombatantByIndex( i );
			if( PartyMember )
			{
				if( DidCombatantEarnXP( PartyMember ) )
				{
					SurvivingFighters++;
				}

				// If the combat result was defeat but there is any escaped party
				// member then the combat result is Escaped, but any unconscious
				// party members die.

				if( Team.m_Results.Result == ex_combat_result::Defeat || Team.m_Results.Result == ex_combat_result::Fled )
				{
					if( PartyMember->GetCombatState() == ex_combat_s::ESCAPED )
					{
						Team.m_Results.Result = ex_combat_result::Fled;
					}
					else
					{
						PartyMember->SetDead( true );
					}
				}
			}
		}

		if( SurvivingFighters > 0 )
		{
			Team.m_Results.XPPerSurvivor = EG_Max<ex_xp_value>( 1 , m_Teams[OtherTeamIndex].m_XPDropped / SurvivingFighters );

			for( eg_uint i = 0; i < Team.GetNumCombatants(); i++ )
			{
				ExCombatant* PartyMember = Team.GetCombatantByIndex( i );
				if( PartyMember )
				{
					if( DidCombatantEarnXP( PartyMember ) )
					{
						PartyMember->AwardXP( Team.m_Results.XPPerSurvivor );
					}
				}
			}
		}
	}
}

void ExCombat::SpeculateNextCombatant()
{
	m_SpeculationFirstCombatantNextRound = nullptr;

	for( ExCombatTeam& Team : m_Teams )
	{
		for( ExCombatant& Combatant : Team.m_TeamMembers )
		{
			if( Combatant.IsInActiveCombat() )
			{
				Combatant.ResetSpeculationCombatRoundTime();
				if( Combatant.IsReadyForSpeculationCombatTurn() )
				{
					Combatant.ApplySpeculataionCombatTurnCompleted();
				}
			}
		}
	}

	// Advance the speed of every fighter until at least one fighter is ready to go.
	eg_bool bAnyFighterReady = false;
	while( !bAnyFighterReady )
	{
		for( ExCombatTeam& Team : m_Teams )
		{
			for( ExCombatant& Combatant : Team.m_TeamMembers )
			{
				if( Combatant.IsInActiveCombat() )
				{
					Combatant.SetAttackType( ex_attack_t::MELEE ); // SPD should be calculated based on carrying melee weapons.
					Combatant.AdvanceSpeculationCombatRoundTime();
					Combatant.SetAttackType( ex_attack_t::NONE );
					bAnyFighterReady = bAnyFighterReady || Combatant.IsReadyForSpeculationCombatTurn();
				}
			}
		}
	}

	// Put any fighters that are ready in the current round:
	EGArray<ExCombatant*> SpeculationRound;
	for( ExCombatTeam& Team : m_Teams )
	{
		for( ExCombatant& Combatant : Team.m_TeamMembers )
		{
			if( Combatant.IsInActiveCombat() && Combatant.IsReadyForSpeculationCombatTurn() )
			{
				SpeculationRound.Append( &Combatant );
			}
		}
	}

	SpeculationRound.Sort( [this]( const ExCombatant* Left , const ExCombatant* Right ) -> eg_bool
	{
		return CombatRoundSortCompare( Left , Right );
	} );

	m_SpeculationFirstCombatantNextRound = SpeculationRound.IsValidIndex( 0 ) ? SpeculationRound[0] : nullptr;
}

eg_bool ExCombat::IsCombatComplete() const
{
	eg_bool bComplete = false;
	for( eg_size_t i = 0; i < countof( m_Teams ); i++ )
	{
		bComplete = bComplete || m_Teams[i].IsDown();
	}

	return bComplete;
}

eg_bool ExCombat::DidTeamWin( eg_uint TeamIndex )
{
	eg_bool bWon = true;
	for( eg_size_t i=0; i<countof(m_Teams); i++ )
	{
		if( i == TeamIndex )
		{
			bWon = bWon && !m_Teams[i].IsDown();
		}
		else
		{
			bWon = bWon && m_Teams[i].IsDown();
		}
	}

	return bWon;
}

void ExCombat::GetCombatLog( eg_loc_char* Out, eg_size_t OutSize ) const
{
	EGString_Copy( Out , m_CombatLog , OutSize );
}

void ExCombat::SendCombatResultsToServer( ExGame* Game )
{
	if( Game && Game->IsInCombat() )
	{
		// Replicate the party state to the server.
		for( eg_uint i=0; i<m_Teams[0].GetNumCombatants(); i++ )
		{
			const ExCombatant* PartyMember = m_Teams[0].GetCombatantByIndex( i );
			if( PartyMember )
			{
				PartyMember->UpdateFighterOnServer( Game );
			}
		}

		if( m_Teams[0].m_Results.Result == ex_combat_result::Victory && m_Teams[0].m_Results.GoldDropped > 0 )
		{
			Game->SDK_RunServerEvent( eg_crc("DropGold") , m_Teams[0].m_Results.GoldDropped );
		}
		Game->SDK_RunServerEvent( eg_crc("OnCombatResults") , static_cast<eg_int64>(m_Teams[0].m_Results.Result) );
	}
}

ExSkills* ExCombat::GetSkillsModule()
{
	return m_Skills.GetObject();
}

const ExGame* ExCombat::GetGame() const
{
	return m_Game.GetObject();
}

const ExFighter* ExCombat::GetPartyMemberByIndex( eg_int TeamIndex , eg_int PartyIndex ) const
{
	if( 0 <= TeamIndex && TeamIndex < countof(m_Teams) )
	{
		for( const ExCombatant& Combatant : m_Teams[TeamIndex].m_TeamMembers )
		{
			if( Combatant.GetCombatState() != ex_combat_s::ESCAPED && Combatant.GetServerPartyIndex() == PartyIndex )
			{
				return &Combatant.GetFighter();
			}
		}
	}

	return nullptr;
}

void ExCombat::GetCombatantList( eg_uint TeamIndex, eg_flags What, ExCombatantList& Out )
{
	Out.Clear();

	if( 0 <= TeamIndex && TeamIndex < countof(m_Teams) )
	{
		for( ExCombatant& Combatant : m_Teams[TeamIndex].m_TeamMembers )
		{
			if( What.IsSet( F_ALL ) )
			{
				Out.Append( &Combatant );
				continue;
			}

			switch( Combatant.GetCombatState() )
			{
			case ex_combat_s::OUT_OF_COMBAT:
			{
				if( What.IsSet( F_OUTOFCOMBAT ) )
				{
					Out.Append( &Combatant );
				}
			} break;
			case ex_combat_s::FIRST_ROW:
			case ex_combat_s::BACK_ROW:
			{
				if( What.IsSet( F_ACTIVE ) )
				{
					Out.Append( &Combatant );
				}
			}break;
			case ex_combat_s::DOWN:
			{
				if( What.IsSet( F_DOWN ) )
				{
					Out.Append( &Combatant );
				}
			} break;
			case ex_combat_s::ESCAPED:
			{
				if( What.IsSet( F_ESCAPED ) )
				{
					Out.Append( &Combatant );
				}
			} break;
			}
		}
	}
	else
	{
		assert( false ); // Nothing to get.
	}

}

void ExCombat::GetCombatOrderList( ExCombatantList& Out )
{
	Out.Clear();
	for( ExCombatant* Combatant : m_CurrentRoundCombatants )
	{
		Out.Append( Combatant );
	}
}

eg_bool ExCombat::IsStateInActiveCombat( ex_combat_s State )
{
	return !( State == ex_combat_s::OUT_OF_COMBAT || State == ex_combat_s::DOWN || State == ex_combat_s::ESCAPED );
}

eg_bool ExCombat::IsStateEligibleForActiveCombat( ex_combat_s State )
{
	return !( State == ex_combat_s::DOWN || State == ex_combat_s::ESCAPED );
}

eg_bool ExCombat::CombatRoundSortCompare( const ExCombatant* Left , const ExCombatant* Right )
{
	// Should never have null...
	if( nullptr == Left || nullptr == Right )
	{
		assert( false );
		return false;
	}

	// The person that has advanced in the most speed goes first.
	if( Left->GetCombatRoundTime() != Right->GetCombatRoundTime() )
	{
		return Left->GetCombatRoundTime() > Right->GetCombatRoundTime();
	}

	if( Left->GetAttrValue( ex_attr_t::SPD ) != Right->GetAttrValue( ex_attr_t::SPD ) )
	{
		return Left->GetAttrValue( ex_attr_t::SPD ) > Right->GetAttrValue( ex_attr_t::SPD );
	}

	// The first team goes first so the player will have an advantage with creatures with the same speed.
	if( Left->GetTeamIndex() != Right->GetTeamIndex() )
	{
		return Left->GetTeamIndex() < Right->GetTeamIndex();
	}

	// Finally go by order:
	return Left->GetArrayIndex() < Right->GetArrayIndex();
}

void ExCombat::ChangeFighterState( ExCombatant* Combatant, ex_combat_s NewState, eg_bool bPreserveRest )
{
	unused( bPreserveRest  );
	
	ex_combat_s OldState = Combatant->GetCombatState();

	Combatant->SetCombatState( NewState );

	if( IsStateInActiveCombat( OldState ) && IsStateInActiveCombat( NewState ) )
	{
		// States are basically the same, nothing special to do.
	}
	else
	{
		if( IsStateInActiveCombat( OldState ) )
		{
			// If the fighter is getting moved out of combat and was queued for the current round, remove them.
			const eg_size_t CurrentRoundIndex = m_CurrentRoundCombatants.GetIndexOf( Combatant );
			if( m_CurrentRoundCombatants.IsValidIndex( CurrentRoundIndex ) )
			{
				m_CurrentRoundCombatants.DeleteByIndex( CurrentRoundIndex );
				// If the next fighter was after the current one, we need to move the index so no one gets skipped.
				if( m_NextCombatantForRound > CurrentRoundIndex )
				{
					m_NextCombatantForRound--;
				}
			}
		}

		if( IsStateInActiveCombat( NewState ) )
		{
			// ???: Should we do anything special when entering combat... Basically without doing
			// anything the fighter will pick up where they left off, but if entering combat for
			// the first time maybe they should get a boost? (We don't really have a mechanism for
			// knowing if it's the first time entering combat at this time, that would have to be
			// added.)
			// if( !bPreserveRest )
			// {
			// 	Combatant->ResetCombatRoundTime();
			// 	// ???: Fighter is entering combat, give them a speed boost since they've been waiting...
			// 	Combatant->AdvanceCombatRoundTime();
			// }
			// 
			// // ???: Wait till next round? Or put them in current round...
			// // m_CurrentRoundCombatants.Append( Combatant );
		}
	}
}

void ExCombat::CreateSamplePlayerTeam( const ExGame* Game )
{
	if( Game && Game->GetPlayerEnt() != nullptr )
	{
		for( eg_uint i=0; i<ExRoster::PARTY_SIZE; i++ )
		{
			const ExFighter* RosterChar = Game->GetPartyMemberByIndex( i );
			if( RosterChar )
			{
				// Level the roster character to the level of the monster.
				ExFighter AdjustedFighter = *RosterChar;
				AdjustedFighter.SetAttrValue( ex_attr_t::LVL , EG_Max<eg_int>( 1 , s_SampleCombatPlayerTeamLevel ) );
				AdjustedFighter.RestoreAllHP();
				AdjustedFighter.RestoreAllMP();
				m_Teams[0].PreInit_AddFighter( AdjustedFighter , i , CT_Clear );
			}
		}
	}
	else
	{
		ex_class_t FighterClass = ex_class_t::Unknown;
		eg_cpstr16 FighterName = L"";

		switch( s_SampleCombatTeamClass )
		{
		default:
		case ex_class_t::Warrior:
			FighterClass = ex_class_t::Warrior;
			FighterName = L"Test Warrior";
			break;
		case ex_class_t::Thief:
			FighterClass = ex_class_t::Thief;
			FighterName = L"Test Thief";
			break;
		case ex_class_t::Mage:
			FighterClass = ex_class_t::Mage;
			FighterName = L"Test Mage";
			break;
		case ex_class_t::Cleric:
			FighterClass = ex_class_t::Cleric;
			FighterName = L"Test Cleric";
			break;
		}

		ExFighter Fighter( CT_Clear );
		Fighter.InitAs( FighterClass , false );
		Fighter.InitDefaultItemsAndSkills();
		Fighter.SetAttrValue( ex_attr_t::LVL , EG_Max<eg_int>( 1 , s_SampleCombatPlayerTeamLevel ) );
		EGString_Copy( Fighter.LocName , FighterName , countof(Fighter.LocName) );
		Fighter.PortraitId = m_Rng.GetRandomRangeI(0,1) == 0 ? eg_crc("Male01") : eg_crc("Female01");
		Fighter.RestoreAllHP();
		Fighter.RestoreAllMP();
		Fighter.SetCreated( true );
		m_Teams[0].PreInit_AddFighter( Fighter, 0 , CT_Clear );
	}
}

void ExCombat::CreateSampleOpposingTeam()
{
	if( s_SampleCombatOpponentList.HasItems() )
	{
		for( const exSampleCombatMonsterInfo& MonsterInfo : s_SampleCombatOpponentList )
		{
			const exBeastInfo& BeastInfo = ExBeastiary::Get().FindInfo( MonsterInfo.MonsterId );

			if( MonsterInfo.MonsterLevel > 0 )
			{
				ExFighter Fighter( CT_Clear );
				Fighter.InitMonster( BeastInfo , MonsterInfo.MonsterLevel );
				m_Teams[1].PreInit_AddFighter( Fighter , 0 , BeastInfo.Id );
			}
		}
	}


	if( m_Teams[1].m_TeamMembers.IsEmpty() )
	{
		CreateOpposingTeam( eg_crc("DefaultEncounter") );
	}
}

void ExCombat::CreatePlayerTeam( const ExGame* Game )
{
	for( eg_uint i=0; i<ExRoster::PARTY_SIZE; i++ )
	{
		const ExFighter* RosterChar = Game->GetPartyMemberByIndex( i );
		if( RosterChar )
		{
			m_Teams[0].PreInit_AddFighter( *RosterChar , i , CT_Clear );
		}
	}
}

void ExCombat::CreateOpposingTeam( const eg_string_crc& CombatEncounterId )
{
	const exCombatEncounterInfo* Encounter = ExCombatEncounter::Get().FindCombatEncounter( CombatEncounterId );

	if( Encounter )
	{
		EGArray<exCombatEncounterMonster> MonsterList;
		Encounter->CreateMonsterList( m_Rng , MonsterList );

		for( const exCombatEncounterMonster& MonsterInfo : MonsterList )
		{
			if( MonsterInfo.BeastInfo && MonsterInfo.Level > 0 )
			{
				ExFighter Fighter( CT_Clear );
				Fighter.InitMonster( *MonsterInfo.BeastInfo , MonsterInfo.Level );
				m_Teams[1].PreInit_AddFighter( Fighter , 0 , MonsterInfo.BeastInfo->Id );
			}
		}
	}
	
	if( m_Teams[1].m_TeamMembers.IsEmpty() )
	{
		EGLogf( eg_log_t::Error , "Could not find specified combat encounter." );
		assert( false );
		CreateRandomOppsingTeam( 6  , 1 , 1 );
	}
}

void ExCombat::CreateRandomOppsingTeam( eg_uint NumMonsters, eg_int MinLevel, eg_int MaxLevel )
{
	// Create some random monsters:
	static const eg_uint NUM_MONSTERS = NumMonsters;
	for( eg_uint i = 0; i < NUM_MONSTERS; i++ )
	{
		const exBeastInfo& BeastInfo = ExBeastiary::Get().GetRandomBeast( m_Rng , exRandomBeastParms() );

		ExFighter Fighter( CT_Clear );
		Fighter.InitMonster( BeastInfo , m_Rng.GetRandomRangeI( MinLevel , MaxLevel ) );
		m_Teams[1].PreInit_AddFighter( Fighter , 0 , BeastInfo.Id );
	}
}

void ExCombat::ClearCombatLog()
{
	m_CombatLog[0] = '\0';
}

void ExCombat::AppendCombatLog( const eg_loc_text& Text )
{
	if( m_CombatLog[0] != '\0' )
	{
		EGString_StrCat( m_CombatLog , countof(m_CombatLog) , L" " );
	}

	EGString_StrCatCount( m_CombatLog , countof(m_CombatLog) , Text.GetString() , Text.GetLen() );
}
