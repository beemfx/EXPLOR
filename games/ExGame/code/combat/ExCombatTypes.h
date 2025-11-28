// (c) 2017 Beem Media

#pragma once

#include "ExGameTypes.h"

enum class ex_combat_anim : eg_uint
{
	MeleeAttack,
	AnyDeath,
	SpellBonus,
	SpellAttack,
	DamageNumber,
	Burgled,
	Resist,
	SpellFail,
	CriticalHit,
	COUNT,
};

enum class ex_combat_s
{
	OUT_OF_COMBAT,
	FIRST_ROW,
	BACK_ROW,
	DOWN, // Dead until revived (the ExFighter will have more information as to why).
	ESCAPED, // Left combat for the remainder (no XP awarded)
};

enum class ex_combat_turn_opt
{
	AUTO   , // AI handles the turn, all other combat turn input parameters are ignored.
	CUSTOM , // The input parameters specify what the fighter does
};

enum class ex_combat_actions
{
	UNKNOWN,
	AUTO,
	FIGHT,
	BLOCK,
	MELEE,
	SHOOT,
	CAST,
	ADVANCE,
	BACKAWAY,
	RUN,
	VIEW,
	ABANDON,
	DEBUG_WIN,
};

class ExCombatant;

struct exCombatAnimInfo
{
	ex_combat_anim Type;
	ExCombatant*   Attacker;
	ExCombatant*   Defender;
	ex_attr_value  Damage;

	eg_bool IsDamageNumberType() const { return Type != ex_combat_anim::COUNT && Type != ex_combat_anim::AnyDeath && Type != ex_combat_anim::Burgled && Type != ex_combat_anim::Resist; }
};

class IExCombatAnimHandler
{
public:

	virtual void PlayAnimation( const exCombatAnimInfo& AnimInfo ) = 0;
};

typedef EGArray<ExCombatant*> ExCombatantList;
typedef EGFixedArray<ex_combat_actions,10> ExCombatActionsArray;