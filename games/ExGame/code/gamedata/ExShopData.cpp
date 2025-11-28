// (c) 2017 Beem Media

#include "ExShopData.h"
#include "ExInventoryBag.h"
#include "EGRandom.h"

EG_CLASS_DECL( ExShopData )

ExShopData* ExShopData::s_ShopData = nullptr;

void ExShopData::Init( eg_cpstr Filename )
{
	s_ShopData = EGDataAsset::LoadDataAsset<ExShopData>( EGString_ToWide(Filename) );
	assert( nullptr != s_ShopData ); // Game will crash.
	Get();
}

void ExShopData::Deinit()
{
	EGDeleteObject( s_ShopData );
	s_ShopData = nullptr;
}

void ExShopData::GetInventory( const eg_string_crc& InventoryId, EGArray<exRawItem>& ItemsOut , ex_attr_value PartyMaxLevel , eg_uint RngSeed ) const
{	
	EGRandom Rng( RngSeed );
	
	for( const exShopDataInventory& Inventory : m_Inventories )
	{
		if( Inventory.Id.IsNotNull() && Inventory.Id == InventoryId )
		{
			const ex_attr_value EffectivePartyLevel = PartyMaxLevel + Inventory.PartyLevelAdjustment;

			struct exSourceItem
			{
				exRawItem Item;
				eg_int ShopBaseLevel;
				eg_int ShopIndex;
			};
			EGArray<exSourceItem> SourceList;

			for( eg_size_t i=0; i<Inventory.Items.Len(); i++ )
			{
				const exShopDataItem& ShopItem = Inventory.Items[i];
				if( EffectivePartyLevel >= ShopItem.RequiredPartyLevel )
				{
					exSourceItem NewItem;
					NewItem.Item = ShopItem.Item;
					NewItem.ShopBaseLevel = ShopItem.Item.Level;
					NewItem.ShopIndex = EG_To<eg_int>(i);
					if( ShopItem.bItemLevelBasedOnPartyLevel )
					{
						NewItem.Item.Level = GetScaledItemLevel( ShopItem , EffectivePartyLevel );
					}

					SourceList.Append( NewItem );
				}
			}

			if( 0 != RngSeed && Inventory.SelectionMethod.Method == ex_shop_data_selection_method::RandomSelection )
			{
				const eg_int NumItemsWanted = EG_Max( 0 , Rng.GetRandomRangeI( Inventory.SelectionMethod.RandomSelectionCountRange.x , Inventory.SelectionMethod.RandomSelectionCountRange.y ) );
				
				if( Inventory.SelectionMethod.bAllowDuplicateItems )
				{
					const eg_int NumItemsToSelect = EG_Min( NumItemsWanted , SourceList.LenAs<eg_int>() );

					EGArray<exSourceItem> NewList;
					NewList.Reserve( SourceList.Len() );
					for( eg_int i=0; i<NumItemsToSelect; i++ )
					{
						const eg_int ItemToSelect = Rng.GetRandomRangeI( 0 , SourceList.LenAs<eg_int>()-1 );
						if( SourceList.IsValidIndex( ItemToSelect ) )
						{
							NewList.Append( SourceList[ItemToSelect] );
						}
						else
						{
							assert( false );
						}
					}

					SourceList = std::move( NewList );
				}
				else
				{
					// Randomly remove items till we have the desired amount.
					while( SourceList.Len() > NumItemsWanted )
					{
						const eg_int ItemToDelete = Rng.GetRandomRangeI( 0 , SourceList.LenAs<eg_int>()-1 );
						if( SourceList.IsValidIndex( ItemToDelete ) )
						{
							SourceList.DeleteByIndex( ItemToDelete ); // Preserves ordering.
						}
						else
						{
							assert( false );
							break;
						}
					}
				}

				for( exSourceItem& Item : SourceList )
				{
					// There is some chance that we want a lower level item:
					if( Inventory.SelectionMethod.RandomSelectionLevelsBack != 0 && (Rng.GetRandomRangeI( 0 , 100 ) < Inventory.SelectionMethod.RandomSelectionLevelBackChance) )
					{
						const eg_int MinItemLevel = EG_Max<eg_int>( Item.ShopBaseLevel , Item.Item.Level - EG_Abs( Inventory.SelectionMethod.RandomSelectionLevelsBack ) );
						const eg_int MaxItemLevel = EG_Max<eg_int>( Item.ShopBaseLevel , Item.Item.Level - 1 );
						Item.Item.Level = Rng.GetRandomRangeI( MinItemLevel , MaxItemLevel );
					}
				}

				SourceList.Sort( []( const exSourceItem& Left , const exSourceItem& Right ) -> eg_bool
				{
					if( Left.ShopIndex != Right.ShopIndex )
					{
						return Left.ShopIndex < Right.ShopIndex;
					}

					if( Left.Item.Level != Right.Item.Level )
					{
						return Left.Item.Level > Right.Item.Level;
					}

					return Left.Item.ItemId < Right.Item.ItemId;
				} );
			}

			ItemsOut.Reserve( SourceList.Len() );
			for( const exSourceItem& Item : SourceList )
			{
				ItemsOut.Append( Item.Item );
			}
		}
	}
}

eg_int ExShopData::GetScaledItemLevel( const exShopDataItem& ShopItem , eg_int PartyMaxLevel )
{
	const eg_vec2 PartyRange(EG_To<eg_real>(ShopItem.RequiredPartyLevel),EG_To<eg_real>(ShopItem.RequiredPartyLevel + EG_Max<eg_int>(ShopItem.ScalingPartyLevelRange - 1,1)));
	const eg_vec2 ItemRange(EG_To<eg_real>(ShopItem.Item.Level),EG_To<eg_real>(ShopItem.ScalingMaxItemLevel));
	const eg_real EffectivePartyLevel = EG_Clamp<eg_real>( EG_To<eg_real>(PartyMaxLevel) , PartyRange.x , PartyRange.y );
	const eg_real ScaledItemLevel = EGMath_GetMappedRangeValue( EffectivePartyLevel , PartyRange , ItemRange );
	const eg_int ItemLevelOut = EG_Clamp<eg_int>( EGMath_floor(ScaledItemLevel) , ShopItem.Item.Level , ShopItem.ScalingMaxItemLevel );
	return ItemLevelOut;
}

exShopPurchaseParms::exShopPurchaseParms( const struct exInventoryItem& InItem , eg_bool bInIsBuyback )
{
	ArmoryItemId =  InItem.RawItem.ItemId;
	ItemLevel = static_cast<eg_uint16>(InItem.RawItem.Level);
	bIsBuyback = bInIsBuyback;
}

exShopPurchaseParms::exShopPurchaseParms( const struct eg_event_parms& Rhs )
{
	*this = Rhs;
}

exShopPurchaseParms::operator eg_event_parms() const
{
	static_assert( sizeof( eg_event_parms ) == sizeof( *this ), "Too big!" );
	eg_event_parms Out( AsInt64 );
	return Out;
}

void exShopPurchaseParms::operator=( const struct eg_event_parms& Rhs )
{
	AsInt64 = Rhs.as_int64();
}
