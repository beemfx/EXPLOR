// (c) 2017 Beem Media

#pragma once

#include "ExCombatTypes.h"
#include "ExFighter.h"
#include "EGTextFormat.h"
#include "ExSpellInfo.h"
#include "EGWeakPtr.h"

class ExCombat;
class EGRandom;

class ExCombatant : public IEGCustomFormatHandler
{
private:

	ExFighter           m_Fighter;
	ExCombat*           m_CombatOwner = nullptr;
	eg_uint             m_TeamIndex;
	eg_uint             m_ArrayIndex;
	eg_string_crc       m_BeastiaryId;
	ex_combat_s         m_CombatState;
	eg_uint             m_ServerPartyIndex;
	ex_attr_value       m_CombatRoundTime;
	ex_attr_value       m_SpeculationCombatRoundTime;
	ExSpellList         m_Spells;
	exAttrSet           m_Boosts;
	eg_uint             m_TimesBurgled = 0;
	eg_bool             m_bIsBlocking:1;

public:

	ExCombatant();

	void InitCombatant( ExCombat* CombatOwner , const ExFighter& FighterIn , eg_uint ArrayIndexIn , eg_string_crc BeastiaryIdIn , eg_uint ServerPartyIndexIn );
	void InitCombatInfo( eg_uint TeamIndexIn , eg_uint ArrayIndexIn );
	ex_combat_s GetCombatState() const { return m_CombatState; }
	eg_bool CanBeKilled() const { return m_CombatState == ex_combat_s::DOWN && !IsDead(); }
	eg_bool IsInFrontLines() const { return m_CombatState == ex_combat_s::FIRST_ROW; }
	eg_bool IsInFrontLinesOrCanBeKilled() const { return IsInFrontLines() || CanBeKilled(); } 
	eg_bool IsInShootingRange() const { return IsInFrontLines() || m_CombatState == ex_combat_s::BACK_ROW; }
	eg_bool IsInShootingRangeOrCanBeKilled() const { return IsInShootingRange() || CanBeKilled(); }
	void SetCombatState( ex_combat_s NewState ){ m_CombatState = NewState; }
	eg_uint GetTeamIndex() const { return m_TeamIndex; }
	eg_uint GetArrayIndex() const { return m_ArrayIndex; }
	eg_uint GetServerPartyIndex() const { return m_ServerPartyIndex; }
	const ExSpellList& GetSpells() const { return m_Spells; }
	eg_bool IsInActiveCombat() const;
	eg_bool CanCast( const exSpellInfo* Spell ) const;
	eg_bool CanFight() const;
	void GetAvailableActions( ExCombatActionsArray& OutActions );
	eg_bool SpellCanReach( const exSpellInfo* Spell ) const;
	void OnBeginTurn();
	void SetAttackType( ex_attack_t AttackType );
	exDamage GetAttackDamage() const;
	exDamage GetFullDefense() const;
	exDamage GetRolledDefense( EGRandom& Rng ) const;
	ex_attr_value GetAttrValue( ex_attr_t Attr ) const;
	ex_attr_value GetMeleeAttrValue( ex_attr_t Attr ) const;
	eg_string_crc GetBeastiaryId() const { return m_BeastiaryId; }
	ex_attr_value GetCombatRoundTime() const { return m_CombatRoundTime; }
	void AdvanceCombatRoundTime() { m_CombatRoundTime += EG_Clamp( GetAttrValue( ex_attr_t::SPD ) , 1 , EX_COMBAT_MAX_SPEED ); }
	eg_bool IsReadyForCombatTurn() const { return m_CombatRoundTime >= EX_COMBAT_ROUND_DURATION; }
	void ApplyCombatTurnCompleted() { assert( m_CombatRoundTime >= EX_COMBAT_ROUND_DURATION ); m_CombatRoundTime -= EX_COMBAT_ROUND_DURATION; }
	void ResetCombatRoundTime() { m_CombatRoundTime = 0; }
	void ResetSpeculationCombatRoundTime() { m_SpeculationCombatRoundTime = m_CombatRoundTime; }
	void AdvanceSpeculationCombatRoundTime() { m_SpeculationCombatRoundTime += EG_Clamp( GetAttrValue( ex_attr_t::SPD ) , 1 , EX_COMBAT_MAX_SPEED ); }
	void ApplySpeculataionCombatTurnCompleted() { assert( m_SpeculationCombatRoundTime >= EX_COMBAT_ROUND_DURATION ); m_SpeculationCombatRoundTime -= EX_COMBAT_ROUND_DURATION; }
	eg_bool IsReadyForSpeculationCombatTurn() const { return m_SpeculationCombatRoundTime >= EX_COMBAT_ROUND_DURATION; }
	ex_attr_value GetHP() const { return m_Fighter.GetHP(); }
	ex_attr_value GetMP() const { return m_Fighter.GetMP(); }
	void UseMP( ex_attr_value Amount ){ m_Fighter.UseMana( Amount ); }
	void AdjustAggression( ex_attr_value DeltaValue );
	void ResetAgression();
	void AdjustBoostAttr( ex_attr_t Attr , ex_attr_value DeltaValue );
	void SetBoostAttr( ex_attr_t Attr , ex_attr_value NewValue );
	void OnAbandonedCombat();
	void ApplyRawHit( ex_attr_value Hit ){ m_Fighter.ApplyRawHit( Hit ); }
	void ApplyRawHeal( ex_attr_value Heal ){ m_Fighter.ApplyRawHeal( Heal ); }
	void ApplyManaRestoration( ex_attr_value Amount ) { m_Fighter.ApplyManaRestoration( Amount ); }
	void ApplyPostSpellBoosts();
	void SetIsBlocking( eg_bool bIsBlockingIn ){ m_bIsBlocking = bIsBlockingIn; }
	eg_bool IsDead() const { return m_Fighter.IsDead(); }
	void SetDead( eg_bool bIsDead ){ m_Fighter.SetDead( bIsDead ); }
	eg_bool IsUnconcious() const { return m_Fighter.IsUnconscious(); }
	void SetUnconcious( eg_bool bIsUnconcious ){ m_Fighter.SetUnconcious( bIsUnconcious ); if( bIsUnconcious ) { ResetAgression(); } }
	eg_bool IsCursed() const { return m_Fighter.IsCursed(); }
	void SetCursed( eg_bool bIsCursed ){ m_Fighter.SetCursed( bIsCursed ); }
	eg_bool IsPetrified() const { return m_Fighter.IsPetrified(); }
	void SetPetrified( eg_bool bIsPetrified ){ m_Fighter.SetPetrified( bIsPetrified ); }
	void AwardXP( ex_xp_value XpToAward ){ m_Fighter.AwardXP( XpToAward ); }
	void UpdateFighterOnServer( class ExGame* Game ) const;
	const ExFighter& GetFighter() const { return m_Fighter; }
	ex_target_strategy GetTargetStrategy() const;
	eg_bool HasRangedAttack() const;
	eg_int ApplyBurgling() { m_TimesBurgled++; return EG_Max<eg_int>( 1 , m_TimesBurgled ); }
	virtual void FormatText( eg_cpstr Flags , class EGTextParmFormatter* Formatter ) const override final;

private:
	
	ex_attr_value GetBoostAttrValue( ex_attr_t Attr ) const;
};