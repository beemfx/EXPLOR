// (c) 2017 Beem Media

#include "ExInventory.h"
#include "ExGame.h"
#include "ExArmory.h"
#include "ExGlobalData.h"

//
// exInventoryItem
//

const ExFighter* exInventoryItem::s_ReferenceFighter = nullptr;

ex_attr_value exInventoryItem::GetSellValue() const
{
	assert( ArmoryItem );
	return EGMath_round( GetBuyValue() * EX_SELL_PERCENT ); 
}

ex_attr_value exInventoryItem::GetBuyValue() const
{
	assert( ArmoryItem );
	ex_attr_value BaseValue = ArmoryItem ? ArmoryItem->BaseGoldValue : 0;
	const ex_attr_value AdjustedLevel = EG_Max<ex_attr_value>(1,EGMath_round((RawItem.Level-1) * EX_ITEM_COST_GROWTH_MULTIPLIER) + 1);
	BaseValue = ExCore_GetRampedValue_Level( EG_To<eg_int>(BaseValue) , AdjustedLevel , EX_GOLD_GROWTH );
	return BaseValue;
}

void exInventoryItem::FormatText( eg_cpstr Flags, class EGTextParmFormatter* Formatter ) const
{
	assert( ArmoryItem );
	if( nullptr == ArmoryItem )
	{
		return;
	}

	eg_int TierBonus = 0;
	const exArmoryItemTier& TierData = GetTierData( &TierBonus );

	eg_cpstr FlagsParm = Flags;
	eg_string_crc BaseFlag = Formatter->GetNextFlag( &FlagsParm );

	switch_crc( BaseFlag )
	{
		case_crc("NAME"):
		{
			eg_string_crc NameParm = Formatter->GetNextFlag( &FlagsParm );
			eg_bool bNoColor = (NameParm == eg_crc("NOCOLOR") );
			eg_bool bWantColor = !bNoColor;
			
			eg_loc_text ItemName;

			if( TierBonus <= 0 )
			{
				ItemName = EGFormat( eg_loc("InventoryItemNameNoLevel","{0}") , TierData.Name.Text );
			}
			else
			{
				ItemName = EGFormat( eg_loc("InventoryItemNameWithLevel","{0} +{1}") , TierData.Name.Text , TierBonus );
			}

			if( bWantColor )
			{
				switch( TierData.Rarity )
				{
					case ex_rarity_t::Normal:
						ItemName = EGFormat( L"|SC(EX_RARE_1)|{0}|RC()|" , ItemName );
						break;
					case ex_rarity_t::Rare:
						ItemName = EGFormat( L"|SC(EX_RARE_2)|{0}|RC()|" , ItemName );
						break;
					case ex_rarity_t::VeryRare:
						ItemName = EGFormat( L"|SC(EX_RARE_3)|{0}|RC()|" , ItemName );
						break;
					case ex_rarity_t::Elite:
						ItemName = EGFormat( L"|SC(EX_RARE_4)|{0}|RC()|" , ItemName );
						break;
					case ex_rarity_t::Artifact:
						ItemName = EGFormat( L"|SC(EX_RARE_5)|{0}|RC()|" , ItemName );
						break;
				}
			}

			Formatter->SetText( ItemName.GetString() );
		} break;
		case_crc("COST"):
		{
			eg_string_crc CostParm = Formatter->GetNextFlag( &FlagsParm );
			eg_loc_text CostText;
			switch_crc(CostParm)
			{
				case_crc("SELL"):
					CostText = EGFormat(eg_loc("InventoryItemCostSellText","Sell Price: |SC(EX_GOLD)|{0} gold|RC()|") , GetSellValue() );
					break;
				case_crc("BUY"):
					CostText = EGFormat(eg_loc("InventoryItemCostBuyText","Purchase Price: |SC(EX_GOLD)|{0} gold|RC()|") , GetBuyValue() );
					break;
				case_crc("BUYBACK"):
					CostText = EGFormat(eg_loc("InventoryItemCostBuyBackText","Buy-back Price: |SC(EX_GOLD)|{0} gold|RC()|") , GetSellValue() );
					break;
				default:
					CostText = EGFormat(eg_loc("InventoryItemCostText","|SC(EX_GOLD)|{0} gold|RC()|") , GetBuyValue() );
					break;
			}

			Formatter->SetText( CostText.GetString() );
		} break;
		case_crc("DESC"):
		{
			eg_string_crc DescFlag = Formatter->GetNextFlag( &FlagsParm );
			const eg_loc_text RawDesc( TierData.Desc.Text );
			if( DescFlag == eg_crc("ITEM_CARD") )
			{
				eg_string_crc ItemCardParm = Formatter->GetNextFlag( &FlagsParm );
				eg_loc_text FullDesc = EGFormat( L"|SC(EX_ATTR)|{0:REQS}|RC()|\n{0:BOOSTS}\n{1}" , this , RawDesc );
				Formatter->SetText( FullDesc.GetString() );
			}
			else
			{
				Formatter->SetText( eg_loc_text(TierData.Desc.Text).GetString() );
			}
		} break;
		case_crc("BOOSTS"):
		{
			const eg_string_crc Flag = Formatter->GetNextFlag( &FlagsParm );
			const eg_bool bSingleLine = Flag == eg_crc("SINGLELINE");

			eg_bool bCanUse = true;
			exScaleFuncAttrs ScaleAttrs;
			eg_bool bHasRef = false;
			if( s_ReferenceFighter && s_ReferenceFighter->GetClass() != ex_class_t::Unknown )
			{
				bHasRef = true;
				bCanUse = s_ReferenceFighter->MeetsRequirementsToEquip( *this );
				ScaleAttrs.LVL = s_ReferenceFighter->GetAttrValue( ex_attr_t::LVL );
				ScaleAttrs.STR = s_ReferenceFighter->GetAttrValue( ex_attr_t::STR );
				ScaleAttrs.END = s_ReferenceFighter->GetAttrValue( ex_attr_t::END );
				ScaleAttrs.MAG = s_ReferenceFighter->GetAttrValue( ex_attr_t::MAG );
				ScaleAttrs.SPD = s_ReferenceFighter->GetAttrValue( ex_attr_t::SPD );
			}
			if( ScaleAttrs.LVL < 1 )
			{
				assert( false );
				ScaleAttrs.LVL = 1;
			}
			exAttrSet Boost = GetAttrBoost( ScaleAttrs );

			// Do up to X attributes boosts
			eg_uint NumShown = 0;
			eg_bool bDidFirst = false;
			eg_loc_text BoostsText;

			auto HandleBoost = [&NumShown,&bDidFirst,&BoostsText,&bSingleLine,&bHasRef,&bCanUse]( ex_attr_t AttrType , ex_attr_value Value ) -> void
			{
				if( Value != 0 && NumShown < 6 )
				{
					eg_loc_text ThisBoostText;
					
					if( !bCanUse )
					{
						ThisBoostText = EGFormat(eg_loc("InventoryCantUseAttribute","|SC(EX_ATTR)|{0}|RC()| |SC(RED)|N/A|RC()|") , ExCore_GetAttributeName( AttrType ));
					}
					else
					{
						ThisBoostText = EGFormat( Value >= 0 ? L"|SC(EX_ATTR)|{0}|RC()| +{1}" : L"|SC(EX_ATTR)|{0}|RC()| {1}" , ExCore_GetAttributeName( AttrType ) , Value );
					}

					if( bSingleLine )
					{
						BoostsText = EGFormat( bDidFirst ? L"{0} {1}" : L"{1}" , BoostsText , ThisBoostText );
					}
					else
					{
						BoostsText = EGFormat( bDidFirst ? L"{0}\n{1}" : L"{1}" , BoostsText , ThisBoostText );
					}
					NumShown++;
					bDidFirst = true;
				}
			};


			#define ATTR_ITEM( _var_ , _aname_ , _lname_ , _desc_ ) HandleBoost( ex_attr_t::_var_ , Boost._var_ );
			#include "ExAttrs.items"
			
			Formatter->SetText( BoostsText.GetString() );
		} break;
		case_crc("REQS"):
		{
			const eg_bool bCanUse = s_ReferenceFighter && s_ReferenceFighter->GetClass() != ex_class_t::Unknown ? s_ReferenceFighter->MeetsRequirementsToEquip( *this ) : true;

			eg_loc_text FullReqs = bCanUse ? EGFormat( L"{0:EQUIP_SLOTS}\n{0:EQUIP_CLASSES}" , this ) : EGFormat( L"{0:EQUIP_SLOTS}\n|SC(RED)|{0:EQUIP_CLASSES}|RC()|" , this );
			Formatter->SetText( FullReqs.GetString() );
		} break;

		case_crc("EQUIP_SLOTS"):
		{
			eg_loc_text AllSlots;

			auto IsTwoHanded = [this]() -> eg_bool
			{
				return ArmoryItem->EquipMode == ex_equip_m::ALL_SLOTS && (ArmoryItem->EquipSlots.bWeapon1 && ArmoryItem->EquipSlots.bWeapon2);

			};

			auto IsEitherHand = [this]() -> eg_bool
			{
				if( ArmoryItem->EquipMode != ex_equip_m::SINGLE_SLOT )
				{
					return false;
				}

				return ArmoryItem->EquipSlots.bWeapon1 && ArmoryItem->EquipSlots.bWeapon2;
			};

			if( IsTwoHanded() )
			{
				AllSlots = eg_loc_text( eg_loc("ExArmoryItemTwoHanded","Two-handed") );
			}
			else if( IsEitherHand() )
			{
				AllSlots = eg_loc_text( eg_loc("ExArmoryItemAnyHand","Any Hand") );
			}
			else
			{
				EGArray<ex_item_slot> AllowedSlots;
				ArmoryItem->EquipSlots.GetAllowedSlots( AllowedSlots );
				for( eg_size_t i=0; i<AllowedSlots.Len(); i++ )
				{
					ex_item_slot Slot = AllowedSlots[i];
					eg_bool bHasNext = (i != (AllowedSlots.Len()-1));

					if( bHasNext )
					{
						AllSlots = EGFormat( L"{0}{1}, " , AllSlots.GetString() , eg_loc_text(ExCore_GetItemSlotName( Slot )).GetString() );
					}
					else
					{
						AllSlots = EGFormat( L"{0}{1}" , AllSlots.GetString() , eg_loc_text(ExCore_GetItemSlotName( Slot )).GetString() );
					}
				}
			}
			Formatter->SetText( AllSlots.GetString() );
		} break;

		case_crc("EQUIP_CLASSES"):
		{
			eg_bool bAnyNotAllowed = false;
			eg_loc_text ClassesText;
			eg_bool bFirst = true;

			auto CheckClass = [&bAnyNotAllowed,&ClassesText,&bFirst,this]( ex_class_t ClassType ) -> void
			{
				if( ArmoryItem->CanBeUsedByClass( ClassType ) )
				{
					if( bFirst )
					{
						ClassesText = eg_loc_text(ExCore_GetClassName( ClassType ));
					}
					else
					{
						ClassesText = EGFormat( L"{0}, {1}" , ClassesText , ExCore_GetClassName( ClassType ) );
					}

					bFirst = false;
				}
				else
				{
					bAnyNotAllowed = true;
				}
			};

			CheckClass( ex_class_t::Warrior );
			CheckClass( ex_class_t::Thief );
			CheckClass( ex_class_t::Mage );
			CheckClass( ex_class_t::Cleric );

			if( !bAnyNotAllowed )
			{
				ClassesText = eg_loc_text(eg_loc("ExArmoryItemAnyClass","Any Class"));
			}

			Formatter->SetText( ClassesText.GetString() );
		} break;

		default:
		{
			assert( false );
		} break;
	}
}

