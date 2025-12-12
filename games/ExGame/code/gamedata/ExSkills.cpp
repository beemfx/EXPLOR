// (c) 2017 Beem Media

#include "ExSkills.h"
#include "ExSpellBook.h"
#include "ExFighter.h"
#include "EGFileData.h"
#include "ExCore.h"
#include "ExGame.h"
#include "ExMapInfos.h"

EG_CLASS_DECL( ExSkills )

eg_bool ExSkills::IsSkillUnlocked( eg_string_crc SpellId ) const
{
	return m_UnlockedSkills.Contains( SpellId );
}

void ExSkills::SetSkillUnlocked( eg_string_crc SpellId, eg_bool bUnlocked )
{
	if( bUnlocked )
	{
		m_UnlockedSkills.AppendUnique( SpellId );
	}
	else
	{
		if( m_UnlockedSkills.Contains( SpellId ) )
		{
			m_UnlockedSkills.DeleteByItem( SpellId );
		}
	}
}

void ExSkills::GetAllUnlocked( EGArray<eg_string_crc>& ListOut ) const
{
	ListOut = m_UnlockedSkills;
}

void ExSkills::ResetUnlockedSkills()
{
	m_UnlockedSkills.Clear();
}

void ExSkills::GetSkills( const ExGame* Game , const ExFighter* Fighter, ExSpellList& ListOut , ex_skills_get_type InGetType , eg_int IntValue /* = 0 */ ) const
{
	if( nullptr == Fighter )
	{
		return;
	}

	const ex_map_info_map_t MapType = Game ? Game->GetCurrentMapInfo().MapType : ex_map_info_map_t::Dungeon;

	ExSpellList AllSpells;
	ExSpellBook::Get().GetAll( AllSpells );

	if( InGetType == ex_skills_get_type::UnlockingThisLevel || InGetType == ex_skills_get_type::UnlockingAtLevel )
	{
		const ex_class_t Class = Fighter ? Fighter->GetClass() : ex_class_t::Unknown;
		const ex_attr_value Level = InGetType == ex_skills_get_type::UnlockingAtLevel ? IntValue : Fighter ? Fighter->GetAttrValue( ex_attr_t::LVL ) : 1;
		for( const exSpellInfo* SpellInfo : AllSpells )
		{
			if( SpellInfo && !SpellInfo->bIsLocked && SpellInfo->MinLevel == Level && SpellInfo->CanClassUse( Class ) )
			{
				ListOut.Append( SpellInfo );
			}
		}
	}
	else if( InGetType == ex_skills_get_type::Available || InGetType == ex_skills_get_type::AvailableOutOfCombat || InGetType == ex_skills_get_type::AvailableForCombat )
	{
		for( const exSpellInfo* SpellInfo : AllSpells )
		{
			if( CanFighterUseSkill( Fighter , SpellInfo ) )
			{
				eg_bool bWants = false;
				if( InGetType == ex_skills_get_type::AvailableOutOfCombat && ( (MapType == ex_map_info_map_t::Town && SpellInfo->bCanCastInWorldTown) || (MapType == ex_map_info_map_t::Dungeon && SpellInfo->bCanCastInWorldDungeon) ) )
				{
					bWants = true;
				}
				else if( InGetType == ex_skills_get_type::Available )
				{
					bWants = true;
				}
				else if( InGetType == ex_skills_get_type::AvailableForCombat && SpellInfo->bCanCastInCombat )
				{
					bWants = true;
				}

				if( bWants )
				{
					ListOut.Append( SpellInfo );
				}
			}
		}
	}
	else
	{
		for( eg_uint i=0; i<EX_MAX_ATTUNED_SKILLS; i++ )
		{
			eg_string_crc SkillId = Fighter->GetAttunedSkill( i );
			if( SkillId.IsNotNull() )
			{
				const exSpellInfo& SpellInfo = ExSpellBook::Get().FindInfo( SkillId );
				if( SpellInfo.Id.IsNotNull() && CanFighterUseSkill( Fighter , &SpellInfo ) )
				{
					eg_bool bWantsSkill = false;
					if( InGetType == ex_skills_get_type::Attuned )
					{
						bWantsSkill = true;
					}
					else if( InGetType == ex_skills_get_type::AttunedCombat )
					{
						bWantsSkill = SpellInfo.bCanCastInCombat;
					}
					else if( InGetType == ex_skills_get_type::AttunedOutOfCombat )
					{
						bWantsSkill = SpellInfo.bCanCastInWorldDungeon || SpellInfo.bCanCastInWorldTown;
					}

					if( bWantsSkill )
					{
						ListOut.Append( &SpellInfo );
					}
				}
			}
		}
	}
}

void ExSkills::SaveTo( EGFileData& FileOut )
{
	const eg_string_crc HeaderId = eg_crc("ExSkills");
	const eg_uint32 NumUnlockedSkills = m_UnlockedSkills.LenAs<eg_uint32>();
	FileOut.Write( &HeaderId , sizeof(HeaderId) );
	FileOut.Write( &NumUnlockedSkills , sizeof(NumUnlockedSkills) );
	for( eg_uint32 i=0; i<NumUnlockedSkills; i++ )
	{
		const eg_string_crc& SkillId = m_UnlockedSkills[i];
		FileOut.Write( &SkillId , sizeof(SkillId) );
	}
}

void ExSkills::LoadFrom( const EGFileData& FileIn )
{
	ResetUnlockedSkills();

	eg_string_crc HeaderId = CT_Clear;
	FileIn.Read( &HeaderId , sizeof(HeaderId) );
	if( HeaderId != eg_crc("ExSkills") )
	{
		EGLogf( eg_log_t::Error , __FUNCTION__ ": Header was incorrect, bad data." );
		assert( false );
		return;
	}
	eg_uint32 NumUnlockedSkills = 0;
	FileIn.Read( &NumUnlockedSkills , sizeof(NumUnlockedSkills) );
	for( eg_uint32 i=0; i<NumUnlockedSkills; i++ )
	{
		eg_string_crc SkillId = CT_Clear;
		FileIn.Read( &SkillId , sizeof(SkillId) );
		m_UnlockedSkills.AppendUnique( SkillId );
	}
}

eg_bool ExSkills::CanFighterUseSkill( const ExFighter* Fighter, const exSpellInfo* SpellInfo ) const
{
	eg_bool bCanUse = false;
	if( Fighter && SpellInfo )
	{
		eg_bool bClassAllowed = SpellInfo->CanClassUse( Fighter->GetClass() );
		eg_bool bLevelAllowed = SpellInfo->MinLevel <= Fighter->GetAttrValue( ex_attr_t::LVL );
		eg_bool bIsUnlocked = !SpellInfo->bIsLocked || IsSkillUnlocked( SpellInfo->Id );

		bCanUse = bClassAllowed && bLevelAllowed && bIsUnlocked;
	}

	return bCanUse;
}
