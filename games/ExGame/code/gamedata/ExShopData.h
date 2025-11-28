// (c) 2017 Beem Media

#pragma once

#include "ExGameTypes.h"
#include "EGDataAsset.h"
#include "ExShopData.reflection.h"

struct exRawItem;

struct exShopPurchaseParms
{
	union
	{
		struct
		{
			eg_string_crc ArmoryItemId;
			eg_uint16     ItemLevel;
			eg_bool       bIsBuyback:1;
		};
		eg_int64 AsInt64;
	};

	exShopPurchaseParms() = default;
	exShopPurchaseParms( const struct eg_event_parms& Rhs );
	exShopPurchaseParms( const struct exInventoryItem& InItem , eg_bool bInIsBuyback );
	operator eg_event_parms() const;
	void operator = ( const struct eg_event_parms& Rhs );
};

egreflect enum class ex_shop_data_selection_method
{
	AllItems ,
	RandomSelection ,
};

egreflect struct exShopDataSelectionInfo
{
	egprop ex_shop_data_selection_method Method = ex_shop_data_selection_method::AllItems;
	egprop eg_ivec2 RandomSelectionCountRange = eg_ivec2( 1 , 1 );
	egprop eg_int RandomSelectionLevelsBack = 0;
	egprop eg_int RandomSelectionLevelBackChance = 50;
	egprop eg_bool bAllowDuplicateItems = true;
};

egreflect struct exShopDataItem
{
	egprop exRawItem Item = CT_Default;
	egprop eg_int RequiredPartyLevel = 1;
	egprop eg_bool bItemLevelBasedOnPartyLevel = true;
	egprop eg_int ScalingMaxItemLevel = 3;
	egprop eg_int ScalingPartyLevelRange = 5;
};

egreflect struct exShopDataInventory
{
	egprop eg_string_crc           Id = CT_Clear;
	egprop eg_int                  PartyLevelAdjustment = 0;
	egprop exShopDataSelectionInfo SelectionMethod;
	egprop EGArray<exShopDataItem> Items;
};

egreflect class ExShopData : public egprop EGDataAsset
{
	EG_CLASS_BODY( ExShopData , EGDataAsset );
	EG_FRIEND_RFL( ExShopData )

private:

	egprop EGArray<exShopDataInventory> m_Inventories;

	static ExShopData* s_ShopData;

public:

	static void Init( eg_cpstr Filename );
	static void Deinit();
	static ExShopData& Get() { return *s_ShopData; }

	void GetInventory( const eg_string_crc& InventoryId , EGArray<exRawItem>& ItemsOut , ex_attr_value PartyMaxLevel , eg_uint RngSeed ) const;

private:

	static eg_int GetScaledItemLevel( const exShopDataItem& ShopItem , eg_int PartyMaxLevel );;
};
