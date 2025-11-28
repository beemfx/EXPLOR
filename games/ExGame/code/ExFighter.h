// (c) 2017 Beem Media

#pragma once

#include "ExGameTypes.h"
#include "EGTextFormat.h"
#include "ExDamage.h"
#include "ExInventory.h"

enum class ex_fighter_get_attr_t
{
	FinalValue,
	BaseValue,
	BaseValueWithEquipmentBoosts,
	BaseValueWithEquipmentAndCombatBoosts,
	EquipmentBoosts,
	CombatBoosts,
};

class ExFighter
{
private:

	typedef EGFixedArray<exInventoryItem,enum_count(ex_item_slot)> ExEquippedArray;
	typedef EGFixedArray<eg_string_crc,EX_MAX_ATTUNED_SKILLS> ExAttunedSkillsArray;

public:

	ExFighter( eg_ctor_t Ct );

	void SetCombatBoosts( exAttrSet* Boosts ){	m_CombatBoosts = Boosts; }
	eg_bool HasCombatBoost() const;
	void InitAs( ex_class_t Class , eg_bool bForCharacterCreate );
	void InitAs( const eg_string_crc& ClassId , eg_bool bForCharacterCreate );
	void InitMonster( const struct exBeastInfo& BeastInfo , ex_attr_value Level );
	void InitDefaultItemsAndSkills();
	ex_attr_value GetAttrValue( ex_attr_t Type , ex_fighter_get_attr_t GetType = ex_fighter_get_attr_t::FinalValue ) const;
	ex_attr_value GetMeleeAttrValue( ex_attr_t Type ) const;
	void SetAttrValue( ex_attr_t Type , ex_attr_value NewValue );
	void SetAttackType( ex_attack_t NewAttackType );
	void ApplyRawHit( ex_attr_value Hit ){	m_HitPoints = EG_Clamp<ex_attr_value>( m_HitPoints - Hit , 0 , GetAttrValue( ex_attr_t::HP ) );	}
	void ApplyRawHeal( ex_attr_value Heal ){ m_HitPoints = EG_Clamp<ex_attr_value>( m_HitPoints + Heal , 0 , GetAttrValue( ex_attr_t::HP ) );	}
	void ApplyManaRestoration( ex_attr_value Amount ){ m_MagicPoints = EG_Clamp<ex_attr_value>( m_MagicPoints + Amount , 0 , GetAttrValue( ex_attr_t::MP ) ); }
	void UseMana( ex_attr_value ManaUsage ){ m_MagicPoints = EG_Clamp<ex_attr_value>( m_MagicPoints - ManaUsage , 0 , GetAttrValue( ex_attr_t::MP ) ); }
	void RestoreAllHP(){	m_HitPoints = GetAttrValue( ex_attr_t::HP );	}
	void RestoreAllMP(){	m_MagicPoints = GetAttrValue( ex_attr_t::MP ); }
	void AwardXP( ex_xp_value XpToAward ){ assert( XpToAward >= 0 ); m_XP += XpToAward; }
	exDamage GetDamage() const;
	exDamage GetDefense() const;
	ex_class_t GetClass() const { return m_Class; }
	ex_attr_value GetHP() const {	return m_HitPoints; }
	ex_attr_value GetMP() const {	return m_MagicPoints; }
	ex_xp_value GetXP() const { return m_XP; }

	eg_bool CanTrainToNextLevel() const;
	eg_bool IsInConditionToTrainToNextLevel() const;
	void TrainToNextLevel();

	eg_bool CanFight() const;
	eg_bool IsDead() const { return m_bIsDead; }
	void SetDead( eg_bool bIsDead ){ m_bIsDead = bIsDead; }
	eg_bool IsUnconscious() const { return m_bIsUnconscious; }
	void SetUnconcious( eg_bool bIsUnconcious ){m_bIsUnconscious = bIsUnconcious; }
	eg_bool IsCursed() const { return m_bIsCursed; }
	void SetCursed( eg_bool bIsCursed ){ m_bIsCursed = bIsCursed; }
	eg_bool IsPetrified() const { return m_bIsPetrified; }
	void SetPetrified( eg_bool bIsPetrified ){ m_bIsPetrified = bIsPetrified; }
	eg_bool IsCreated() const { return m_bCreated; }
	void SetCreated( eg_bool val ) { m_bCreated = val; }
	ex_gender_t   GetGender() const;
	const exInventoryItem GetItemInSlot( ex_item_slot Slot , eg_bool bActualSlot ) const;
	eg_bool IsInventorySlotFull( ex_item_slot Slot ) const;
	eg_bool MeetsRequirementsToEquip( const exInventoryItem& Item ) const;
	void GetItemsRemovedOnEquip(  ex_item_slot Slot , const exInventoryItem& Item , EGArray<exInventoryItem>& ItemsRemoved ) const;
	void EquipInventoryItem( ex_item_slot Slot , const exInventoryItem& Item , EGArray<exInventoryItem>& ItemsRemoved );
	eg_string_crc GetAttunedSkill( eg_size_t SlotIndex ) const { return m_AttunedSkills.IsValidIndex( SlotIndex ) ? m_AttunedSkills[SlotIndex] : CT_Clear; }
	void SetSkillAttuned( eg_string_crc SkillId , eg_bool bAttuned );
	eg_bool IsSkillAttuned( eg_string_crc SkillId , eg_size_t* AttunedSlotOut ) const;
	eg_bool IsAttunedSkillsFull() const;
	void ResolveReplicatedData();
	void ResolveEquipmentBoosts();
	void ClearPointers();
	void DeleteEquipment();
	eg_string_crc GetStatusText() const;
	ex_target_strategy GetTargetStrategy() const;
	eg_bool HasRangedAttack() const;

	void FormatTextInternal( eg_cpstr Flags , EGTextParmFormatter* Formatter ) const;
	operator eg_loc_parm() const;

public:

	eg_loc_char   LocName[21];
	eg_string_crc PortraitId;

//
// Vars:
//
private:

	ex_class_t    m_Class;
	eg_string_crc m_BeastId = CT_Clear;
	ex_xp_value   m_XP;
	ex_attr_value m_HitPoints;
	ex_attr_value m_MagicPoints;
	eg_bool       m_bIsDead : 1;
	eg_bool       m_bIsUnconscious : 1;
	eg_bool       m_bIsCursed : 1;
	eg_bool       m_bIsPetrified : 1;
	eg_bool       m_bCreated : 1; // For roster characters indicates that this one actually exists.

	#define BASE_ATTR_ITEM( _var_ , _aname_ , _lname_ , _desc_ ) ex_attr_value m_##_var_ ;
	#include "ExAttrs.items"

	ExEquippedArray      m_EquippedItems;
	exAttrSet            m_EquipmentBoosts;
	ex_attack_t          m_CurrentAttackType = ex_attack_t::NONE;
	ExAttunedSkillsArray m_AttunedSkills;

	exAttrSet* m_CombatBoosts;
	const exBeastInfo* m_BeastInfo;

private:

	#define COMP_ATTR_ITEM( _var_ , _aname_ , _lname_ , _desc_ ) ex_attr_value GetCompAttr_##_var_()const;
	#include "ExAttrs.items"
};


class ExFighterTextHander : public IEGCustomFormatHandler
{
private:

	const ExFighter* m_Fighter;
public:

	ExFighterTextHander( const ExFighter* InFighter )
	: m_Fighter( InFighter )
	{

	}

	// BEGIN IEGCustomFormatHandler
	virtual void FormatText( eg_cpstr Flags , EGTextParmFormatter* Formatter ) const override final;
	// END IEGCustomFormatHandler
};