void exInventoryItem::ResolvePointers()
{
	if( RawItem.ItemId.IsNotNull() )
	{
		ArmoryItem = &ExArmory::Get().FindInfo( RawItem.ItemId );
	}
	else
	{
		ArmoryItem = nullptr;
	}
}

exAttrSet exInventoryItem::GetAttrBoost( const exScaleFuncAttrs& Attrs ) const
{
	assert( ArmoryItem );
	exAttrSet Out = ArmoryItem ? ExCore_GetModiferBoost( ArmoryItem->Modifiers , Attrs , RawItem.Level ) : CT_Clear;
	return Out;
}

const struct exArmoryItemTier& exInventoryItem::GetTierData( eg_int* TierBonus ) const
{
	assert( ArmoryItem );
	return ArmoryItem->GetItemTierData( RawItem.Level , TierBonus );
}

void exInventoryItem::SetReferenceFighter( ExFighter* FighterIn )
{
	s_ReferenceFighter = FighterIn;
}

void ExInventory::SetBagSize( eg_uint NewBagSize )
{
	assert( NewBagSize <= EX_INVENTORY_MAX_BAG_SIZE ); // Cannot have a bigger bag than this.
	m_BagSize = EG_Min( NewBagSize, EX_INVENTORY_MAX_BAG_SIZE );
	m_bBagSizeDirty = true;
}

