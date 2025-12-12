//
// ExCombat - The Combat System for EXPLOR
// (c) 2016 Beem Media
//
#pragma once

#include "ExCombatTypes.h"
#include "ExCombatTeam.h"
#include "ExCombatant.h"
#include "ExCore.h"
#include "EGList.h"
#include "ExGameTypes.h"
#include "EGRandom.h"

class ExSkills;
class ExGame;

class ExCombat : public EGObject
{
	EG_CLASS_BODY( ExCombat , EGObject )

friend class ExCombatTeam;

public:
	
	static const eg_size_t MAX_ACTIVE_COMBAT_FIGHTERS = 20;

	struct exTurnInput
	{
		ex_combat_turn_opt  TurnOpt;
		ex_combat_actions   Action;
		eg_uint             ActionIndex; // Spell index, etc...
		ExCombatant*        Target; // Target for single target attack, or the first target for multi-target attack (targeting happens in team order ignoring down combatants).

		exTurnInput( eg_ctor_t Ct )
		: TurnOpt( ex_combat_turn_opt::AUTO )
		, Action( ex_combat_actions::AUTO )
		, ActionIndex( 0 )
		, Target( nullptr )
		{
			unused( Ct );
			assert( Ct == CT_Clear || Ct == CT_Default );
		}
	};
	
	struct exSampleCombatMonsterInfo
	{
		eg_string_crc MonsterId = CT_Clear;
		eg_int        MonsterLevel = 1;
	};

	static eg_int                             s_SampleCombatPlayerTeamLevel;
	static ex_class_t                         s_SampleCombatTeamClass;
	static EGArray<exSampleCombatMonsterInfo> s_SampleCombatOpponentList;

public:

	ExCombat();

	void AddAnimHandler( IExCombatAnimHandler* Handler );
	void BeginCombat( const ExGame* Game );
	void ApplyAmbush();
	ExCombatant* GetNextCombatant();
	ExCombatant* GetFutureCombatant();
	void ProcessTurn( const exTurnInput& TurnInput );
	eg_bool IsCombatComplete() const;
	eg_bool DidTeamWin( eg_uint TeamIndex );
	eg_bool IsAmbush() const { return m_bIsAmbush; }
	void GetCombatLog( eg_loc_char* Out , eg_size_t OutSize ) const;
	exCombatTeamResults GetCombatResultsForTeam( eg_uint Index ) const { return m_Teams[Index].m_Results; }
	void SendCombatResultsToServer( ExGame* Game );
	ExSkills* GetSkillsModule();
	const ExGame* GetGame() const;
	const ExFighter* GetPartyMemberByIndex( eg_int TeamIndex , eg_int PartyIndex ) const;

	EG_DECLARE_FLAG( F_ACTIVE , 0 );
	EG_DECLARE_FLAG( F_ESCAPED , 1 );
	EG_DECLARE_FLAG( F_OUTOFCOMBAT , 2 );
	EG_DECLARE_FLAG( F_DOWN , 3 );
	EG_DECLARE_FLAG( F_ALL , 4 );
	void GetCombatantList( eg_uint TeamIndex , eg_flags What , ExCombatantList& Out );
	void GetCombatOrderList( ExCombatantList& Out );
	const ExCombatTeam& GetTeamByIndex( eg_int TeamIndex ) const { assert( EG_IsBetween<eg_int>( TeamIndex , 0 , 1 ) ); return m_Teams[TeamIndex]; }

private:

	ExCombatant* GetCombatant( eg_uint TeamIndex , eg_uint ArrayIndex );
	void ProcessTurnFor( ExCombatant* Combatant, const exTurnInput& TurnInput );
	void AdvanceRanks();
	void CullDeadFighters();
	void DoCast( ExCombatant* Attacker , const exTurnInput& TurnInput );
	void DoCastInternal( ExCombatant* Attacker , ExCombatant* ChosenTarget , const exSpellInfo* Spell );
	void DoMeleeCombat( ExCombatant* Attacker , ExCombatant* Defender );
	void DoRangedCombat( ExCombatant* Attacker , ExCombatant* Defender );
	void DoBlock( ExCombatant* Attacker );
	void DoRun( ExCombatant* Attacker );
	void DoAdvance( ExCombatant* Attacker );
	void DoBackAway( ExCombatant* Attacker );
	void BeginTurn( const exTurnInput& TurnInput );
	void AdvanceResting();
	void HandleAnim( const exCombatAnimInfo& AnimInfo );
	void HandleXPForDownedCombatant( const ExCombatant* Combatant );
	eg_int HandleBurgleOfCombatant( ExCombatant* Combatant );
	void ProcessCombatResults();
	void SpeculateNextCombatant();


//
// State:
//
private:

	EGWeakPtr<ExGame>                     m_Game;
	EGWeakPtr<ExSkills>                   m_Skills;
	ExCombatTeam                          m_Teams[2];
	EGArray<ExCombatant*>                 m_CurrentRoundCombatants;
	eg_int                                m_NextCombatantForRound;
	eg_int                                m_CombatRound;
	ExCombatant*                          m_SpeculationFirstCombatantNextRound = nullptr;
	EGRandom                              m_Rng;
	ex_combat_log_verbosity               m_Verbosity = ex_combat_log_verbosity::Minimal;
	eg_loc_char                           m_CombatLog[1024];
	EGFixedArray<IExCombatAnimHandler*,2> m_AnimHandlers;
	eg_bool                               m_bIsAmbush = false;

//
// Private Implementation:
//
private:

	static eg_bool IsStateInActiveCombat( ex_combat_s State );
	static eg_bool IsStateEligibleForActiveCombat( ex_combat_s State );
	static eg_bool CombatRoundSortCompare( const ExCombatant* Left , const ExCombatant* Right );

	void ChangeFighterState( ExCombatant* Combatant , ex_combat_s NewState , eg_bool bPreserveRest );

	void CreateSamplePlayerTeam( const ExGame* Game );
	void CreateSampleOpposingTeam();
	void CreatePlayerTeam( const ExGame* Game );
	void CreateOpposingTeam( const eg_string_crc& CombatEncounterId );
	void CreateRandomOppsingTeam( eg_uint NumMonsters , eg_int MinLevel , eg_int MaxLevel );

	void ClearCombatLog();
	void AppendCombatLog( const eg_loc_text& Text );
};