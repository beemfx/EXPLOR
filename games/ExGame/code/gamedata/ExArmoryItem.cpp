// (c) 2017 Beem Media

#include "ExArmoryItem.h"
#include "ExCore.h"
#include "ExGlobalData.h"

eg_bool exArmoryItem::CanEquipTo( ex_item_slot TestSlot ) const
{
	switch( EquipMode )
	{
	case ex_equip_m::NONE:
		break;
	case ex_equip_m::ALL_SLOTS:
	{
		EGArray<ex_item_slot> AllowedSlots;
		EquipSlots.GetAllowedSlots( AllowedSlots );
		if( AllowedSlots.IsValidIndex(0) && TestSlot == AllowedSlots[0] )
		{
			return true;
		}
	} break;
	case ex_equip_m::SINGLE_SLOT:
	{
		if( EquipSlots.IsSlotSet( TestSlot ) )
		{
			return true;
		}
	} break;
	}

	return false;
}

eg_bool exArmoryItem::AlwaysOccupiesSlot( ex_item_slot TestSlot ) const
{
	if( EquipMode == ex_equip_m::ALL_SLOTS )
	{
		if( EquipSlots.IsSlotSet( TestSlot ) )
		{
			return true;
		}
	}

	return false;
}

eg_bool exArmoryItem::CanBeUsedByClass( const ex_class_t InClassType ) const
{
	return AllowedPlayerClasses.IsClassSet( InClassType );
}

eg_bool exArmoryItem::CanEquipToMultipleSlots() const
{
	if( EquipMode == ex_equip_m::SINGLE_SLOT )
	{
		return EquipSlots.GetNumSlots() > 1;
	}
	return false;
}

ex_item_slot exArmoryItem::GetDefaultEquipToSlot() const
{
	return EquipSlots.GetDefaultSlot();
}

const exArmoryItemTier& exArmoryItem::GetItemTierData( eg_int ItemLevel , eg_int* TierBonus ) const
{
	const exArmoryItemTier* Out = Tiers.IsValidIndex(0) ? &Tiers[0] : &m_DefaultTierData;

	for( const exArmoryItemTier& Tier : Tiers )
	{
		if( ItemLevel >= Tier.TierLevel )
		{
			Out = &Tier;
		}
	}

	if( TierBonus )
	{
		*TierBonus = EG_Max<eg_int>( 0 , ItemLevel - Out->TierLevel );
	}

	return *Out;
}
