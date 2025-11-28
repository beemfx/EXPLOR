// (c) 2017 Beem Media

#pragma once

#include "ExGameTypes.h"
#include "ExDamage.h"
#include "EGTextFormat.h"
#include "ExSpellInfo.reflection.h"

class ExFighter;

egreflect struct exSpellInfo : public IEGCustomFormatHandler
{
	egprop eg_string_crc   Id = CT_Clear;
	egprop ex_spell_t      Type = ex_spell_t::CombatDamage;
	egprop eg_string_crc   Name = CT_Clear;
	egprop eg_d_string     Name_enus = CT_Clear;
	egprop eg_string_crc   Description = CT_Clear;
	egprop eg_d_string     Description_enus = CT_Clear;
	egprop ex_spell_target Target = ex_spell_target::None;
	egprop eg_int          TargetCount = 1;
	egprop eg_real         ManaCost = 0;
	egprop ex_spell_func   ManaCostFunction = ex_spell_func::Fixed;
	egprop ex_spell_func   DamageFunction = ex_spell_func::MagicMulLvlGrowth;
	egprop exAttrSet       AttributesEffected = ex_attr_set_t::Boosts;
	egprop exPlayerClassSet AllowedPlayerClasses;
	egprop eg_bool         bCanCastInCombat = true;
	egprop eg_bool         bCanCastInWorldDungeon = false;
	egprop eg_bool         bCanCastInWorldTown = false;
	egprop eg_bool         bIsLocked = false;
	egprop eg_int          MinLevel = 1;

	ex_attr_value GetManaCost( ex_attr_value Magic , ex_attr_value Level ) const;
	exDamage GetDamage( ex_attr_value Magic , ex_attr_value Level ) const;
	ex_attr_value GetAttrBoost( ex_attr_t AttrType , ex_attr_value Magic , ex_attr_value Level ) const;
	eg_bool CanTargetEnemy() const;
	eg_bool CanTargetFriendly() const;
	eg_bool CanTargetOutOfCombat() const;
	eg_bool CanTargetInWorld() const;
	eg_bool TargetsEveryoneInWorld() const;
	eg_bool CanClassUse( ex_class_t ClassType ) const;

	// BEGIN IEGCustomFormatHandler
	virtual void FormatText( eg_cpstr Flags , class EGTextParmFormatter* Formatter ) const override final;
	// END IEGCustomFormatHandler

private:

	static const ExFighter* s_ReferenceFighter;

public:

	static void SetReferenceFighter( ExFighter* FighterIn );
};

typedef EGArray<const exSpellInfo*> ExSpellList;