eg_uint ExInventory::GetNumItems() const
{
	eg_uint Out = 0;
	for( const exInventoryItem& Item : m_ItemArray )
	{
		if( Item.RawItem.ItemId.IsNotNull() )
		{
			Out++;
		}
	}
	return Out;
}

eg_bool ExInventory::IsBagFull() const
{
	return GetNumItems() >= GetBagSize();
}

eg_uint ExInventory::GetFreeSpace() const
{
	return GetBagSize() - GetNumItems();
}

//
// ExInventory
//

void ExInventory::AddItem( const exInventoryItem& NewItem )
{
	assert( !IsBagFull() ); // Should have checked this before adding...

	// Find the first slot for an empty item and put it there.
	for( eg_uint i=0; i<m_ItemArray.LenAsUInt(); i++ )
	{
		if( m_ItemArray[i].RawItem.ItemId == CT_Clear )
		{
			m_ItemArray[i] = NewItem;
			m_ItemArray[i].InventoryListIndex = i;
			m_ItemArray[i].ArmoryItem = &ExArmory::Get().FindInfo( NewItem.RawItem.ItemId );
			m_ItemHashes[i] = CT_Clear;
			return;
		}
	}

	assert( false ); // No room to add this item.
}

void ExInventory::AddItemAt( const exInventoryItem& NewItem, eg_uint BagPosition )
{
	if( m_ItemArray.IsValidIndex( BagPosition ) && BagPosition < GetBagSize() )
	{
		assert( m_ItemArray[BagPosition].RawItem.ItemId.IsNull() ); // SHould have removed item from bag before replacing it.
		m_ItemArray[BagPosition] = NewItem;
		m_ItemArray[BagPosition].InventoryListIndex = BagPosition;
		m_ItemArray[BagPosition].ArmoryItem = &ExArmory::Get().FindInfo( NewItem.RawItem.ItemId );
		m_ItemHashes[BagPosition] = CT_Clear;
	}
	else
	{
		assert( false ); // This slot is not in the bag.
	}
}

