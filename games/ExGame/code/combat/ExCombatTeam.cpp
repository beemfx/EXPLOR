// (c) 2017 Beem Media

#include "ExCombatTeam.h"
#include "ExCombat.h"
#include "ExCombatAI.h"
#include "ExRoster.h"
#include "ExGlobalData.h"

void ExCombatTeam::PreInit( ExCombat* Combat )
{
	m_Combat = Combat;
}

void ExCombatTeam::PreInit_AddFighter( const ExFighter& Fighter , eg_uint PartyIndex , eg_string_crc BeastiaryId )
{
	ExCombatant NewCombatant;
	NewCombatant.InitCombatant( m_Combat , Fighter , static_cast<eg_uint>(m_TeamMembers.Len()) , BeastiaryId , PartyIndex );
	m_TeamMembers.Append( NewCombatant );
}

void ExCombatTeam::Init( eg_uint TeamIndex , eg_int FrontLineSize , eg_int FrontLineCount )
{
	// We are presuming the monster team is index 2 right now
	switch( TeamIndex )
	{
	case 0:
		// Ranks are all even...
		m_FrontRowWidth = FrontLineSize;
		m_BackRowWidth = ExRoster::PARTY_SIZE - FrontLineSize;
		break;
	case 1:
		// All active combat monsters are in the front row.
		m_FrontRowWidth = FrontLineSize;
		m_BackRowWidth = 0;
		break;
	}

	for( eg_uint i = 0; i < m_TeamMembers.Len(); i++ )
	{
		m_TeamMembers[i].InitCombatInfo( TeamIndex , i );
	}

	AdvanceRanks();

	eg_uint FrontRowFound = 0;
	for( eg_uint i = 0; i < m_TeamMembers.Len(); i++ )
	{
		if( FrontRowFound < EG_To<eg_uint>(FrontLineCount) )
		{
			if( m_TeamMembers[i].CanFight() )
			{
				FrontRowFound++;
				m_Combat->ChangeFighterState( &m_TeamMembers[i] , ex_combat_s::FIRST_ROW , true );
			}
		}
	}
}

ExCombatant* ExCombatTeam::GetCombatantByIndex( eg_uint Index )
{
	if( m_TeamMembers.IsValidIndex( Index ) )
	{
		return &m_TeamMembers[Index];
	}

	return nullptr;
}

void ExCombatTeam::AdvanceRanks()
{
	// Get counts:
	eg_size_t FrontRowCount = 0;
	eg_size_t BackRowCount = 0;

	auto ChangeCountByType = [&FrontRowCount,&BackRowCount]( ex_combat_s State, eg_int Delta ) -> void
	{
		switch( State )
		{
		case ex_combat_s::OUT_OF_COMBAT: break;
		case ex_combat_s::FIRST_ROW: FrontRowCount += Delta; break;
		case ex_combat_s::BACK_ROW: BackRowCount += Delta; break;
		case ex_combat_s::DOWN: break;
		case ex_combat_s::ESCAPED: break;
		}
	};

	auto AreRanksFull = [FrontRowCount,BackRowCount,this]() -> eg_bool
	{
		return FrontRowCount == m_FrontRowWidth && BackRowCount == m_BackRowWidth;
	};

	// Get the counts:
	for( eg_size_t i = 0; i < m_TeamMembers.Len(); i++ )
	{
		ChangeCountByType( m_TeamMembers[i].GetCombatState(), 1 );
	}

	// Advance ranks until all are full, or we run out of fighters:
	for( eg_size_t i = 0; i < m_TeamMembers.Len() && !AreRanksFull(); i++ )
	{
		ExCombatant& Combatant = m_TeamMembers[i];
		if( ExCombat::IsStateEligibleForActiveCombat( Combatant.GetCombatState() ) )
		{
			if( FrontRowCount < m_FrontRowWidth )
			{
				if( Combatant.GetCombatState() > ex_combat_s::FIRST_ROW || Combatant.GetCombatState() == ex_combat_s::OUT_OF_COMBAT )
				{
					ChangeCountByType( Combatant.GetCombatState(), -1 );
					m_Combat->ChangeFighterState( &Combatant, ex_combat_s::FIRST_ROW, true );
					ChangeCountByType( Combatant.GetCombatState(), 1 );
				}
			}
			else if( BackRowCount < m_BackRowWidth )
			{
				if( Combatant.GetCombatState() > ex_combat_s::BACK_ROW || Combatant.GetCombatState() == ex_combat_s::OUT_OF_COMBAT)
				{
					ChangeCountByType( Combatant.GetCombatState(), -1 );
					m_Combat->ChangeFighterState( &Combatant, ex_combat_s::BACK_ROW, true );
					ChangeCountByType( Combatant.GetCombatState(), 1 );
				}
			}
		}
	}

	m_bTeamDown = 0 == FrontRowCount && 0 == BackRowCount;
}

void ExCombatTeam::SetCompletedCombatRanks()
{
	for( eg_size_t i = 0; i < m_TeamMembers.Len(); i++ )
	{
		ExCombatant& Combatant = m_TeamMembers[i];
		if( Combatant.GetCombatState() <= ex_combat_s::BACK_ROW )
		{
			m_Combat->ChangeFighterState( &Combatant , ex_combat_s::OUT_OF_COMBAT , true );
		}
	}
}

ExCombatant* ExCombatTeam::ChooseCombatantToAttack( eg_bool bForceInOrder , ex_target_strategy Strategy , eg_bool bRanged )
{
	return ExCombatAI_ChooseCombatantToAttack( m_TeamMembers , m_Combat->m_Rng , bForceInOrder , Strategy , bRanged );
}

eg_bool ExCombatTeam::IsFrontRowOverFull() const
{
	eg_uint NumFightersOnFrontRow = 0;

	for( const ExCombatant& Combatant : m_TeamMembers )
	{
		if( Combatant.GetCombatState() == ex_combat_s::FIRST_ROW )
		{
			NumFightersOnFrontRow++;
		}
	}

	return NumFightersOnFrontRow > m_FrontRowWidth;
}

void ExCombatTeam::ApplySpellFortifyRanks()
{
	eg_ivec2 Range = ExGlobalData::Get().GetCombatSpellFortifyRanksRange();
	m_FrontRowWidth = EG_Clamp<eg_uint>( m_FrontRowWidth - 1 , Range.x , Range.y );
	for( ExCombatant& Combatant : m_TeamMembers )
	{
		if( Combatant.GetCombatState() == ex_combat_s::FIRST_ROW )
		{
			m_Combat->ChangeFighterState( &Combatant , ex_combat_s::BACK_ROW , true );
		}
	}
}

void ExCombatTeam::ApplySpellExpandRanks()
{
	m_FrontRowWidth = EG_Clamp<eg_uint>( m_FrontRowWidth + 1 , 1 , 6 );
	for( ExCombatant& Combatant : m_TeamMembers )
	{
		if( Combatant.GetCombatState() == ex_combat_s::FIRST_ROW )
		{
			m_Combat->ChangeFighterState( &Combatant , ex_combat_s::BACK_ROW , true );
		}
	}
}
