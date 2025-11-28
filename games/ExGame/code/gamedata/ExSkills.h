// (c) 2017 Beem Media

#pragma once

#include "ExSpellInfo.h"

class ExFighter;
class EGFileData;
class ExGame;

enum class ex_skills_get_type
{
	Available,
	AvailableForCombat,
	Attuned,
	AttunedCombat,
	AttunedOutOfCombat,
	AvailableOutOfCombat,
	UnlockingThisLevel,
	UnlockingAtLevel,
};

class ExSkills : public EGObject
{
	EG_CLASS_BODY( ExSkills , EGObject )

private:

	EGArray<eg_string_crc> m_UnlockedSkills;

public:

	eg_bool IsSkillUnlocked( eg_string_crc SpellId ) const;
	void SetSkillUnlocked( eg_string_crc SpellId , eg_bool bUnlocked );
	void GetAllUnlocked( EGArray<eg_string_crc>& ListOut ) const;
	void ResetUnlockedSkills();

	void GetSkills( const ExGame* Game , const ExFighter* Fighter , ExSpellList& ListOut , ex_skills_get_type InGetType , eg_int IntValue = 0 ) const;

	void SaveTo( EGFileData& FileOut );
	void LoadFrom( const EGFileData& FileIn );

	eg_bool CanFighterUseSkill( const ExFighter* Fighter , const exSpellInfo* SpellInfo ) const;
};

struct exSkillRepInfo
{
	union
	{
		struct
		{
			eg_string_crc SkillId;
			eg_bool       bUnlocked;
		};
		eg_int64 AsInt64;
	};

	exSkillRepInfo() = default;
	exSkillRepInfo( const eg_event_parms& Rhs )
	{
		*this = Rhs;
	}
	exSkillRepInfo( eg_string_crc SkillIdIn , eg_bool bUnlockedIn )
	{
		SkillId = SkillIdIn;
		bUnlocked = bUnlockedIn;
	}

	operator eg_event_parms() const 
	{ 
		static_assert( sizeof(eg_event_parms) == sizeof(*this) , "Too big!" );
		eg_event_parms Out( AsInt64 );
		return Out;
	}

	void operator = ( const eg_event_parms& Rhs )
	{
		AsInt64 = Rhs.as_int64();
	}
};

struct exAttuneSkillRepInfo
{
	union
	{
		struct
		{
			eg_string_crc SkillId;
			eg_uint16     RosterIndex;
			eg_bool       bAttune;
		};
		eg_int64 AsInt64;
	};

	exAttuneSkillRepInfo() = default;
	exAttuneSkillRepInfo( const eg_event_parms& Rhs )
	{
		*this = Rhs;
	}
	exAttuneSkillRepInfo( eg_uint RosterIndexIn , eg_bool bAttuneIn , eg_string_crc SkillIdIn )
	{
		SkillId = SkillIdIn;
		bAttune = bAttuneIn;
		RosterIndex = static_cast<eg_uint16>(RosterIndexIn);
	}

	operator eg_event_parms() const 
	{ 
		static_assert( sizeof(eg_event_parms) == sizeof(*this) , "Too big!" );
		eg_event_parms Out( AsInt64 );
		return Out;
	}

	void operator = ( const eg_event_parms& Rhs )
	{
		AsInt64 = Rhs.as_int64();
	}
};