void ExInventory::GetItems( EGArray<exInventoryItem>& Out ) const
{
	for( const exInventoryItem& Item : m_ItemArray )
	{
		if( !Item.RawItem.ItemId.IsNull() )
		{
			Out.Append( Item );
		}
	}
}

void ExInventory::CreateDefaultInventory()
{
	EGArray<exRawItem> DefaultInventory;
	ExGlobalData::Get().GetDefaultInventory( DefaultInventory );
	for( const exRawItem& Item : DefaultInventory )
	{
		AddItem( exInventoryItem( Item ) );
	}
}

void ExInventory::Sort()
{
	for( exInventoryItem& Item : m_ItemArray )
	{
		Item.ResolvePointers();
	}

	m_ItemArray.Sort( []( const exInventoryItem& Left , const exInventoryItem& Right ) -> eg_bool
	{
		if( Left.RawItem.ItemId.IsNotNull() && Right.RawItem.ItemId.IsNull() )
		{
			return true;
		}
		else if( Left.RawItem.ItemId.IsNull() && Right.RawItem.ItemId.IsNotNull() )
		{
			return false;
		}
		else if( Left.RawItem.ItemId.IsNull() && Right.RawItem.ItemId.IsNull() )
		{
			return false;
		}
		else if( Left.ArmoryItem && Right.ArmoryItem && Left.ArmoryItem != Right.ArmoryItem )
		{
			if( Left.ArmoryItem->SortIndex != Right.ArmoryItem->SortIndex )
			{
				return Left.ArmoryItem->SortIndex > Right.ArmoryItem->SortIndex;
			}

			return Left.ArmoryItem->Id < Right.ArmoryItem->Id; // Use Id so sort is consistent.
		}
		return Left.RawItem.Level > Right.RawItem.Level;
	} );

	for( eg_int i=0; i<m_ItemArray.LenAs<eg_int>(); i++ )
	{
		m_ItemArray[i].InventoryListIndex = i;
	}

	ClearPointers();

	MarkAllDataDirty();
}

