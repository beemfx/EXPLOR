// (c) 2017 Beem Media

#pragma once

#include "ExGameTypes.h"
#include "EGEngineSerializeTypes.h"
#include "EGAssetPath.h"
#include "EGEngineConfig.h"
#include "ExArmoryItem.reflection.h"

egreflect struct exArmoryItemTier
{
	egprop eg_int TierLevel = 1;
	egprop exLocText Name;
	egprop exLocText Desc;
	egprop ex_rarity_t Rarity = ex_rarity_t::Normal;
	egprop eg_asset_path Icon = EXT_TEX;
};

egreflect struct exArmoryItem
{
	egprop eg_string_crc Id = CT_Clear;
	egprop eg_int SortIndex = 0;
	egprop EGArray<exArmoryItemTier> Tiers;
	egprop exSlotSet EquipSlots = CT_Clear;
	egprop ex_equip_m EquipMode = ex_equip_m::NONE;
	egprop ex_attack_t AttackType = ex_attack_t::NONE;
	egprop eg_int BaseGoldValue = 0;
	egprop exPlayerClassSet AllowedPlayerClasses;
	egprop EGArray<exAttrModifier> Modifiers;

	exArmoryItemTier m_DefaultTierData;

	exArmoryItem() = default;
	exArmoryItem( eg_ctor_t Ct )
	: exArmoryItem()
	{
		unused( Ct );
		assert( Ct == CT_Default );
	}

	eg_bool CanEquipTo( ex_item_slot TestSlot ) const;
	eg_bool AlwaysOccupiesSlot( ex_item_slot TestSlot ) const;
	eg_bool CanBeUsedByClass( const ex_class_t InClassType ) const;
	eg_bool CanEquipToMultipleSlots() const;
	ex_item_slot GetDefaultEquipToSlot() const;

	const exArmoryItemTier& GetItemTierData( eg_int ItemLevel , eg_int* TierBonus ) const;
};
