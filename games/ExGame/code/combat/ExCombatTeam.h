// (c) 2017 Beem Media

#pragma once

#include "ExCombatTypes.h"
#include "ExCombatant.h"

class ExFighter;
class ExCombat;
class ExCombatant;

struct exCombatTeamResults
{
	ex_combat_result Result = ex_combat_result::Defeat;
	ex_xp_value      XPPerSurvivor = 0;
	eg_int           GoldDropped = 0;
	eg_bool          bKilledBossCreature = false;
};

class ExCombatTeam
{
public:

	typedef EGArray<ExCombatant>  ExCombatantArrary;

public:

	ExCombatTeam()
	: m_Combat( nullptr )
	, m_FrontRowWidth( 0 )
	, m_BackRowWidth( 0 )
	, m_bTeamDown( false )
	, m_bKilledBossCreature( false )
	, m_XPDropped( 0 )
	, m_GoldDropped( 0 )
	{

	}

	void PreInit( ExCombat* Combat );
	void PreInit_AddFighter( const ExFighter& Fighter , eg_uint ServerIndex , eg_string_crc BeastiaryId );
	void Init( eg_uint TeamIndex , eg_int FrontLineSize , eg_int FrontLineCount );
	eg_size_t GetNumCombatants()const{ return m_TeamMembers.Len(); }
	ExCombatant* GetCombatantByIndex( eg_uint Index );
	void AdvanceRanks();
	void SetCompletedCombatRanks();
	ExCombatant* ChooseCombatantToAttack( eg_bool bForceInOrder , ex_target_strategy Strategy , eg_bool bRanged );
	eg_bool IsDown() const { return m_bTeamDown; }
	eg_bool IsFrontRowOverFull() const;
	eg_uint GetFrontRowWidth() const { return m_FrontRowWidth; }
	void ApplySpellFortifyRanks();
	void ApplySpellExpandRanks();

public:

	ExCombat*           m_Combat;
	eg_uint             m_FrontRowWidth;
	eg_uint             m_BackRowWidth;
	ExCombatantArrary   m_TeamMembers;
	ex_xp_value         m_XPDropped;
	eg_int              m_GoldDropped;
	eg_bool             m_bTeamDown : 1;
	eg_bool             m_bKilledBossCreature : 1;
	exCombatTeamResults m_Results;
};