void ExInventory::ReplicateDirtyData( ExGame* Game )
{
	assert( m_ItemArray.Len() == m_ItemHashes.Len() );
	assert( Game->IsServer() );

	if( m_ItemArray.Len() != m_ItemHashes.Len() )
	{
		return;
	}

	if( m_bBagSizeDirty )
	{
		m_bBagSizeDirty = false;
		Game->SDK_ReplicateToClient( &m_BagSize , sizeof(m_BagSize) );
	}

	for( eg_size_t i=0; i<m_ItemArray.Len(); i++ )
	{
		exInventoryItem& Item = m_ItemArray[i];
		Item.ArmoryItem = nullptr; // We don't want this as part of the hash.
		eg_string_crc ItemHash = eg_string_crc::HashData( &Item , sizeof(Item) );
		if( ItemHash != m_ItemHashes[i] )
		{
			m_ItemHashes[i] = ItemHash;
			Game->SDK_ReplicateToClient( &Item , sizeof(Item) );
			Game->SDK_RunClientEvent( eg_crc("OnItemReplicated") , i );
		}
	}
}

void ExInventory::OnItemReplicated( eg_size_t ItemIndex )
{
	assert( m_ItemArray.IsValidIndex( ItemIndex ) );
	if( m_ItemArray.IsValidIndex( ItemIndex ) )
	{
		m_ItemArray[ItemIndex].ArmoryItem = &ExArmory::Get().FindInfo( m_ItemArray[ItemIndex].RawItem.ItemId );
	}
}

void ExInventory::ResolveReplicatedData()
{
	for( exInventoryItem& Item : m_ItemArray )
	{
		Item.ResolvePointers();
	}
}

void ExInventory::MarkAllDataDirty()
{
	for( eg_string_crc& ItemHash : m_ItemHashes )
	{
		ItemHash = CT_Clear;
	}

	m_bBagSizeDirty = true;
}

void ExInventory::ClearPointers()
{
	for( exInventoryItem& Item : m_ItemArray )
	{
		Item.ArmoryItem = nullptr;
	}
}

exInventoryItem ExInventory::GetItemByIndex( eg_uint Index ) const
{
	return m_ItemArray.IsValidIndex( Index ) && !m_ItemArray[Index].RawItem.ItemId.IsNull() ? m_ItemArray[Index] : exInventoryItem( CT_Clear );
}

void ExInventory::DeleteItemByIndex( eg_uint Index )
{
	if( m_ItemArray.IsValidIndex( Index ) )
	{
		m_ItemArray[Index] = CT_Clear;
	}
}
