// (c) 2017 Beem Media

#pragma once

#include "ExGameTypes.h"
#include "EGTextFormat.h"
#include "EGRemoteEvent.h"

class ExGame;
class ExFighter;

static const eg_uint EX_INVENTORY_MAX_BAG_SIZE = 100;

struct exInventoryItem : public IEGCustomFormatHandler
{
	exRawItem RawItem;
	eg_uint InventoryListIndex;
	const struct exArmoryItem* ArmoryItem;

	exInventoryItem() = default;

	exInventoryItem( eg_ctor_t Ct )
	: ArmoryItem( nullptr )
	, RawItem( Ct )
	{
		if( Ct == CT_Default || Ct == CT_Clear )
		{
			InventoryListIndex = 0;
		}
	}

	exInventoryItem( const exRawItem& InRawItem )
	: ArmoryItem( nullptr )
	, InventoryListIndex( 0 )
	{
		RawItem = InRawItem;
	}

	exInventoryItem( const exRawItem& InRawItem , eg_bool bResolvePointers )
	: ArmoryItem( nullptr )
	, InventoryListIndex( 0 )
	{
		RawItem = InRawItem;
		if( bResolvePointers )
		{
			ResolvePointers();
		}
	}

	eg_bool operator == ( const exInventoryItem& rhs ) const
	{
		return RawItem == rhs.RawItem;
	}

	operator exRawItem() const { return RawItem; }

	ex_attr_value GetSellValue() const;
	ex_attr_value GetBuyValue() const;

	// BEGIN IEGCustomFormatHandler
	virtual void FormatText( eg_cpstr Flags , class EGTextParmFormatter* Formatter ) const override final;
	// END IEGCustomFormatHandler

	void ResolvePointers();

	exAttrSet GetAttrBoost( const exScaleFuncAttrs& Attrs ) const;
	const struct exArmoryItemTier& GetTierData( eg_int* TierBonus ) const;

private:

	static const ExFighter* s_ReferenceFighter;

public:

	static void SetReferenceFighter( ExFighter* FighterIn );
};

struct exEquipEventData
{
	static const eg_uint UNEQUIP_INDEX = EX_INVENTORY_MAX_BAG_SIZE;
	
	union
	{
		eg_int64 AsInt64;
		struct 
		{
			eg_uint16 InventoryIndex;
			eg_uint8  PartyMemberIndex;
			eg_uint8  ItemSlot;
		};
	};

	exEquipEventData( eg_uint InInventoryIndex , eg_uint InPartyMemberIndex , ex_item_slot InSlot )
	{
		InventoryIndex = static_cast<eg_uint16>(InInventoryIndex);
		PartyMemberIndex = static_cast<eg_uint8>(InPartyMemberIndex);
		ItemSlot = static_cast<eg_uint8>(InSlot);
	}
	exEquipEventData( const eg_event_parms& Parms ){ AsInt64 = Parms.as_int64(); }
	eg_event_parms AsEventParms() const { return eg_event_parms( AsInt64 ); }
	operator eg_event_parms() const { return AsEventParms(); }
	eg_uint GetInventoryIndex() const { return InventoryIndex; }
	eg_uint GetPartyMemberIndex() const { return PartyMemberIndex; }
	ex_item_slot GetItemSlot() const { return static_cast<ex_item_slot>(ItemSlot); }
};

static_assert( sizeof(exEquipEventData) == sizeof(eg_int64) , "Equip event data must fit in int64 for remote event" );

struct exSwapEventData
{
	union
	{
		eg_int64 AsInt64;
		struct 
		{
			eg_uint16 FromSlot;
			eg_uint16 ToSlot;
		};
	};

	exSwapEventData( eg_uint InFromSlot , eg_uint InToSlot )
	{
		FromSlot = static_cast<eg_uint16>(InFromSlot);
		ToSlot = static_cast<eg_uint16>(InToSlot);
	}
	exSwapEventData( const eg_event_parms& Parms ){ AsInt64 = Parms.as_int64(); }
	eg_event_parms AsEventParms() const { return eg_event_parms( AsInt64 ); }
	operator eg_event_parms() const { return AsEventParms(); }
	eg_uint GetFromSlot() const { return FromSlot; }
	eg_uint GetToSlot() const { return ToSlot; }
};

static_assert( sizeof(exSwapEventData) == sizeof(eg_int64) , "Swap event data must fit in int64 for remote event" );

class ExInventory
{
private:

	static const eg_uint BAG_WIDTH = 5; // NEEDS to match inventory menu

private:
	
	EGFixedArray<exInventoryItem,EX_INVENTORY_MAX_BAG_SIZE> m_ItemArray;
	EGFixedArray<eg_string_crc,EX_INVENTORY_MAX_BAG_SIZE> m_ItemHashes;
	eg_uint m_BagSize;
	eg_bool m_bBagSizeDirty;

public:

	ExInventory()
	: m_ItemArray( CT_Preserve )
	, m_ItemHashes( CT_Clear )
	{
		m_ItemArray.Resize( EX_INVENTORY_MAX_BAG_SIZE );
		m_ItemHashes.Resize( EX_INVENTORY_MAX_BAG_SIZE );

		MarkAllDataDirty();
	}

	void SetBagSize( eg_uint NewBagSize );
	eg_uint GetBagSize() const { return m_BagSize; }
	eg_uint GetNumItems() const;
	eg_bool IsBagFull() const;
	eg_uint GetFreeSpace() const;

	void AddItem( const exInventoryItem& NewItem );
	void AddItemAt( const exInventoryItem& NewItem , eg_uint BagPosition );

	void GetItems( EGArray<exInventoryItem>& Out ) const;

	void CreateDefaultInventory();

	void Sort();

	void ReplicateDirtyData( ExGame* Game );
	void OnItemReplicated( eg_size_t ItemIndex );
	void ResolveReplicatedData();
	void MarkAllDataDirty();
	void ClearPointers();

	exInventoryItem GetItemByIndex( eg_uint Index ) const;
	void DeleteItemByIndex( eg_uint Index );
};