// (c) 2018 Beem Media

#pragma once

#include "ExGameTypes.h"
#include "EGDataAsset.h"
#include "ExCombatEncounter.reflection.h"

class EGRandom;
struct exBeastInfo;

struct exCombatEncounterMonster
{
	const exBeastInfo* BeastInfo = nullptr;
	ex_attr_value      Level = 0;
	eg_bool            bIsBoss = false;
	eg_int             EncounterListIndex = 0;
};

egreflect struct exCeMonsterInfo
{
	egprop eg_string_crc MonsterId = CT_Clear;
	egprop eg_int        MinLevel  = 1;
	egprop eg_int        MaxLevel  = 1;
	egprop eg_int        Weight    = 10;
	egprop eg_bool       bIsUniqueBoss = false;
};

egreflect struct exCombatEncounterInfo
{
	egprop eg_string_crc            Id          = CT_Clear;
	egprop eg_int                   MinMonsters = 1;
	egprop eg_int                   MaxMonsters = 20;
	egprop EGArray<exCeMonsterInfo> Monsters;

	void CreateMonsterList( EGRandom& Rng , EGArray<exCombatEncounterMonster>& Out ) const;
};

egreflect struct exCombatEncounterCategory
{
	egprop eg_string_crc CategoryName = CT_Clear;
	egprop EGArray<exCombatEncounterInfo> Items;
};

egreflect class ExCombatEncounter : public egprop EGDataAsset
{
	EG_CLASS_BODY( ExCombatEncounter , EGDataAsset )
	EG_FRIEND_RFL( ExCombatEncounter )

private:

	egprop EGArray<exCombatEncounterCategory> m_Categories;

	EGArray<exCombatEncounterInfo> m_CombatEncounters;

	static ExCombatEncounter* s_Inst;

public:

	static void Init( eg_cpstr Filename );
	static void Deinit();
	static ExCombatEncounter& Get() { return *s_Inst; }

	const exCombatEncounterInfo* FindCombatEncounter( const eg_string_crc& EncounterId ) const;

public:

	virtual void OnPropChanged( const egRflEditor& ChangedProperty , egRflEditor& RootEditor , eg_bool& bNeedsRebuildOut ) override;
	virtual void PostLoad( eg_cpstr16 Filename , eg_bool bForEditor , egRflEditor& RflEditor ) override;
};
